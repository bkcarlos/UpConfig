#include "Libssh2UpFile.h"
#include <libssh2_sftp.h>

#include <QFileInfo>
#include <QFile>
#include <QHostAddress>
#include <QTextCodec>
#include <QTextStream>

#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
# ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

Libssh2UpFile::Libssh2UpFile(const QString& ip, const int nPort, const QString& user, const QString& passwd)
{
    m_strIP = ip;
    m_strUser = user;
    m_strPasswd = passwd;
    m_nPort = nPort;
    m_bInit = false;

    if (!InitSession())
    {
        CloseSession();
    }
}

Libssh2UpFile::~Libssh2UpFile()
{
    CloseSession();
}

bool Libssh2UpFile::UpFile(const QString& LocalPath, const QString& RemoteDir)
{
    if (!m_bInit)
    {
        return false;
    }

    QFile qFile(LocalPath);
    if (qFile.exists() && !qFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QFileInfo qFileInfo(LocalPath);

    struct stat fileinfo;
    stat(LocalPath.toLocal8Bit().constData(), &fileinfo);

    QString strRemotePath(RemoteDir);
    if (strRemotePath.at(strRemotePath.size() - 1) != '/')
    {
        strRemotePath += '/';
    }
    strRemotePath += qFileInfo.fileName();

    LIBSSH2_SFTP* pSftpSession = libssh2_sftp_init(m_pSession);
    if (nullptr == pSftpSession)
    {
        return false;
    }

    /* Request a file via SFTP */
    LIBSSH2_SFTP_HANDLE* pSftpHandle =
        libssh2_sftp_open(pSftpSession, strRemotePath.toLocal8Bit().constData(),
            LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
            LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR |
            LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);

    if (nullptr == pSftpHandle)
    {
        return false;
    }

    QTextStream toText(&qFile);
    toText.setCodec("GBK");
    QString strReadData = toText.readAll();
    qFile.close();

    int nIndex = 0;
    // 文件分多次发送
    do
    {
        QString strSendData = strReadData.mid(nIndex, 512);
        if (strSendData.isEmpty())
        {
            break;
        }

        int nSend = libssh2_sftp_write(pSftpHandle, strSendData.toLocal8Bit().constData(), strSendData.size());
        if (nSend < 0)
        {
            libssh2_sftp_close(pSftpHandle);
            libssh2_sftp_shutdown(pSftpSession);
            return false;
        }
        nIndex += nSend;
    } while (true);

    libssh2_sftp_close(pSftpHandle);
    libssh2_sftp_shutdown(pSftpSession);

    return true;
}

bool Libssh2UpFile::UpFile(QStringList& LocalPaths, QString& RemoteDir)
{
    for (int i = 0; i < LocalPaths.size(); ++i)
    {
        bool bIsUp = UpFile(LocalPaths.at(i), RemoteDir);
        if (bIsUp)
        {
            return bIsUp;
        }
    }

    return true;
}

bool Libssh2UpFile::InitSession()
{
#ifdef WIN32
    WSADATA wsadata;
    int nSocketErr = 0;

    nSocketErr = WSAStartup(MAKEWORD(2, 0), &wsadata);
    if (nSocketErr != 0) {
        return false;
    }
#endif

    m_nSock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_nSock) {
        return false;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(22);
    sin.sin_addr.s_addr = inet_addr(m_strIP.toLocal8Bit().constData());
    if (::connect(m_nSock, (struct sockaddr*)(&sin),
        sizeof(struct sockaddr_in)) != 0) {
        return false;
    }

    int nLibsshErr = libssh2_init(0);
    if (0 != nLibsshErr)
    {
        return false;
    }

    m_pSession = libssh2_session_init();
    if (nullptr == m_pSession)
    {
        return false;
    }

    /* Since we have set non-blocking, tell libssh2 we are blocking */
    libssh2_session_set_blocking(m_pSession, 1);
    nLibsshErr = libssh2_session_handshake(m_pSession, m_nSock);
    if (0 != nLibsshErr)
    {
        return false;
    }

    libssh2_hostkey_hash(m_pSession, LIBSSH2_HOSTKEY_HASH_SHA1);
    QString strUserList = libssh2_userauth_list(m_pSession, m_strUser.toLocal8Bit().constData(), m_strUser.toLocal8Bit().length());

    /* We could authenticate via password */
    if (libssh2_userauth_password(m_pSession, m_strUser.toLocal8Bit().constData(), m_strPasswd.toLocal8Bit().constData()))
    {
        CloseSession();
        return false;
    }

    m_bInit = true;
    return true;
}

void Libssh2UpFile::CloseSession()
{
    if (!m_bInit)
    {
        return;
    }

#ifdef WIN32
    closesocket(m_nSock);
#else
    close(m_nSock);
#endif

    if (nullptr != m_pSession)
    {
        libssh2_session_disconnect(m_pSession, "Client disconnecting normally");
        libssh2_session_free(m_pSession);
        libssh2_exit();
        m_pSession = nullptr;
    }

    m_bInit = false;
}