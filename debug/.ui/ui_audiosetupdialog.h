/********************************************************************************
** Form generated from reading UI file 'audiosetupdialog.ui'
**
** Created: Thu Mar 13 14:21:44 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUDIOSETUPDIALOG_H
#define UI_AUDIOSETUPDIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AudioSetupDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label_AudioBackend;
    QComboBox *comboBox_AudioBackend;
    QLabel *label_AudioDevice;
    QComboBox *comboBox_AudioDevice;
    QPushButton *pushButton_TestTone;
    QLabel *label_OutputChannels;
    QComboBox *comboBox_OutputChannels;
    QLabel *label_SampleRate;
    QComboBox *comboBox_SampleRate;
    QLabel *label_MidiInput;
    QListWidget *listWidget_MidiInput;
    QLabel *label_MidiInputTestTone;
    QLabel *label_BufferSize;
    QComboBox *comboBox_BufferSize;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AudioSetupDialog)
    {
        if (AudioSetupDialog->objectName().isEmpty())
            AudioSetupDialog->setObjectName(QString::fromUtf8("AudioSetupDialog"));
        AudioSetupDialog->resize(510, 400);
        verticalLayout = new QVBoxLayout(AudioSetupDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(20, -1, 20, -1);
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setLabelAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_AudioBackend = new QLabel(AudioSetupDialog);
        label_AudioBackend->setObjectName(QString::fromUtf8("label_AudioBackend"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_AudioBackend);

        comboBox_AudioBackend = new QComboBox(AudioSetupDialog);
        comboBox_AudioBackend->setObjectName(QString::fromUtf8("comboBox_AudioBackend"));

        formLayout->setWidget(0, QFormLayout::FieldRole, comboBox_AudioBackend);

        label_AudioDevice = new QLabel(AudioSetupDialog);
        label_AudioDevice->setObjectName(QString::fromUtf8("label_AudioDevice"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_AudioDevice);

        comboBox_AudioDevice = new QComboBox(AudioSetupDialog);
        comboBox_AudioDevice->setObjectName(QString::fromUtf8("comboBox_AudioDevice"));
        comboBox_AudioDevice->setMinimumSize(QSize(355, 0));

        formLayout->setWidget(1, QFormLayout::FieldRole, comboBox_AudioDevice);

        pushButton_TestTone = new QPushButton(AudioSetupDialog);
        pushButton_TestTone->setObjectName(QString::fromUtf8("pushButton_TestTone"));

        formLayout->setWidget(2, QFormLayout::FieldRole, pushButton_TestTone);

        label_OutputChannels = new QLabel(AudioSetupDialog);
        label_OutputChannels->setObjectName(QString::fromUtf8("label_OutputChannels"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_OutputChannels);

        comboBox_OutputChannels = new QComboBox(AudioSetupDialog);
        comboBox_OutputChannels->setObjectName(QString::fromUtf8("comboBox_OutputChannels"));
        comboBox_OutputChannels->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(3, QFormLayout::FieldRole, comboBox_OutputChannels);

        label_SampleRate = new QLabel(AudioSetupDialog);
        label_SampleRate->setObjectName(QString::fromUtf8("label_SampleRate"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_SampleRate);

        comboBox_SampleRate = new QComboBox(AudioSetupDialog);
        comboBox_SampleRate->setObjectName(QString::fromUtf8("comboBox_SampleRate"));
        comboBox_SampleRate->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(4, QFormLayout::FieldRole, comboBox_SampleRate);

        label_MidiInput = new QLabel(AudioSetupDialog);
        label_MidiInput->setObjectName(QString::fromUtf8("label_MidiInput"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_MidiInput);

        listWidget_MidiInput = new QListWidget(AudioSetupDialog);
        listWidget_MidiInput->setObjectName(QString::fromUtf8("listWidget_MidiInput"));

        formLayout->setWidget(6, QFormLayout::FieldRole, listWidget_MidiInput);

        label_MidiInputTestTone = new QLabel(AudioSetupDialog);
        label_MidiInputTestTone->setObjectName(QString::fromUtf8("label_MidiInputTestTone"));

        formLayout->setWidget(7, QFormLayout::FieldRole, label_MidiInputTestTone);

        label_BufferSize = new QLabel(AudioSetupDialog);
        label_BufferSize->setObjectName(QString::fromUtf8("label_BufferSize"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_BufferSize);

        comboBox_BufferSize = new QComboBox(AudioSetupDialog);
        comboBox_BufferSize->setObjectName(QString::fromUtf8("comboBox_BufferSize"));
        comboBox_BufferSize->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(5, QFormLayout::FieldRole, comboBox_BufferSize);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(AudioSetupDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AudioSetupDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AudioSetupDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AudioSetupDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(AudioSetupDialog);
    } // setupUi

    void retranslateUi(QDialog *AudioSetupDialog)
    {
        AudioSetupDialog->setWindowTitle(QApplication::translate("AudioSetupDialog", "Audio Setup", 0, QApplication::UnicodeUTF8));
        label_AudioBackend->setText(QApplication::translate("AudioSetupDialog", "Audio Backend:", 0, QApplication::UnicodeUTF8));
        label_AudioDevice->setText(QApplication::translate("AudioSetupDialog", "Audio Device:", 0, QApplication::UnicodeUTF8));
        pushButton_TestTone->setText(QApplication::translate("AudioSetupDialog", "Test Tone", 0, QApplication::UnicodeUTF8));
        label_OutputChannels->setText(QApplication::translate("AudioSetupDialog", "Output Channels:", 0, QApplication::UnicodeUTF8));
        label_SampleRate->setText(QApplication::translate("AudioSetupDialog", "Sample Rate:", 0, QApplication::UnicodeUTF8));
        label_MidiInput->setText(QApplication::translate("AudioSetupDialog", "MIDI Input:", 0, QApplication::UnicodeUTF8));
        label_MidiInputTestTone->setText(QApplication::translate("AudioSetupDialog", "MIDI input test tone disabled", 0, QApplication::UnicodeUTF8));
        label_BufferSize->setText(QApplication::translate("AudioSetupDialog", "Audio Buffer Size:", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AudioSetupDialog: public Ui_AudioSetupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOSETUPDIALOG_H
