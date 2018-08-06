#pragma once

#include <QString>
#include <QStringList>
#include <libssh2.h>
#include <QTcpSocket>

class Libssh2UpFile
{
public:
    Libssh2UpFile(const QString& ip, const int nPort, const QString& user, const QString& passwd);
    ~Libssh2UpFile();

    void UpFile(const QString& LocalPath, const QString& RemoteDir);
    void UpFile(QStringList& LocalPaths, QString& RemoteDir);

private:
    bool InitSession();
    void CloseSession();

private:
    QString m_strIP;
    QString m_strUser;
    QString m_strPasswd;
    int     m_nPort;

    LIBSSH2_SESSION *m_pSession;
    QTcpSocket* m_pTcpSocket;
};
