#pragma once

#include <libssh2.h>
#include <QObject>
#include <QString>

class Libssh2Exec : public QObject
{
public:
    Libssh2Exec(const QString& ip, const int port, const QString& user, const QString& passwd);
    ~Libssh2Exec();

    bool Exec(const QString& strCmd);

private:
    int WaitSocket(int nSocketFd, LIBSSH2_SESSION* pSession);

private:
    QString m_strIP;
    QString m_strUser;
    QString m_strPasswd;
    int     m_nPort;
};
