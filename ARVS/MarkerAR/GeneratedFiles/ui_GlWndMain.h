/********************************************************************************
** Form generated from reading UI file 'GlWndMain.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GLWNDMAIN_H
#define UI_GLWNDMAIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ToolTestMainClass
{
public:

    void setupUi(QWidget *ToolTestMainClass)
    {
        if (ToolTestMainClass->objectName().isEmpty())
            ToolTestMainClass->setObjectName(QStringLiteral("ToolTestMainClass"));
        ToolTestMainClass->resize(365, 300);

        retranslateUi(ToolTestMainClass);

        QMetaObject::connectSlotsByName(ToolTestMainClass);
    } // setupUi

    void retranslateUi(QWidget *ToolTestMainClass)
    {
        ToolTestMainClass->setWindowTitle(QApplication::translate("ToolTestMainClass", "ToolTestMain", 0));
    } // retranslateUi

};

namespace Ui {
    class ToolTestMainClass: public Ui_ToolTestMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLWNDMAIN_H
