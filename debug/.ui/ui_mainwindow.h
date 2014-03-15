/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sat Mar 15 15:42:11 2014
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLCDNumber>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "src/wavegraphicsview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionImport_Audio_File;
    QAction *actionQuit;
    QAction *actionAudio_Setup;
    QAction *actionOpen_Project;
    QAction *actionSave_Project;
    QAction *actionClose_Project;
    QAction *actionExport_As;
    QAction *actionUndo;
    QAction *actionRedo;
    QAction *actionAdd_Slice_Point;
    QAction *actionDelete;
    QAction *actionReverse;
    QAction *actionEnvelope;
    QAction *actionUser_Interface;
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionJoin;
    QAction *actionSelect_All;
    QAction *actionClear_Selection;
    QAction *actionApply_Gain;
    QAction *actionNormalise;
    QAction *actionApply_Ramp;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_Play;
    QPushButton *pushButton_Loop;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinBox_BPM;
    QPushButton *pushButton_CalcBPM;
    QCheckBox *checkBox_JackSync;
    QCheckBox *checkBox_TimeStretch;
    QCheckBox *checkBox_CorrectPitch;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton_Slice;
    QLabel *label;
    QLCDNumber *lcdNumber_Threshold;
    QSlider *horizontalSlider_Threshold;
    QPushButton *pushButton_FindOnsets;
    QPushButton *pushButton_FindBeats;
    QCheckBox *checkBox_AdvancedOptions;
    QSpacerItem *horizontalSpacer_3;
    QHBoxLayout *horizontalLayout_AdvancedOptions;
    QLabel *label_7;
    QComboBox *comboBox_DetectMethod;
    QLabel *label_4;
    QComboBox *comboBox_WindowSize;
    QLabel *label_5;
    QComboBox *comboBox_HopSize;
    QSpacerItem *horizontalSpacer_2;
    WaveGraphicsView *waveGraphicsView;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_6;
    QSlider *zoomSlider;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuOptions;
    QMenu *menuEdit;
    QMenu *menuHelp;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(878, 600);
        actionImport_Audio_File = new QAction(MainWindow);
        actionImport_Audio_File->setObjectName(QString::fromUtf8("actionImport_Audio_File"));
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionAudio_Setup = new QAction(MainWindow);
        actionAudio_Setup->setObjectName(QString::fromUtf8("actionAudio_Setup"));
        actionOpen_Project = new QAction(MainWindow);
        actionOpen_Project->setObjectName(QString::fromUtf8("actionOpen_Project"));
        actionSave_Project = new QAction(MainWindow);
        actionSave_Project->setObjectName(QString::fromUtf8("actionSave_Project"));
        actionClose_Project = new QAction(MainWindow);
        actionClose_Project->setObjectName(QString::fromUtf8("actionClose_Project"));
        actionExport_As = new QAction(MainWindow);
        actionExport_As->setObjectName(QString::fromUtf8("actionExport_As"));
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        actionAdd_Slice_Point = new QAction(MainWindow);
        actionAdd_Slice_Point->setObjectName(QString::fromUtf8("actionAdd_Slice_Point"));
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
        actionReverse = new QAction(MainWindow);
        actionReverse->setObjectName(QString::fromUtf8("actionReverse"));
        actionEnvelope = new QAction(MainWindow);
        actionEnvelope->setObjectName(QString::fromUtf8("actionEnvelope"));
        actionUser_Interface = new QAction(MainWindow);
        actionUser_Interface->setObjectName(QString::fromUtf8("actionUser_Interface"));
        actionHelp = new QAction(MainWindow);
        actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionJoin = new QAction(MainWindow);
        actionJoin->setObjectName(QString::fromUtf8("actionJoin"));
        actionSelect_All = new QAction(MainWindow);
        actionSelect_All->setObjectName(QString::fromUtf8("actionSelect_All"));
        actionClear_Selection = new QAction(MainWindow);
        actionClear_Selection->setObjectName(QString::fromUtf8("actionClear_Selection"));
        actionApply_Gain = new QAction(MainWindow);
        actionApply_Gain->setObjectName(QString::fromUtf8("actionApply_Gain"));
        actionNormalise = new QAction(MainWindow);
        actionNormalise->setObjectName(QString::fromUtf8("actionNormalise"));
        actionApply_Ramp = new QAction(MainWindow);
        actionApply_Ramp->setObjectName(QString::fromUtf8("actionApply_Ramp"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton_Play = new QPushButton(centralWidget);
        pushButton_Play->setObjectName(QString::fromUtf8("pushButton_Play"));
        pushButton_Play->setEnabled(false);

        horizontalLayout_2->addWidget(pushButton_Play);

        pushButton_Loop = new QPushButton(centralWidget);
        pushButton_Loop->setObjectName(QString::fromUtf8("pushButton_Loop"));
        pushButton_Loop->setEnabled(false);

        horizontalLayout_2->addWidget(pushButton_Loop);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label_3);

        doubleSpinBox_BPM = new QDoubleSpinBox(centralWidget);
        doubleSpinBox_BPM->setObjectName(QString::fromUtf8("doubleSpinBox_BPM"));
        doubleSpinBox_BPM->setMaximum(999.99);

        horizontalLayout_2->addWidget(doubleSpinBox_BPM);

        pushButton_CalcBPM = new QPushButton(centralWidget);
        pushButton_CalcBPM->setObjectName(QString::fromUtf8("pushButton_CalcBPM"));
        pushButton_CalcBPM->setEnabled(false);

        horizontalLayout_2->addWidget(pushButton_CalcBPM);

        checkBox_JackSync = new QCheckBox(centralWidget);
        checkBox_JackSync->setObjectName(QString::fromUtf8("checkBox_JackSync"));
        checkBox_JackSync->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_2->addWidget(checkBox_JackSync);

        checkBox_TimeStretch = new QCheckBox(centralWidget);
        checkBox_TimeStretch->setObjectName(QString::fromUtf8("checkBox_TimeStretch"));
        checkBox_TimeStretch->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_2->addWidget(checkBox_TimeStretch);

        checkBox_CorrectPitch = new QCheckBox(centralWidget);
        checkBox_CorrectPitch->setObjectName(QString::fromUtf8("checkBox_CorrectPitch"));
        checkBox_CorrectPitch->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_2->addWidget(checkBox_CorrectPitch);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pushButton_Slice = new QPushButton(centralWidget);
        pushButton_Slice->setObjectName(QString::fromUtf8("pushButton_Slice"));
        pushButton_Slice->setEnabled(false);

        horizontalLayout_4->addWidget(pushButton_Slice);

        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label);

        lcdNumber_Threshold = new QLCDNumber(centralWidget);
        lcdNumber_Threshold->setObjectName(QString::fromUtf8("lcdNumber_Threshold"));
        lcdNumber_Threshold->setMaximumSize(QSize(16777215, 40));
        lcdNumber_Threshold->setFrameShadow(QFrame::Sunken);
        lcdNumber_Threshold->setSmallDecimalPoint(true);
        lcdNumber_Threshold->setNumDigits(5);
        lcdNumber_Threshold->setSegmentStyle(QLCDNumber::Flat);
        lcdNumber_Threshold->setProperty("value", QVariant(0.3));

        horizontalLayout_4->addWidget(lcdNumber_Threshold);

        horizontalSlider_Threshold = new QSlider(centralWidget);
        horizontalSlider_Threshold->setObjectName(QString::fromUtf8("horizontalSlider_Threshold"));
        horizontalSlider_Threshold->setMinimum(1);
        horizontalSlider_Threshold->setMaximum(1000);
        horizontalSlider_Threshold->setValue(300);
        horizontalSlider_Threshold->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(horizontalSlider_Threshold);

        pushButton_FindOnsets = new QPushButton(centralWidget);
        pushButton_FindOnsets->setObjectName(QString::fromUtf8("pushButton_FindOnsets"));
        pushButton_FindOnsets->setEnabled(false);

        horizontalLayout_4->addWidget(pushButton_FindOnsets);

        pushButton_FindBeats = new QPushButton(centralWidget);
        pushButton_FindBeats->setObjectName(QString::fromUtf8("pushButton_FindBeats"));
        pushButton_FindBeats->setEnabled(false);

        horizontalLayout_4->addWidget(pushButton_FindBeats);

        checkBox_AdvancedOptions = new QCheckBox(centralWidget);
        checkBox_AdvancedOptions->setObjectName(QString::fromUtf8("checkBox_AdvancedOptions"));
        checkBox_AdvancedOptions->setChecked(true);

        horizontalLayout_4->addWidget(checkBox_AdvancedOptions);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_AdvancedOptions = new QHBoxLayout();
        horizontalLayout_AdvancedOptions->setSpacing(6);
        horizontalLayout_AdvancedOptions->setObjectName(QString::fromUtf8("horizontalLayout_AdvancedOptions"));
        horizontalLayout_AdvancedOptions->setSizeConstraint(QLayout::SetDefaultConstraint);
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_AdvancedOptions->addWidget(label_7);

        comboBox_DetectMethod = new QComboBox(centralWidget);
        comboBox_DetectMethod->setObjectName(QString::fromUtf8("comboBox_DetectMethod"));

        horizontalLayout_AdvancedOptions->addWidget(comboBox_DetectMethod);

        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_AdvancedOptions->addWidget(label_4);

        comboBox_WindowSize = new QComboBox(centralWidget);
        comboBox_WindowSize->setObjectName(QString::fromUtf8("comboBox_WindowSize"));

        horizontalLayout_AdvancedOptions->addWidget(comboBox_WindowSize);

        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_AdvancedOptions->addWidget(label_5);

        comboBox_HopSize = new QComboBox(centralWidget);
        comboBox_HopSize->setObjectName(QString::fromUtf8("comboBox_HopSize"));

        horizontalLayout_AdvancedOptions->addWidget(comboBox_HopSize);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_AdvancedOptions->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_AdvancedOptions);

        waveGraphicsView = new WaveGraphicsView(centralWidget);
        waveGraphicsView->setObjectName(QString::fromUtf8("waveGraphicsView"));

        verticalLayout->addWidget(waveGraphicsView);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_5->addWidget(label_6);

        zoomSlider = new QSlider(centralWidget);
        zoomSlider->setObjectName(QString::fromUtf8("zoomSlider"));
        zoomSlider->setMinimum(1);
        zoomSlider->setMaximum(50);
        zoomSlider->setOrientation(Qt::Horizontal);
        zoomSlider->setTickPosition(QSlider::TicksBothSides);
        zoomSlider->setTickInterval(1);

        horizontalLayout_5->addWidget(zoomSlider);


        verticalLayout->addLayout(horizontalLayout_5);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 878, 24));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuHelp = new QMenu(menuBar);
        menuHelp->setObjectName(QString::fromUtf8("menuHelp"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen_Project);
        menuFile->addAction(actionSave_Project);
        menuFile->addAction(actionClose_Project);
        menuFile->addSeparator();
        menuFile->addAction(actionImport_Audio_File);
        menuFile->addAction(actionExport_As);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuOptions->addAction(actionAudio_Setup);
        menuOptions->addAction(actionUser_Interface);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All);
        menuEdit->addAction(actionClear_Selection);
        menuEdit->addAction(actionDelete);
        menuEdit->addSeparator();
        menuEdit->addAction(actionAdd_Slice_Point);
        menuEdit->addSeparator();
        menuEdit->addAction(actionApply_Gain);
        menuEdit->addAction(actionApply_Ramp);
        menuEdit->addAction(actionEnvelope);
        menuEdit->addAction(actionJoin);
        menuEdit->addAction(actionNormalise);
        menuEdit->addAction(actionReverse);
        menuHelp->addAction(actionHelp);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Shuriken Beat Slicer", 0, QApplication::UnicodeUTF8));
        actionImport_Audio_File->setText(QApplication::translate("MainWindow", "Import Audio File", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionImport_Audio_File->setToolTip(QApplication::translate("MainWindow", "Import Audio File", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionImport_Audio_File->setShortcut(QApplication::translate("MainWindow", "Ctrl+I", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("MainWindow", "Quit", 0, QApplication::UnicodeUTF8));
        actionQuit->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0, QApplication::UnicodeUTF8));
        actionAudio_Setup->setText(QApplication::translate("MainWindow", "Audio Setup", 0, QApplication::UnicodeUTF8));
        actionOpen_Project->setText(QApplication::translate("MainWindow", "Open Project", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionOpen_Project->setToolTip(QApplication::translate("MainWindow", "Open Project", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionOpen_Project->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionSave_Project->setText(QApplication::translate("MainWindow", "Save Project", 0, QApplication::UnicodeUTF8));
        actionSave_Project->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionClose_Project->setText(QApplication::translate("MainWindow", "Close Project", 0, QApplication::UnicodeUTF8));
        actionExport_As->setText(QApplication::translate("MainWindow", "Export As...", 0, QApplication::UnicodeUTF8));
        actionUndo->setText(QApplication::translate("MainWindow", "Undo", 0, QApplication::UnicodeUTF8));
        actionUndo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
        actionRedo->setText(QApplication::translate("MainWindow", "Redo", 0, QApplication::UnicodeUTF8));
        actionAdd_Slice_Point->setText(QApplication::translate("MainWindow", "Add Slice Point", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionAdd_Slice_Point->setToolTip(QApplication::translate("MainWindow", "Add Slice Point", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionAdd_Slice_Point->setShortcut(QApplication::translate("MainWindow", "Ins", 0, QApplication::UnicodeUTF8));
        actionDelete->setText(QApplication::translate("MainWindow", "Delete", 0, QApplication::UnicodeUTF8));
        actionDelete->setShortcut(QApplication::translate("MainWindow", "Del", 0, QApplication::UnicodeUTF8));
        actionReverse->setText(QApplication::translate("MainWindow", "Reverse", 0, QApplication::UnicodeUTF8));
        actionEnvelope->setText(QApplication::translate("MainWindow", "Envelope", 0, QApplication::UnicodeUTF8));
        actionUser_Interface->setText(QApplication::translate("MainWindow", "User Interface", 0, QApplication::UnicodeUTF8));
        actionHelp->setText(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("MainWindow", "About", 0, QApplication::UnicodeUTF8));
        actionJoin->setText(QApplication::translate("MainWindow", "Join", 0, QApplication::UnicodeUTF8));
        actionSelect_All->setText(QApplication::translate("MainWindow", "Select All", 0, QApplication::UnicodeUTF8));
        actionSelect_All->setShortcut(QApplication::translate("MainWindow", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionClear_Selection->setText(QApplication::translate("MainWindow", "Clear Selection", 0, QApplication::UnicodeUTF8));
        actionApply_Gain->setText(QApplication::translate("MainWindow", "Apply Gain", 0, QApplication::UnicodeUTF8));
        actionNormalise->setText(QApplication::translate("MainWindow", "Normalise", 0, QApplication::UnicodeUTF8));
        actionApply_Ramp->setText(QApplication::translate("MainWindow", "Apply Ramp", 0, QApplication::UnicodeUTF8));
        pushButton_Play->setText(QApplication::translate("MainWindow", "Play", 0, QApplication::UnicodeUTF8));
        pushButton_Loop->setText(QApplication::translate("MainWindow", "Loop", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "BPM:", 0, QApplication::UnicodeUTF8));
        pushButton_CalcBPM->setText(QApplication::translate("MainWindow", "Calc", 0, QApplication::UnicodeUTF8));
        checkBox_JackSync->setText(QApplication::translate("MainWindow", "JACK Sync", 0, QApplication::UnicodeUTF8));
        checkBox_TimeStretch->setText(QApplication::translate("MainWindow", "Time Stretch", 0, QApplication::UnicodeUTF8));
        checkBox_CorrectPitch->setText(QApplication::translate("MainWindow", "Correct Pitch", 0, QApplication::UnicodeUTF8));
        pushButton_Slice->setText(QApplication::translate("MainWindow", "Slice", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Threshold:", 0, QApplication::UnicodeUTF8));
        pushButton_FindOnsets->setText(QApplication::translate("MainWindow", "Find Onsets", 0, QApplication::UnicodeUTF8));
        pushButton_FindBeats->setText(QApplication::translate("MainWindow", "Find Beats", 0, QApplication::UnicodeUTF8));
        checkBox_AdvancedOptions->setText(QApplication::translate("MainWindow", "Show Advanced Options", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Detection Method:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Window Size:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "Window Overlap:", 0, QApplication::UnicodeUTF8));
        label_6->setText(QApplication::translate("MainWindow", "Zoom:", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuOptions->setTitle(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
        menuEdit->setTitle(QApplication::translate("MainWindow", "Edit", 0, QApplication::UnicodeUTF8));
        menuHelp->setTitle(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
