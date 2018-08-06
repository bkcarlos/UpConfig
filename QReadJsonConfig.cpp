#include "QReadJsonConfig.h"

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QMessageBox>

const char* jsonConfigPath = "./config.json";
QReadJsonConfig* QReadJsonConfig::m_instance = nullptr;

QReadJsonConfig::QReadJsonConfig()
{
    QString strConfigPath(jsonConfigPath);
    if (!ReadJson(strConfigPath))
    {
        QMessageBox::information(NULL, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("读取配置文件失败"));
    }
}

QReadJsonConfig::~QReadJsonConfig()
{
    std::map<int, Config*>::iterator iter = m_mapConfig.begin();
    for (; iter != m_mapConfig.end(); ++iter)
    {
        Config* pConfig = iter->second;
        if (nullptr != pConfig)
        {
            delete pConfig;
            pConfig = nullptr;
        }
    }
}

QReadJsonConfig* QReadJsonConfig::GetInstance()
{
    if (nullptr == m_instance)
    {
        m_instance = new QReadJsonConfig();
    }

    return m_instance;
}

bool QReadJsonConfig::ReadJson(QString& strConfigPath)
{
    QString strFileJson;
    QFile* file = new QFile(strConfigPath);
    if (file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        strFileJson = file->readAll();
        file->close();
        delete file;
        file = nullptr;
    }
    else
    {
        delete file;
        file = nullptr;
        return false;
    }

    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(strFileJson.toUtf8(), &jsonError);
    if (jsonDoc.isNull() || (jsonError.error != QJsonParseError::NoError))
    {
        return false;
    }
    if (!jsonDoc.isObject())
    {
        return false;
    }

    QJsonObject object = jsonDoc.object();
    if (!object.contains("config"))
    {
        return false;
    }
    QJsonValue value = object.value("config");
    if (!value.isArray())
    {
        return false;
    }

    QJsonArray jsonArray = value.toArray();
    int nSize = jsonArray.size();
    for (int i = 0; i < nSize; ++i)
    {
        QJsonValue value = jsonArray.at(i);
        if (!value.isObject())
        {
            continue;
        }
        QJsonObject configObject = value.toObject();
        Config *pConfig = new Config;
        if (nullptr == pConfig)
        {
            continue;
        }

        if (configObject.contains("id"))
        {
            QJsonValue cValue = configObject.value("id");
            pConfig->id = cValue.toInt();
        }
        else
        {
            delete pConfig;
            continue;
        }

        if (configObject.contains("name"))
        {
            QJsonValue cValue = configObject.value("name");
            pConfig->name = cValue.toString();
        }
        if (configObject.contains("ip"))
        {
            QJsonValue cValue = configObject.value("ip");
            pConfig->ip = cValue.toString();
        }
        if (configObject.contains("port"))
        {
            QJsonValue cValue = configObject.value("port");
            pConfig->port = cValue.toInt();
        }
        if (configObject.contains("path"))
        {
            QJsonValue cValue = configObject.value("path");
            pConfig->path = cValue.toString();
        }
        if (configObject.contains("user"))
        {
            QJsonValue cValue = configObject.value("user");
            pConfig->user = cValue.toString();
        }
        if (configObject.contains("passwd"))
        {
            QJsonValue cValue = configObject.value("passwd");
            pConfig->passwd = cValue.toString();
        }

        m_mapConfig.insert(std::make_pair(pConfig->id, pConfig));
    }

    return true;
}

const Config* QReadJsonConfig::GetJsonConfig(int nID)
{
    std::map<int, Config*>::iterator iterFind = m_mapConfig.find(nID);
    if (iterFind == m_mapConfig.end())
    {
        return nullptr;
    }

    return static_cast<Config*>(iterFind->second);
}

void QReadJsonConfig::GetConfigNameList(std::map<int, QString>& mapIdToName)
{
    std::map<int, Config*>::iterator iter = m_mapConfig.begin();
    for (; iter != m_mapConfig.end(); ++iter)
    {
        const Config* pConfig = iter->second;
        if (nullptr == pConfig)
        {
            continue;
        }

        mapIdToName.insert(std::make_pair(pConfig->id, pConfig->name));
    }
}