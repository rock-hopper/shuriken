/********************************************************************************
** Form generated from reading UI file 'audiosetupdialog.ui'
**
** Created: Sun Jun 29 15:05:55 2014
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
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
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
    QGridLayout *gridLayout;
    QLabel *label;
    QRadioButton *radioButton_Offline;
    QRadioButton *radioButton_RealTime;
    QLabel *label_2;
    QRadioButton *radioButton_Elastic;
    QRadioButton *radioButton_Precise;
    QLabel *label_3;
    QRadioButton *radioButton_Crisp;
    QRadioButton *radioButton_Mixed;
    QRadioButton *radioButton_Smooth;
    QLabel *label_4;
    QRadioButton *radioButton_Laminar;
    QRadioButton *radioButton_Independent;
    QLabel *label_6;
    QRadioButton *radioButton_Standard;
    QRadioButton *radioButton_Short;
    QRadioButton *radioButton_Long;
    QLabel *label_7;
    QRadioButton *radioButton_Shifted;
    QRadioButton *radioButton_Preserved;
    QLabel *label_8;
    QRadioButton *radioButton_HighSpeed;
    QRadioButton *radioButton_HighQuality;
    QRadioButton *radioButton_HighConsistency;
    QFrame *line;
    QLabel *label_15;
    QCheckBox *checkBox_JackSync;
    QDialogButtonBox *buttonBox;
    QButtonGroup *buttonGroup_Phase;
    QButtonGroup *buttonGroup_Transients;
    QButtonGroup *buttonGroup_Timestretch;
    QButtonGroup *buttonGroup_WindowSize;
    QButtonGroup *buttonGroup_Processing;
    QButtonGroup *buttonGroup_Formants;
    QButtonGroup *buttonGroup_PitchShifting;

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
        gridLayout = new QGridLayout(tab_TimeStretch);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label = new QLabel(tab_TimeStretch);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        radioButton_Offline = new QRadioButton(tab_TimeStretch);
        buttonGroup_Processing = new QButtonGroup(AudioSetupDialog);
        buttonGroup_Processing->setObjectName(QString::fromUtf8("buttonGroup_Processing"));
        buttonGroup_Processing->addButton(radioButton_Offline);
        radioButton_Offline->setObjectName(QString::fromUtf8("radioButton_Offline"));
        radioButton_Offline->setChecked(true);

        gridLayout->addWidget(radioButton_Offline, 0, 1, 1, 1);

        radioButton_RealTime = new QRadioButton(tab_TimeStretch);
        buttonGroup_Processing->addButton(radioButton_RealTime);
        radioButton_RealTime->setObjectName(QString::fromUtf8("radioButton_RealTime"));

        gridLayout->addWidget(radioButton_RealTime, 0, 2, 1, 1);

        label_2 = new QLabel(tab_TimeStretch);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        radioButton_Elastic = new QRadioButton(tab_TimeStretch);
        buttonGroup_Timestretch = new QButtonGroup(AudioSetupDialog);
        buttonGroup_Timestretch->setObjectName(QString::fromUtf8("buttonGroup_Timestretch"));
        buttonGroup_Timestretch->addButton(radioButton_Elastic);
        radioButton_Elastic->setObjectName(QString::fromUtf8("radioButton_Elastic"));
        radioButton_Elastic->setChecked(true);

        gridLayout->addWidget(radioButton_Elastic, 1, 1, 1, 1);

        radioButton_Precise = new QRadioButton(tab_TimeStretch);
        buttonGroup_Timestretch->addButton(radioButton_Precise);
        radioButton_Precise->setObjectName(QString::fromUtf8("radioButton_Precise"));

        gridLayout->addWidget(radioButton_Precise, 1, 2, 1, 1);

        label_3 = new QLabel(tab_TimeStretch);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 2, 0, 1, 1);

        radioButton_Crisp = new QRadioButton(tab_TimeStretch);
        buttonGroup_Transients = new QButtonGroup(AudioSetupDialog);
        buttonGroup_Transients->setObjectName(QString::fromUtf8("buttonGroup_Transients"));
        buttonGroup_Transients->addButton(radioButton_Crisp);
        radioButton_Crisp->setObjectName(QString::fromUtf8("radioButton_Crisp"));
        radioButton_Crisp->setChecked(true);

        gridLayout->addWidget(radioButton_Crisp, 2, 1, 1, 1);

        radioButton_Mixed = new QRadioButton(tab_TimeStretch);
        buttonGroup_Transients->addButton(radioButton_Mixed);
        radioButton_Mixed->setObjectName(QString::fromUtf8("radioButton_Mixed"));

        gridLayout->addWidget(radioButton_Mixed, 2, 2, 1, 1);

        radioButton_Smooth = new QRadioButton(tab_TimeStretch);
        buttonGroup_Transients->addButton(radioButton_Smooth);
        radioButton_Smooth->setObjectName(QString::fromUtf8("radioButton_Smooth"));

        gridLayout->addWidget(radioButton_Smooth, 2, 3, 1, 1);

        label_4 = new QLabel(tab_TimeStretch);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        radioButton_Laminar = new QRadioButton(tab_TimeStretch);
        buttonGroup_Phase = new QButtonGroup(AudioSetupDialog);
        buttonGroup_Phase->setObjectName(QString::fromUtf8("buttonGroup_Phase"));
        buttonGroup_Phase->addButton(radioButton_Laminar);
        radioButton_Laminar->setObjectName(QString::fromUtf8("radioButton_Laminar"));
        radioButton_Laminar->setChecked(true);

        gridLayout->addWidget(radioButton_Laminar, 3, 1, 1, 1);

        radioButton_Independent = new QRadioButton(tab_TimeStretch);
        buttonGroup_Phase->addButton(radioButton_Independent);
        radioButton_Independent->setObjectName(QString::fromUtf8("radioButton_Independent"));

        gridLayout->addWidget(radioButton_Independent, 3, 2, 1, 1);

        label_6 = new QLabel(tab_TimeStretch);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 4, 0, 1, 1);

        radioButton_Standard = new QRadioButton(tab_TimeStretch);
        buttonGroup_WindowSize = new QButtonGroup(AudioSetupDialog);
        buttonGroup_WindowSize->setObjectName(QString::fromUtf8("buttonGroup_WindowSize"));
        buttonGroup_WindowSize->addButton(radioButton_Standard);
        radioButton_Standard->setObjectName(QString::fromUtf8("radioButton_Standard"));
        radioButton_Standard->setChecked(true);

        gridLayout->addWidget(radioButton_Standard, 4, 1, 1, 1);

        radioButton_Short = new QRadioButton(tab_TimeStretch);
        buttonGroup_WindowSize->addButton(radioButton_Short);
        radioButton_Short->setObjectName(QString::fromUtf8("radioButton_Short"));

        gridLayout->addWidget(radioButton_Short, 4, 2, 1, 1);

        radioButton_Long = new QRadioButton(tab_TimeStretch);
        buttonGroup_WindowSize->addButton(radioButton_Long);
        radioButton_Long->setObjectName(QString::fromUtf8("radioButton_Long"));

        gridLayout->addWidget(radioButton_Long, 4, 3, 1, 1);

        label_7 = new QLabel(tab_TimeStretch);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 5, 0, 1, 1);

        radioButton_Shifted = new QRadioButton(tab_TimeStretch);
        buttonGroup_Formants = new QButtonGroup(AudioSetupDialog);
        buttonGroup_Formants->setObjectName(QString::fromUtf8("buttonGroup_Formants"));
        buttonGroup_Formants->addButton(radioButton_Shifted);
        radioButton_Shifted->setObjectName(QString::fromUtf8("radioButton_Shifted"));
        radioButton_Shifted->setChecked(true);

        gridLayout->addWidget(radioButton_Shifted, 5, 1, 1, 1);

        radioButton_Preserved = new QRadioButton(tab_TimeStretch);
        buttonGroup_Formants->addButton(radioButton_Preserved);
        radioButton_Preserved->setObjectName(QString::fromUtf8("radioButton_Preserved"));

        gridLayout->addWidget(radioButton_Preserved, 5, 2, 1, 1);

        label_8 = new QLabel(tab_TimeStretch);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout->addWidget(label_8, 6, 0, 1, 1);

        radioButton_HighSpeed = new QRadioButton(tab_TimeStretch);
        buttonGroup_PitchShifting = new QButtonGroup(AudioSetupDialog);
        buttonGroup_PitchShifting->setObjectName(QString::fromUtf8("buttonGroup_PitchShifting"));
        buttonGroup_PitchShifting->addButton(radioButton_HighSpeed);
        radioButton_HighSpeed->setObjectName(QString::fromUtf8("radioButton_HighSpeed"));
        radioButton_HighSpeed->setEnabled(false);
        radioButton_HighSpeed->setChecked(true);

        gridLayout->addWidget(radioButton_HighSpeed, 6, 1, 1, 1);

        radioButton_HighQuality = new QRadioButton(tab_TimeStretch);
        buttonGroup_PitchShifting->addButton(radioButton_HighQuality);
        radioButton_HighQuality->setObjectName(QString::fromUtf8("radioButton_HighQuality"));
        radioButton_HighQuality->setEnabled(false);

        gridLayout->addWidget(radioButton_HighQuality, 6, 2, 1, 1);

        radioButton_HighConsistency = new QRadioButton(tab_TimeStretch);
        buttonGroup_PitchShifting->addButton(radioButton_HighConsistency);
        radioButton_HighConsistency->setObjectName(QString::fromUtf8("radioButton_HighConsistency"));
        radioButton_HighConsistency->setEnabled(false);

        gridLayout->addWidget(radioButton_HighConsistency, 6, 3, 1, 1);

        line = new QFrame(tab_TimeStretch);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 7, 0, 1, 4);

        label_15 = new QLabel(tab_TimeStretch);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        gridLayout->addWidget(label_15, 8, 0, 1, 1);

        checkBox_JackSync = new QCheckBox(tab_TimeStretch);
        checkBox_JackSync->setObjectName(QString::fromUtf8("checkBox_JackSync"));
        checkBox_JackSync->setEnabled(false);

        gridLayout->addWidget(checkBox_JackSync, 8, 1, 1, 1);

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
        label_AudioBackend->setText(QApplication::translate("AudioSetupDialog", "Audio Back-end:", 0, QApplication::UnicodeUTF8));
        label_AudioDevice->setText(QApplication::translate("AudioSetupDialog", "Audio Device:", 0, QApplication::UnicodeUTF8));
        pushButton_TestTone->setText(QApplication::translate("AudioSetupDialog", "Test Tone", 0, QApplication::UnicodeUTF8));
        label_OutputChannels->setText(QApplication::translate("AudioSetupDialog", "Output Channels:", 0, QApplication::UnicodeUTF8));
        label_SampleRate->setText(QApplication::translate("AudioSetupDialog", "Sample Rate:", 0, QApplication::UnicodeUTF8));
        label_BufferSize->setText(QApplication::translate("AudioSetupDialog", "Audio Buffer Size:", 0, QApplication::UnicodeUTF8));
        label_MidiInput->setText(QApplication::translate("AudioSetupDialog", "MIDI Input:", 0, QApplication::UnicodeUTF8));
        checkBox_MidiInputTestTone->setText(QApplication::translate("AudioSetupDialog", "Enable MIDI Input Test Tone", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_AudioSetup), QApplication::translate("AudioSetupDialog", "Audio Setup", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("AudioSetupDialog", "Processing:", 0, QApplication::UnicodeUTF8));
        radioButton_Offline->setText(QApplication::translate("AudioSetupDialog", "Offline", 0, QApplication::UnicodeUTF8));
        radioButton_RealTime->setText(QApplication::translate("AudioSetupDialog", "Real-time", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("AudioSetupDialog", "Stretch Profile:", 0, QApplication::UnicodeUTF8));
        radioButton_Elastic->setText(QApplication::translate("AudioSetupDialog", "Elastic", 0, QApplication::UnicodeUTF8));
        radioButton_Precise->setText(QApplication::translate("AudioSetupDialog", "Precise", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("AudioSetupDialog", "Transients:", 0, QApplication::UnicodeUTF8));
        radioButton_Crisp->setText(QApplication::translate("AudioSetupDialog", "Crisp", 0, QApplication::UnicodeUTF8));
        radioButton_Mixed->setText(QApplication::translate("AudioSetupDialog", "Mixed", 0, QApplication::UnicodeUTF8));
        radioButton_Smooth->setText(QApplication::translate("AudioSetupDialog", "Smooth", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("AudioSetupDialog", "Phase:", 0, QApplication::UnicodeUTF8));
        radioButton_Laminar->setText(QApplication::translate("AudioSetupDialog", "Laminar", 0, QApplication::UnicodeUTF8));
        radioButton_Independent->setText(QApplication::translate("AudioSetupDialog", "Independent", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("AudioSetupDialog", "Window Size:", 0, QApplication::UnicodeUTF8));
        radioButton_Standard->setText(QApplication::translate("AudioSetupDialog", "Standard", 0, QApplication::UnicodeUTF8));
        radioButton_Short->setText(QApplication::translate("AudioSetupDialog", "Short", 0, QApplication::UnicodeUTF8));
        radioButton_Long->setText(QApplication::translate("AudioSetupDialog", "Long", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("AudioSetupDialog", "Formants:", 0, QApplication::UnicodeUTF8));
        radioButton_Shifted->setText(QApplication::translate("AudioSetupDialog", "Shifted", 0, QApplication::UnicodeUTF8));
        radioButton_Preserved->setText(QApplication::translate("AudioSetupDialog", "Preserved", 0, QApplication::UnicodeUTF8));
        label_8->setText(QApplication::translate("AudioSetupDialog", "Pitch Shifting:", 0, QApplication::UnicodeUTF8));
        radioButton_HighSpeed->setText(QApplication::translate("AudioSetupDialog", "High Speed", 0, QApplication::UnicodeUTF8));
        radioButton_HighQuality->setText(QApplication::translate("AudioSetupDialog", "High Quality", 0, QApplication::UnicodeUTF8));
        radioButton_HighConsistency->setText(QApplication::translate("AudioSetupDialog", "High Consistency", 0, QApplication::UnicodeUTF8));
        label_15->setText(QApplication::translate("AudioSetupDialog", "JACK Sync:", 0, QApplication::UnicodeUTF8));
        checkBox_JackSync->setText(QApplication::translate("AudioSetupDialog", "Enable", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_TimeStretch), QApplication::translate("AudioSetupDialog", "Time Stretch", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class AudioSetupDialog: public Ui_AudioSetupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUDIOSETUPDIALOG_H
