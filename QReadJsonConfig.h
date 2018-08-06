#pragma once
#include <QString>
#include <map>
#include <QStringList>

struct Config
{
    int id;
    QString name;
    QString ip;
    int     port;
    QString path;
    QString user;
    QString passwd;
};

class QReadJsonConfig
{
public:
    ~QReadJsonConfig();

    static QReadJsonConfig* GetInstance();

    bool ReadJson(QString& strConfigPath);
    const Config* GetJsonConfig(int nID);
    void GetConfigNameList(std::map<int, QString>& mapIdToName);
private:
    QReadJsonConfig();
    static QReadJsonConfig* m_instance;
    std::map<int, Config*> m_mapConfig;
};
