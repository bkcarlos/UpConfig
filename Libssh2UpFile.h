#pragma once

#include <QString>
#include <QStringList>
#include <libssh2.h>
#include <QObject>

class Libssh2UpFile : public QObject
{
public:
    Libssh2UpFile(const QString& ip, const int nPort, const QString& user, const QString& passwd);
    ~Libssh2UpFile();

    bool UpFile(const QString& LocalPath, const QString& RemoteDir);
    bool UpFile(QStringList& LocalPaths, QString& RemoteDir);

private:
    bool InitSession();
    void CloseSession();

private:
    QString m_strIP;
    QString m_strUser;
    QString m_strPasswd;
    int     m_nPort;

    bool    m_bInit;
    int     m_nSock;

    LIBSSH2_SESSION *m_pSession;
};
