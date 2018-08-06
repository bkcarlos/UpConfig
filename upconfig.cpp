#include "upconfig.h"
#include "ui_upconfig.h"
#include "QReadJsonConfig.h"
#include "Libssh2UpFile.h"

#include <QMessageBox>
#include <QFileDialog>

const QString strShowTypeEnable("background-color: rgba(00,255,00,100);\
                          border: 1px groove gray;\
                          border-top-left-radius:6px;\
                          border-top-right-radius:6px;\
                          border-bottom-left-radius:6px;\
                          border-bottom-right-radius:6px;\
                          padding-left:0px;\
                          padding-right:0px;");

const QString strShowTypeDisenable("background-color: rgba(255,00,00,100);\
                          border: 1px groove gray;\
                          border-top-left-radius:6px;\
                          border-top-right-radius:6px;\
                          border-bottom-left-radius:6px;\
                          border-bottom-right-radius:6px;\
                          padding-left:0px;\
                          padding-right:0px;");

UpConfig::UpConfig(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpConfig)
{
    ui->setupUi(this);

    if (!InitControl())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("错误"),
            QString::fromLocal8Bit("初始化控件错误"), QMessageBox::Ok);

        exit(-1);
    }

    InitShow();
    InitConnects();
    m_nSelIndex = 1;
}

UpConfig::~UpConfig()
{
    if (nullptr != m_pSelectButton)
    {
        delete m_pSelectButton;
        m_pSelectButton = nullptr;
    }

    if (nullptr != m_pUpFileButton)
    {
        delete m_pUpFileButton;
        m_pUpFileButton = nullptr;
    }

    if (nullptr != m_pSelComboBox)
    {
        delete m_pSelComboBox;
        m_pSelComboBox = nullptr;
    }

    if (nullptr != m_pListWidget)
    {
        delete m_pListWidget;
        m_pListWidget = nullptr;
    }

    delete ui;
}

void UpConfig::InitShow()
{
    m_pSelectButton->setText(QString::fromLocal8Bit("选择文件"));
    m_pSelectButton->setGeometry(rect().x() + 20, rect().y() + 20, 100, 40);
    ControlSetEnbale(m_pSelectButton, true);

    m_pUpFileButton->setText(QString::fromLocal8Bit("上传文件"));
    m_pUpFileButton->setGeometry(rect().x() + 130, rect().y() + 20, 100, 40);
    ControlSetEnbale(m_pUpFileButton, false);

    m_pSelComboBox->setGeometry(rect().x() + 240, rect().y() + 20, 100, 40);
    m_pSelComboBox->setStyleSheet(strShowTypeEnable);

    std::map<int, QString> mapIdToName;
    QReadJsonConfig::GetInstance()->GetConfigNameList(mapIdToName);
    int nIndex = 0;
    std::map<int, QString>::iterator iter = mapIdToName.begin();
    for (; iter != mapIdToName.end(); ++iter)
    {
        m_pSelComboBox->addItem(QString(iter->first));
        m_pSelComboBox->setItemText(nIndex++, iter->second);
    }

    m_pReloadConfig->setText(QString::fromLocal8Bit("加载文件"));
    m_pReloadConfig->setGeometry(rect().x() + 350, rect().y() + 20, 100, 40);
    ControlSetEnbale(m_pReloadConfig, false);

    m_pListWidget->setGeometry(rect().x() + 20, rect().y() + 80, 340, 100);
}

bool UpConfig::InitControl()
{
    m_pSelectButton = new QPushButton(this);
    if (nullptr == m_pSelectButton)
    {
        return false;
    }

    m_pUpFileButton = new QPushButton(this);
    if (nullptr == m_pUpFileButton)
    {
        return false;
    }

    m_pReloadConfig = new QPushButton(this);
    if (nullptr == m_pReloadConfig)
    {
        return false;
    }

    m_pSelComboBox = new QComboBox(this);
    if (nullptr == m_pSelComboBox)
    {
        return false;
    }

    m_pListWidget = new QListWidget(this);
    if (nullptr == m_pListWidget)
    {
        return false;
    }

    return true;
}

void UpConfig::InitConnects()
{
    connect(m_pSelectButton, &QPushButton::clicked, this, &UpConfig::OnClickSelectFiles);
    connect(m_pUpFileButton, &QPushButton::clicked, this, &UpConfig::OnClickUpFilesButton);
    connect(m_pReloadConfig, &QPushButton::clicked, this, &UpConfig::OnClickReloadConfig);
    void(QComboBox::*fp)(int) = &QComboBox::currentIndexChanged;
    connect(m_pSelComboBox, fp, this, &UpConfig::OnSelectComBox);
}

void UpConfig::OnClickSelectFiles()
{
    QFileDialog* pFileDiglogSelectFiles = new QFileDialog(this);
    pFileDiglogSelectFiles->setWindowTitle(QString::fromLocal8Bit("请选择需要生成的文件"));
    pFileDiglogSelectFiles->setDirectory(".");
    pFileDiglogSelectFiles->setViewMode(QFileDialog::List);
    pFileDiglogSelectFiles->setNameFilter("txt files(*.txt)");
    pFileDiglogSelectFiles->setFileMode(QFileDialog::ExistingFiles);

    QStringList fileNameList;
    if (pFileDiglogSelectFiles->exec() == QFileDialog::Accepted)
    {
        fileNameList = pFileDiglogSelectFiles->selectedFiles();
    }

    if (fileNameList.isEmpty())
    {
        return;
    }
    m_pListWidget->clear();
    m_pListWidget->addItems(fileNameList);

    ControlSetEnbale(m_pUpFileButton, true);
    ControlSetEnbale(m_pSelectButton, false);
}

void UpConfig::OnClickUpFilesButton()
{
    const Config* pConfig = QReadJsonConfig::GetInstance()->GetJsonConfig(m_nSelIndex);
    if (nullptr == pConfig)
    {
        return;
    }

    Libssh2UpFile upFiles(pConfig->ip, pConfig->port, pConfig->user, pConfig->passwd);
    for (int i = 0; i < m_pListWidget->count(); ++i)
    {
        QListWidgetItem* pListWidgetItem = m_pListWidget->item(i);
        QString filePath = pListWidgetItem->text();
        upFiles.UpFile(filePath, pConfig->path);
    }
}

void UpConfig::OnClickReloadConfig()
{
}

void UpConfig::OnSelectComBox(int nIndex)
{
    m_nSelIndex = nIndex + 1;
}

void UpConfig::ControlSetEnbale(QWidget* pWidget, bool bEnable)
{
    if (bEnable)
    {
        pWidget->setStyleSheet(strShowTypeEnable);
        pWidget->setEnabled(true);
    }
    else
    {
        pWidget->setStyleSheet(strShowTypeDisenable);
        pWidget->setEnabled(false);
    }
}