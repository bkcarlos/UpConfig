#include "Libssh2UpFile.h"
#include <libssh2_sftp.h>

#include <QFileInfo>
#include <QFile>

Libssh2UpFile::Libssh2UpFile(const QString& ip, const int nPort, const QString& user, const QString& passwd)
{
    m_strIP = ip;
    m_strUser = user;
    m_strPasswd = passwd;
    m_nPort = nPort;

    if (!InitSession())
    {
        CloseSession();
    }
}

Libssh2UpFile::~Libssh2UpFile()
{
    CloseSession();
}

void Libssh2UpFile::UpFile(const QString& LocalPath, const QString& RemoteDir)
{
    QFile qFile(LocalPath);
    if (qFile.exists() && !qFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    LIBSSH2_SFTP* pSftpSession = libssh2_sftp_init(m_pSession);
    if (nullptr == pSftpSession) {
        return;
    }

    /* Request a file via SFTP */
    LIBSSH2_SFTP_HANDLE* pSftpHandle = libssh2_sftp_open(pSftpSession, RemoteDir.toLocal8Bit().constData(),
        LIBSSH2_FXF_WRITE | LIBSSH2_FXF_CREAT | LIBSSH2_FXF_TRUNC,
        LIBSSH2_SFTP_S_IRUSR | LIBSSH2_SFTP_S_IWUSR |
        LIBSSH2_SFTP_S_IRGRP | LIBSSH2_SFTP_S_IROTH);
    if (nullptr == pSftpHandle) {
        return;
    }

    QString strReadData = qFile.readAll();
    libssh2_sftp_write(pSftpHandle, strReadData.toLocal8Bit().constData(), strReadData.length());

    libssh2_sftp_close(pSftpHandle);
    libssh2_sftp_shutdown(pSftpSession);
}

void Libssh2UpFile::UpFile(QStringList& LocalPaths, QString& RemoteDir)
{
    for (int i = 0; i < LocalPaths.size(); ++i)
    {
        UpFile(LocalPaths.at(i), RemoteDir);
    }
}

bool Libssh2UpFile::InitSession()
{
    int nLibsshErr = libssh2_init(0);
    if (0 != nLibsshErr)
    {
        return false;
    }

    m_pTcpSocket = new QTcpSocket;
    if (nullptr == m_pTcpSocket)
    {
        return false;
    }

    m_pTcpSocket->connectToHost(m_strIP, m_nPort);
    if (!m_pTcpSocket->waitForConnected(100000))
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
    nLibsshErr = libssh2_session_handshake(m_pSession, m_pTcpSocket->socketDescriptor());
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

    return true;
}

void Libssh2UpFile::CloseSession()
{
    if (m_pTcpSocket->isOpen())
    {
        m_pTcpSocket->disconnectFromHost();
    }

    m_pTcpSocket->deleteLater();
    m_pTcpSocket = nullptr;

    libssh2_session_disconnect(m_pSession, "Client disconnecting normally");
    libssh2_session_free(m_pSession);
    libssh2_exit();
    m_pSession = nullptr;
}