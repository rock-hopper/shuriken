/********************************************************************************
** Form generated from reading UI file 'applygaindialog.ui'
**
** Created: Mon Jul 14 12:01:31 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APPLYGAINDIALOG_H
#define UI_APPLYGAINDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDial>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ApplyGainDialog
{
public:
    QVBoxLayout *verticalLayout;
    QDial *dial_Gain;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_Gain;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ApplyGainDialog)
    {
        if (ApplyGainDialog->objectName().isEmpty())
            ApplyGainDialog->setObjectName(QString::fromUtf8("ApplyGainDialog"));
        ApplyGainDialog->resize(194, 143);
        verticalLayout = new QVBoxLayout(ApplyGainDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        dial_Gain = new QDial(ApplyGainDialog);
        dial_Gain->setObjectName(QString::fromUtf8("dial_Gain"));
        dial_Gain->setMaximum(110);
        dial_Gain->setValue(10);

        verticalLayout->addWidget(dial_Gain);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(ApplyGainDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label);

        doubleSpinBox_Gain = new QDoubleSpinBox(ApplyGainDialog);
        doubleSpinBox_Gain->setObjectName(QString::fromUtf8("doubleSpinBox_Gain"));
        doubleSpinBox_Gain->setMaximum(11);
        doubleSpinBox_Gain->setSingleStep(0.1);
        doubleSpinBox_Gain->setValue(1);

        horizontalLayout->addWidget(doubleSpinBox_Gain);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(ApplyGainDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ApplyGainDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ApplyGainDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ApplyGainDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ApplyGainDialog);
    } // setupUi

    void retranslateUi(QDialog *ApplyGainDialog)
    {
        ApplyGainDialog->setWindowTitle(QApplication::translate("ApplyGainDialog", "Apply Gain", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ApplyGainDialog", "Gain:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ApplyGainDialog: public Ui_ApplyGainDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APPLYGAINDIALOG_H
