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

class Ui_GlWndMainClass
{
public:

    void setupUi(QWidget *GlWndMainClass)
    {
        if (GlWndMainClass->objectName().isEmpty())
            GlWndMainClass->setObjectName(QStringLiteral("GlWndMainClass"));
        GlWndMainClass->resize(365, 300);

        retranslateUi(GlWndMainClass);

        QMetaObject::connectSlotsByName(GlWndMainClass);
    } // setupUi

    void retranslateUi(QWidget *GlWndMainClass)
    {
        GlWndMainClass->setWindowTitle(QApplication::translate("GlWndMainClass", "GlWndMain", 0));
    } // retranslateUi

};

namespace Ui {
    class GlWndMainClass: public Ui_GlWndMainClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GLWNDMAIN_H
