/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QLabel *label;
    QLineEdit *folderPathEdit;
    QPushButton *browseButton;
    QPlainTextEdit *commandEdit;
    QPushButton *previewButton;
    QPushButton *runButton;
    QPlainTextEdit *reportEdit;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1578, 761);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(280, 60, 241, 21));
        folderPathEdit = new QLineEdit(centralwidget);
        folderPathEdit->setObjectName("folderPathEdit");
        folderPathEdit->setGeometry(QRect(280, 80, 271, 41));
        browseButton = new QPushButton(centralwidget);
        browseButton->setObjectName("browseButton");
        browseButton->setGeometry(QRect(570, 80, 211, 41));
        commandEdit = new QPlainTextEdit(centralwidget);
        commandEdit->setObjectName("commandEdit");
        commandEdit->setGeometry(QRect(280, 150, 441, 71));
        previewButton = new QPushButton(centralwidget);
        previewButton->setObjectName("previewButton");
        previewButton->setGeometry(QRect(740, 150, 131, 61));
        runButton = new QPushButton(centralwidget);
        runButton->setObjectName("runButton");
        runButton->setGeometry(QRect(280, 240, 481, 61));
        reportEdit = new QPlainTextEdit(centralwidget);
        reportEdit->setObjectName("reportEdit");
        reportEdit->setGeometry(QRect(280, 350, 611, 331));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1578, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Inteligentny Organizator Plik\303\263w", nullptr));
        browseButton->setText(QCoreApplication::translate("MainWindow", "\360\237\223\201 Wybierz folder", nullptr));
        commandEdit->setPlainText(QCoreApplication::translate("MainWindow", "Wpisz polecenie, np: Pogrupuj zdj\304\231cia do folderu Wakacje...", nullptr));
        previewButton->setText(QCoreApplication::translate("MainWindow", "\360\237\221\201\357\270\217 Podgl\304\205d skryptu", nullptr));
        runButton->setText(QCoreApplication::translate("MainWindow", "\342\226\266\357\270\217 Wykonaj", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
