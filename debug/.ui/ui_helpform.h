/********************************************************************************
** Form generated from reading UI file 'helpform.ui'
**
** Created: Thu Jul 3 10:12:11 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HELPFORM_H
#define UI_HELPFORM_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextBrowser>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HelpForm
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_Home;
    QPushButton *pushButton_Back;
    QPushButton *pushButton_Forward;
    QSpacerItem *horizontalSpacer;
    QTextBrowser *textBrowser;

    void setupUi(QWidget *HelpForm)
    {
        if (HelpForm->objectName().isEmpty())
            HelpForm->setObjectName(QString::fromUtf8("HelpForm"));
        HelpForm->resize(768, 768);
        verticalLayout = new QVBoxLayout(HelpForm);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_Home = new QPushButton(HelpForm);
        pushButton_Home->setObjectName(QString::fromUtf8("pushButton_Home"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/go-home.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Home->setIcon(icon);

        horizontalLayout->addWidget(pushButton_Home);

        pushButton_Back = new QPushButton(HelpForm);
        pushButton_Back->setObjectName(QString::fromUtf8("pushButton_Back"));
        pushButton_Back->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/go-previous.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Back->setIcon(icon1);

        horizontalLayout->addWidget(pushButton_Back);

        pushButton_Forward = new QPushButton(HelpForm);
        pushButton_Forward->setObjectName(QString::fromUtf8("pushButton_Forward"));
        pushButton_Forward->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/go-next.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Forward->setIcon(icon2);

        horizontalLayout->addWidget(pushButton_Forward);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        textBrowser = new QTextBrowser(HelpForm);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));

        verticalLayout->addWidget(textBrowser);


        retranslateUi(HelpForm);

        QMetaObject::connectSlotsByName(HelpForm);
    } // setupUi

    void retranslateUi(QWidget *HelpForm)
    {
        HelpForm->setWindowTitle(QApplication::translate("HelpForm", "Help", 0, QApplication::UnicodeUTF8));
        pushButton_Home->setText(QApplication::translate("HelpForm", "Home", 0, QApplication::UnicodeUTF8));
        pushButton_Back->setText(QApplication::translate("HelpForm", "Back", 0, QApplication::UnicodeUTF8));
        pushButton_Forward->setText(QApplication::translate("HelpForm", "Forward", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class HelpForm: public Ui_HelpForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HELPFORM_H
