#include "Libssh2Exec.h"

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

Libssh2Exec::Libssh2Exec(const QString & ip, const int port, const QString & user, const QString & passwd)
{
    m_strIP = ip;
    m_strUser = user;
    m_strPasswd = passwd;
    m_nPort = port;
}

Libssh2Exec::~Libssh2Exec()
{
}

bool Libssh2Exec::Exec(const QString& strCmd)
{
    int exitcode;
    char *exitsignal = (char *)"none";
    int bytecount = 0;

#ifdef WIN32
    WSADATA wsadata;
    int err;

    err = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if (err != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", err);
        return false;
    }
#endif

    int nLibssh2Err = libssh2_init(0);
    if (nLibssh2Err != 0) {
        return false;
    }

    /* Ultra basic "connect to port 22 on localhost"
    * Your code is responsible for creating the socket establishing the
    * connection
    */
    int nSocketFd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(m_nPort);
    sin.sin_addr.s_addr = inet_addr(m_strIP.toLocal8Bit().constData());
    if (::connect(nSocketFd, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
        return false;
    }

    /* Create a session instance */
    LIBSSH2_SESSION *pSession = libssh2_session_init();
    if (nullptr == pSession)
        return false;

    /* tell libssh2 we want it all done non-blocking */
    libssh2_session_set_blocking(pSession, 0);

    /* ... start it up. This will trade welcome banners, exchange keys,
    * and setup crypto, compression, and MAC layers
    */
    while ((nLibssh2Err = libssh2_session_handshake(pSession, nSocketFd)) == LIBSSH2_ERROR_EAGAIN);
    if (nLibssh2Err != 0)
    {
        return false;
    }

    LIBSSH2_KNOWNHOSTS *pNH = libssh2_knownhost_init(pSession);
    if (nullptr == pNH) {
        /* eeek, do cleanup here */
        return false;
    }

    /* read all hosts from here */
    libssh2_knownhost_readfile(pNH, "known_hosts", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    /* store all known hosts to here */
    libssh2_knownhost_writefile(pNH, "dumpfile", LIBSSH2_KNOWNHOST_FILE_OPENSSH);

    size_t len;
    int type;
    QString fingerprint = libssh2_session_hostkey(pSession, &len, &type);
    if (fingerprint.isEmpty()) {
        struct libssh2_knownhost *host;
#if LIBSSH2_VERSION_NUM >= 0x010206
        /* introduced in 1.2.6 */
        int check = libssh2_knownhost_checkp(pNH, m_strIP.toLocal8Bit().constData(), m_nPort,
            fingerprint.toLocal8Bit().constData(), len,
            LIBSSH2_KNOWNHOST_TYPE_PLAIN |
            LIBSSH2_KNOWNHOST_KEYENC_RAW,
            &host);
#else
        /* 1.2.5 or older */
        int check = libssh2_knownhost_check(pNH, m_strIP.toLocal8Bit().constData(),
            fingerprint.toLocal8Bit().constData(), len,
            LIBSSH2_KNOWNHOST_TYPE_PLAIN |
            LIBSSH2_KNOWNHOST_KEYENC_RAW,
            &host);
#endif
        /*****
        * At this point, we could verify that 'check' tells us the key is
        * fine or bail out.
        *****/
    }
    else {
        /* eeek, do cleanup here */
        return false;
    }
    libssh2_knownhost_free(pNH);

    /* We could authenticate via password */
    while ((nLibssh2Err = libssh2_userauth_password(pSession, m_strUser.toLocal8Bit().constData(),
        m_strPasswd.toLocal8Bit().constData())) == LIBSSH2_ERROR_EAGAIN);
    if (nLibssh2Err != 0)
    {
        return false;
    }

#if 0
    libssh2_trace(session, ~0);
#endif

    LIBSSH2_CHANNEL *pChannel;
    /* Exec non-blocking on the remove host */
    while ((pChannel = libssh2_channel_open_session(pSession)) == NULL &&
        libssh2_session_last_error(pSession, NULL, NULL, 0) == LIBSSH2_ERROR_EAGAIN)
    {
        WaitSocket(nSocketFd, pSession);
    }
    if (nullptr == pChannel)
    {
        return false;
    }
    while ((nLibssh2Err = libssh2_channel_exec(pChannel, strCmd.toUtf8().constData())) ==
        LIBSSH2_ERROR_EAGAIN)
    {
        WaitSocket(nSocketFd, pSession);
    }
    if (nLibssh2Err != 0)
    {
        return false;
    }
    for (;; )
    {
        /* loop until we block */
        int rc;
        do
        {
            char buffer[0x4000];
            rc = libssh2_channel_read(pChannel, buffer, sizeof(buffer));
            if (rc > 0)
            {
                int i;
                bytecount += rc;
                fprintf(stderr, "We read:\n");
                for (i = 0; i < rc; ++i)
                    fputc(buffer[i], stderr);
                fprintf(stderr, "\n");
            }
            else {
                if (rc != LIBSSH2_ERROR_EAGAIN)
                    /* no need to output this for the EAGAIN case */
                    fprintf(stderr, "libssh2_channel_read returned %d\n", rc);
            }
        } while (rc > 0);

        /* this is due to blocking that would occur otherwise so we loop on
        this condition */
        if (rc == LIBSSH2_ERROR_EAGAIN)
        {
            WaitSocket(nSocketFd, pSession);
        }
        else
            break;
    }

    exitcode = 127;
    while ((nLibssh2Err = libssh2_channel_close(pChannel)) == LIBSSH2_ERROR_EAGAIN)
        WaitSocket(nSocketFd, pSession);

    if (nLibssh2Err != 0)
    {
        exitcode = libssh2_channel_get_exit_status(pChannel);
        libssh2_channel_get_exit_signal(pChannel, &exitsignal,
            NULL, NULL, NULL, NULL, NULL);
    }

    if (exitsignal)
        fprintf(stderr, "\nGot signal: %s\n", exitsignal);
    else
        fprintf(stderr, "\nEXIT: %d bytecount: %d\n", exitcode, bytecount);

    libssh2_channel_free(pChannel);
    pChannel = NULL;

shutdown:

    libssh2_session_disconnect(pSession,
        "Normal Shutdown, Thank you for playing");
    libssh2_session_free(pSession);

#ifdef WIN32
    closesocket(nSocketFd);
#else
    close(nSocketFd);
#endif
    fprintf(stderr, "all done\n");

    libssh2_exit();

    return true;
}

int Libssh2Exec::WaitSocket(int nSocketFd, LIBSSH2_SESSION * pSession)
{
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    fd_set fd;
    FD_ZERO(&fd);

    FD_SET(nSocketFd, &fd);

    /* now make sure we wait in the correct direction */
    int dir = libssh2_session_block_directions(pSession);

    fd_set *pWriteFd = nullptr;
    fd_set *pReadFd = nullptr;

    if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)
        pReadFd = &fd;

    if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
        pWriteFd = &fd;

    int rc = select(nSocketFd + 1, pReadFd, pWriteFd, NULL, &timeout);

    return rc;
}