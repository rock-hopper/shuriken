/********************************************************************************
** Form generated from reading UI file 'audiosetupdialog.ui'
**
** Created: Tue Jun 3 13:06:05 2014
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
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QFormLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AudioSetupDialog
{
public:
    QVBoxLayout *verticalLayout;
    QTabWidget *tabWidget;
    QWidget *tab_AudioSetup;
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
    QLabel *label_BufferSize;
    QComboBox *comboBox_BufferSize;
    QLabel *label_MidiInput;
    QListWidget *listWidget_MidiInput;
    QCheckBox *checkBox_MidiInputTestTone;
    QWidget *tab_TimeStretch;
    QFormLayout *formLayout_2;
    QLabel *label_2;
    QRadioButton *radioButton_Offline;
    QRadioButton *radioButton_RealTime;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *AudioSetupDialog)
    {
        if (AudioSetupDialog->objectName().isEmpty())
            AudioSetupDialog->setObjectName(QString::fromUtf8("AudioSetupDialog"));
        AudioSetupDialog->resize(515, 400);
        verticalLayout = new QVBoxLayout(AudioSetupDialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(20, -1, 20, -1);
        tabWidget = new QTabWidget(AudioSetupDialog);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab_AudioSetup = new QWidget();
        tab_AudioSetup->setObjectName(QString::fromUtf8("tab_AudioSetup"));
        formLayout = new QFormLayout(tab_AudioSetup);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout->setLabelAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        label_AudioBackend = new QLabel(tab_AudioSetup);
        label_AudioBackend->setObjectName(QString::fromUtf8("label_AudioBackend"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_AudioBackend);

        comboBox_AudioBackend = new QComboBox(tab_AudioSetup);
        comboBox_AudioBackend->setObjectName(QString::fromUtf8("comboBox_AudioBackend"));

        formLayout->setWidget(0, QFormLayout::FieldRole, comboBox_AudioBackend);

        label_AudioDevice = new QLabel(tab_AudioSetup);
        label_AudioDevice->setObjectName(QString::fromUtf8("label_AudioDevice"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_AudioDevice);

        comboBox_AudioDevice = new QComboBox(tab_AudioSetup);
        comboBox_AudioDevice->setObjectName(QString::fromUtf8("comboBox_AudioDevice"));
        comboBox_AudioDevice->setMinimumSize(QSize(340, 0));

        formLayout->setWidget(1, QFormLayout::FieldRole, comboBox_AudioDevice);

        pushButton_TestTone = new QPushButton(tab_AudioSetup);
        pushButton_TestTone->setObjectName(QString::fromUtf8("pushButton_TestTone"));

        formLayout->setWidget(2, QFormLayout::FieldRole, pushButton_TestTone);

        label_OutputChannels = new QLabel(tab_AudioSetup);
        label_OutputChannels->setObjectName(QString::fromUtf8("label_OutputChannels"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_OutputChannels);

        comboBox_OutputChannels = new QComboBox(tab_AudioSetup);
        comboBox_OutputChannels->setObjectName(QString::fromUtf8("comboBox_OutputChannels"));
        comboBox_OutputChannels->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(3, QFormLayout::FieldRole, comboBox_OutputChannels);

        label_SampleRate = new QLabel(tab_AudioSetup);
        label_SampleRate->setObjectName(QString::fromUtf8("label_SampleRate"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_SampleRate);

        comboBox_SampleRate = new QComboBox(tab_AudioSetup);
        comboBox_SampleRate->setObjectName(QString::fromUtf8("comboBox_SampleRate"));
        comboBox_SampleRate->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(4, QFormLayout::FieldRole, comboBox_SampleRate);

        label_BufferSize = new QLabel(tab_AudioSetup);
        label_BufferSize->setObjectName(QString::fromUtf8("label_BufferSize"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_BufferSize);

        comboBox_BufferSize = new QComboBox(tab_AudioSetup);
        comboBox_BufferSize->setObjectName(QString::fromUtf8("comboBox_BufferSize"));
        comboBox_BufferSize->setMinimumSize(QSize(170, 0));

        formLayout->setWidget(5, QFormLayout::FieldRole, comboBox_BufferSize);

        label_MidiInput = new QLabel(tab_AudioSetup);
        label_MidiInput->setObjectName(QString::fromUtf8("label_MidiInput"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_MidiInput);

        listWidget_MidiInput = new QListWidget(tab_AudioSetup);
        listWidget_MidiInput->setObjectName(QString::fromUtf8("listWidget_MidiInput"));

        formLayout->setWidget(6, QFormLayout::FieldRole, listWidget_MidiInput);

        checkBox_MidiInputTestTone = new QCheckBox(tab_AudioSetup);
        checkBox_MidiInputTestTone->setObjectName(QString::fromUtf8("checkBox_MidiInputTestTone"));

        formLayout->setWidget(7, QFormLayout::FieldRole, checkBox_MidiInputTestTone);

        tabWidget->addTab(tab_AudioSetup, QString());
        tab_TimeStretch = new QWidget();
        tab_TimeStretch->setObjectName(QString::fromUtf8("tab_TimeStretch"));
        formLayout_2 = new QFormLayout(tab_TimeStretch);
        formLayout_2->setObjectName(QString::fromUtf8("formLayout_2"));
        formLayout_2->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        formLayout_2->setLabelAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        formLayout_2->setFormAlignment(Qt::AlignCenter);
        label_2 = new QLabel(tab_TimeStretch);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label_2);

        radioButton_Offline = new QRadioButton(tab_TimeStretch);
        radioButton_Offline->setObjectName(QString::fromUtf8("radioButton_Offline"));
        radioButton_Offline->setChecked(true);

        formLayout_2->setWidget(0, QFormLayout::FieldRole, radioButton_Offline);

        radioButton_RealTime = new QRadioButton(tab_TimeStretch);
        radioButton_RealTime->setObjectName(QString::fromUtf8("radioButton_RealTime"));

        formLayout_2->setWidget(1, QFormLayout::FieldRole, radioButton_RealTime);

        tabWidget->addTab(tab_TimeStretch, QString());

        verticalLayout->addWidget(tabWidget);

        buttonBox = new QDialogButtonBox(AudioSetupDialog);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(AudioSetupDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), AudioSetupDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), AudioSetupDialog, SLOT(reject()));

        tabWidget->setCurrentIndex(0);


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
        label_BufferSize->setText(QApplication::translate("AudioSetupDialog", "Audio Buffer Size:", 0, QApplication::UnicodeUTF8));
        label_MidiInput->setText(QApplication::translate("AudioSetupDialog", "MIDI Input:", 0, QApplication::UnicodeUTF8));
        checkBox_MidiInputTestTone->setText(QApplication::translate("AudioSetupDialog", "Enable MIDI Input Test Tone", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_AudioSetup), QApplication::translate("AudioSetupDialog", "Audio Setup", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AudioSetupDialog", "TimeStretch Mode:", 0, QApplication::UnicodeUTF8));
        radioButton_Offline->setText(QApplication::translate("AudioSetupDialog", "Offline", 0, QApplication::UnicodeUTF8));
        radioButton_RealTime->setText(QApplication::translate("AudioSetupDialog", "RealTime", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_TimeStretch), QApplication::translate("AudioSetupDialog", "TimeStretch", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AudioSetupDialog: public Ui_AudioSetupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOSETUPDIALOG_H
