#ifndef UPCONFIG_H
#define UPCONFIG_H

#include <QDialog>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>

#include <map>

namespace Ui {
    class UpConfig;
}

class Config;

class UpConfig : public QDialog
{
    Q_OBJECT

public:
    explicit UpConfig(QWidget *parent = 0);
    ~UpConfig();

private:
    void InitShow();
    bool InitControl();
    void InitConnects();

    void OnClickSelectFiles();
    void OnClickUpFilesButton();
    void OnClickReloadConfig();

    void OnSelectComBox(int nIndex);

    void ControlSetEnbale(QWidget* pWidget, bool bEnable);

private:
    Ui::UpConfig *ui;

    QPushButton*   m_pSelectButton;
    QPushButton*   m_pUpFileButton;
    QPushButton*   m_pReloadConfig;
    QComboBox*     m_pSelComboBox;
    QListWidget*   m_pListWidget;

    int       m_nSelIndex;
};

#endif // UPCONFIG_H
