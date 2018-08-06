/********************************************************************************
** Form generated from reading UI file 'upconfig.ui'
**
** Created by: Qt User Interface Compiler version 5.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_UPCONFIG_H
#define UI_UPCONFIG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_UpConfig
{
public:

    void setupUi(QDialog *UpConfig)
    {
        if (UpConfig->objectName().isEmpty())
            UpConfig->setObjectName(QStringLiteral("UpConfig"));
        UpConfig->resize(400, 300);

        retranslateUi(UpConfig);

        QMetaObject::connectSlotsByName(UpConfig);
    } // setupUi

    void retranslateUi(QDialog *UpConfig)
    {
        UpConfig->setWindowTitle(QApplication::translate("UpConfig", "UpConfig", nullptr));
    } // retranslateUi

};

namespace Ui {
    class UpConfig: public Ui_UpConfig {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_UPCONFIG_H
