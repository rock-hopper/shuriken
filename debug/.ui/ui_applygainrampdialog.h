/********************************************************************************
** Form generated from reading UI file 'applygainrampdialog.ui'
**
** Created: Sun Jul 13 14:55:19 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_APPLYGAINRAMPDIALOG_H
#define UI_APPLYGAINRAMPDIALOG_H

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

class Ui_ApplyGainRampDialog
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QDial *dial_StartGain;
    QDial *dial_EndGain;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QDoubleSpinBox *doubleSpinBox_StartGain;
    QLabel *label_2;
    QDoubleSpinBox *doubleSpinBox_EndGain;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ApplyGainRampDialog)
    {
        if (ApplyGainRampDialog->objectName().isEmpty())
            ApplyGainRampDialog->setObjectName(QString::fromUtf8("ApplyGainRampDialog"));
        ApplyGainRampDialog->resize(275, 148);
        verticalLayout = new QVBoxLayout(ApplyGainRampDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        dial_StartGain = new QDial(ApplyGainRampDialog);
        dial_StartGain->setObjectName(QString::fromUtf8("dial_StartGain"));
        dial_StartGain->setMaximum(110);
        dial_StartGain->setValue(10);

        horizontalLayout_2->addWidget(dial_StartGain);

        dial_EndGain = new QDial(ApplyGainRampDialog);
        dial_EndGain->setObjectName(QString::fromUtf8("dial_EndGain"));
        dial_EndGain->setMaximum(110);
        dial_EndGain->setValue(10);

        horizontalLayout_2->addWidget(dial_EndGain);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(ApplyGainRampDialog);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label);

        doubleSpinBox_StartGain = new QDoubleSpinBox(ApplyGainRampDialog);
        doubleSpinBox_StartGain->setObjectName(QString::fromUtf8("doubleSpinBox_StartGain"));
        doubleSpinBox_StartGain->setMaximum(11);
        doubleSpinBox_StartGain->setSingleStep(0.1);
        doubleSpinBox_StartGain->setValue(1);

        horizontalLayout->addWidget(doubleSpinBox_StartGain);

        label_2 = new QLabel(ApplyGainRampDialog);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_2);

        doubleSpinBox_EndGain = new QDoubleSpinBox(ApplyGainRampDialog);
        doubleSpinBox_EndGain->setObjectName(QString::fromUtf8("doubleSpinBox_EndGain"));
        doubleSpinBox_EndGain->setMaximum(11);
        doubleSpinBox_EndGain->setSingleStep(0.1);
        doubleSpinBox_EndGain->setValue(1);

        horizontalLayout->addWidget(doubleSpinBox_EndGain);


        verticalLayout->addLayout(horizontalLayout);

        buttonBox = new QDialogButtonBox(ApplyGainRampDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);

        buttonBox->raise();
        dial_StartGain->raise();
        dial_EndGain->raise();
        doubleSpinBox_StartGain->raise();
        doubleSpinBox_EndGain->raise();
        label->raise();
        label_2->raise();
        dial_StartGain->raise();

        retranslateUi(ApplyGainRampDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), ApplyGainRampDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ApplyGainRampDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(ApplyGainRampDialog);
    } // setupUi

    void retranslateUi(QDialog *ApplyGainRampDialog)
    {
        ApplyGainRampDialog->setWindowTitle(QApplication::translate("ApplyGainRampDialog", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ApplyGainRampDialog", "Start Gain:", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("ApplyGainRampDialog", "End Gain:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ApplyGainRampDialog: public Ui_ApplyGainRampDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_APPLYGAINRAMPDIALOG_H
