/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Sat Jul 12 09:37:11 2014
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
#include <QtGui/QFrame>
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
    QAction *actionOptions;
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
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionJoin;
    QAction *actionSelect_All;
    QAction *actionSelect_None;
    QAction *actionApply_Gain;
    QAction *actionNormalise;
    QAction *actionZoom_In;
    QAction *actionZoom_Out;
    QAction *actionZoom_Original;
    QAction *actionMove;
    QAction *actionSelect;
    QAction *actionSplit;
    QAction *actionAudition;
    QAction *actionSave_As;
    QAction *actionApply_Gain_Ramp;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_Play;
    QPushButton *pushButton_Stop;
    QFrame *line;
    QLabel *label_3;
    QDoubleSpinBox *doubleSpinBox_OriginalBPM;
    QPushButton *pushButton_CalcBPM;
    QFrame *line_2;
    QLabel *label_2;
    QLabel *label_JackSync;
    QDoubleSpinBox *doubleSpinBox_NewBPM;
    QPushButton *pushButton_Apply;
    QFrame *line_3;
    QCheckBox *checkBox_TimeStretch;
    QCheckBox *checkBox_PitchCorrection;
    QPushButton *pushButton_TimestretchOptions;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_Slice;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_7;
    QComboBox *comboBox_DetectMethod;
    QLabel *label_4;
    QComboBox *comboBox_WindowSize;
    QLabel *label_5;
    QComboBox *comboBox_HopSize;
    QSpacerItem *horizontalSpacer_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QLCDNumber *lcdNumber_Threshold;
    QSlider *horizontalSlider_Threshold;
    QPushButton *pushButton_FindOnsets;
    QPushButton *pushButton_FindBeats;
    QSpacerItem *horizontalSpacer_3;
    WaveGraphicsView *waveGraphicsView;
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
        MainWindow->resize(1051, 618);
        actionImport_Audio_File = new QAction(MainWindow);
        actionImport_Audio_File->setObjectName(QString::fromUtf8("actionImport_Audio_File"));
        QIcon icon;
        icon.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-import.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionImport_Audio_File->setIcon(icon);
        actionQuit = new QAction(MainWindow);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionOptions = new QAction(MainWindow);
        actionOptions->setObjectName(QString::fromUtf8("actionOptions"));
        actionOpen_Project = new QAction(MainWindow);
        actionOpen_Project->setObjectName(QString::fromUtf8("actionOpen_Project"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-open.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen_Project->setIcon(icon1);
        actionSave_Project = new QAction(MainWindow);
        actionSave_Project->setObjectName(QString::fromUtf8("actionSave_Project"));
        actionSave_Project->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-save.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_Project->setIcon(icon2);
        actionClose_Project = new QAction(MainWindow);
        actionClose_Project->setObjectName(QString::fromUtf8("actionClose_Project"));
        actionClose_Project->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-close.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionClose_Project->setIcon(icon3);
        actionExport_As = new QAction(MainWindow);
        actionExport_As->setObjectName(QString::fromUtf8("actionExport_As"));
        actionExport_As->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-export.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionExport_As->setIcon(icon4);
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionUndo->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/edit-undo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionUndo->setIcon(icon5);
        actionRedo = new QAction(MainWindow);
        actionRedo->setObjectName(QString::fromUtf8("actionRedo"));
        actionRedo->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/edit-redo.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionRedo->setIcon(icon6);
        actionAdd_Slice_Point = new QAction(MainWindow);
        actionAdd_Slice_Point->setObjectName(QString::fromUtf8("actionAdd_Slice_Point"));
        actionAdd_Slice_Point->setEnabled(false);
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
        actionDelete->setEnabled(false);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/edit-delete.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionDelete->setIcon(icon7);
        actionReverse = new QAction(MainWindow);
        actionReverse->setObjectName(QString::fromUtf8("actionReverse"));
        actionReverse->setEnabled(false);
        actionEnvelope = new QAction(MainWindow);
        actionEnvelope->setObjectName(QString::fromUtf8("actionEnvelope"));
        actionEnvelope->setEnabled(false);
        actionHelp = new QAction(MainWindow);
        actionHelp->setObjectName(QString::fromUtf8("actionHelp"));
        actionHelp->setEnabled(false);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/help-contents.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionHelp->setIcon(icon8);
        actionAbout = new QAction(MainWindow);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        QIcon icon9;
        icon9.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/help-about.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAbout->setIcon(icon9);
        actionJoin = new QAction(MainWindow);
        actionJoin->setObjectName(QString::fromUtf8("actionJoin"));
        actionJoin->setEnabled(false);
        actionSelect_All = new QAction(MainWindow);
        actionSelect_All->setObjectName(QString::fromUtf8("actionSelect_All"));
        actionSelect_All->setEnabled(false);
        actionSelect_None = new QAction(MainWindow);
        actionSelect_None->setObjectName(QString::fromUtf8("actionSelect_None"));
        actionSelect_None->setEnabled(false);
        actionApply_Gain = new QAction(MainWindow);
        actionApply_Gain->setObjectName(QString::fromUtf8("actionApply_Gain"));
        actionApply_Gain->setEnabled(false);
        actionNormalise = new QAction(MainWindow);
        actionNormalise->setObjectName(QString::fromUtf8("actionNormalise"));
        actionNormalise->setEnabled(false);
        actionZoom_In = new QAction(MainWindow);
        actionZoom_In->setObjectName(QString::fromUtf8("actionZoom_In"));
        actionZoom_In->setEnabled(false);
        QIcon icon10;
        icon10.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/zoom-in.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_In->setIcon(icon10);
        actionZoom_Out = new QAction(MainWindow);
        actionZoom_Out->setObjectName(QString::fromUtf8("actionZoom_Out"));
        actionZoom_Out->setEnabled(false);
        QIcon icon11;
        icon11.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/zoom-out.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Out->setIcon(icon11);
        actionZoom_Original = new QAction(MainWindow);
        actionZoom_Original->setObjectName(QString::fromUtf8("actionZoom_Original"));
        actionZoom_Original->setEnabled(false);
        QIcon icon12;
        icon12.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/zoom-original.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionZoom_Original->setIcon(icon12);
        actionMove = new QAction(MainWindow);
        actionMove->setObjectName(QString::fromUtf8("actionMove"));
        actionMove->setEnabled(false);
        QIcon icon13;
        icon13.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/edit-select.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionMove->setIcon(icon13);
        actionSelect = new QAction(MainWindow);
        actionSelect->setObjectName(QString::fromUtf8("actionSelect"));
        actionSelect->setEnabled(false);
        QIcon icon14;
        icon14.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/kdenlive-select-all.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSelect->setIcon(icon14);
        actionSplit = new QAction(MainWindow);
        actionSplit->setObjectName(QString::fromUtf8("actionSplit"));
        actionSplit->setEnabled(false);
        actionAudition = new QAction(MainWindow);
        actionAudition->setObjectName(QString::fromUtf8("actionAudition"));
        actionAudition->setEnabled(false);
        QIcon icon15;
        icon15.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/text-speak.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionAudition->setIcon(icon15);
        actionSave_As = new QAction(MainWindow);
        actionSave_As->setObjectName(QString::fromUtf8("actionSave_As"));
        actionSave_As->setEnabled(false);
        QIcon icon16;
        icon16.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/document-save-as.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionSave_As->setIcon(icon16);
        actionApply_Gain_Ramp = new QAction(MainWindow);
        actionApply_Gain_Ramp->setObjectName(QString::fromUtf8("actionApply_Gain_Ramp"));
        actionApply_Gain_Ramp->setEnabled(false);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout_3 = new QVBoxLayout(centralWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton_Play = new QPushButton(centralWidget);
        pushButton_Play->setObjectName(QString::fromUtf8("pushButton_Play"));
        pushButton_Play->setEnabled(false);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_Play->sizePolicy().hasHeightForWidth());
        pushButton_Play->setSizePolicy(sizePolicy);
        QIcon icon17;
        icon17.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/media-playback-start.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Play->setIcon(icon17);

        horizontalLayout_2->addWidget(pushButton_Play);

        pushButton_Stop = new QPushButton(centralWidget);
        pushButton_Stop->setObjectName(QString::fromUtf8("pushButton_Stop"));
        pushButton_Stop->setEnabled(false);
        QIcon icon18;
        icon18.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/media-playback-stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Stop->setIcon(icon18);

        horizontalLayout_2->addWidget(pushButton_Stop);

        line = new QFrame(centralWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Sunken);

        horizontalLayout_2->addWidget(line);

        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(label_3);

        doubleSpinBox_OriginalBPM = new QDoubleSpinBox(centralWidget);
        doubleSpinBox_OriginalBPM->setObjectName(QString::fromUtf8("doubleSpinBox_OriginalBPM"));
        doubleSpinBox_OriginalBPM->setEnabled(false);
        doubleSpinBox_OriginalBPM->setMaximum(999.99);

        horizontalLayout_2->addWidget(doubleSpinBox_OriginalBPM);

        pushButton_CalcBPM = new QPushButton(centralWidget);
        pushButton_CalcBPM->setObjectName(QString::fromUtf8("pushButton_CalcBPM"));
        pushButton_CalcBPM->setEnabled(false);
        pushButton_CalcBPM->setMaximumSize(QSize(55, 16777215));

        horizontalLayout_2->addWidget(pushButton_CalcBPM);

        line_2 = new QFrame(centralWidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::VLine);
        line_2->setFrameShadow(QFrame::Sunken);

        horizontalLayout_2->addWidget(line_2);

        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        label_JackSync = new QLabel(centralWidget);
        label_JackSync->setObjectName(QString::fromUtf8("label_JackSync"));

        horizontalLayout_2->addWidget(label_JackSync);

        doubleSpinBox_NewBPM = new QDoubleSpinBox(centralWidget);
        doubleSpinBox_NewBPM->setObjectName(QString::fromUtf8("doubleSpinBox_NewBPM"));
        doubleSpinBox_NewBPM->setEnabled(false);
        doubleSpinBox_NewBPM->setMaximum(999.99);

        horizontalLayout_2->addWidget(doubleSpinBox_NewBPM);

        pushButton_Apply = new QPushButton(centralWidget);
        pushButton_Apply->setObjectName(QString::fromUtf8("pushButton_Apply"));
        pushButton_Apply->setEnabled(false);
        sizePolicy.setHeightForWidth(pushButton_Apply->sizePolicy().hasHeightForWidth());
        pushButton_Apply->setSizePolicy(sizePolicy);
        pushButton_Apply->setMaximumSize(QSize(55, 16777215));
        pushButton_Apply->setFlat(false);

        horizontalLayout_2->addWidget(pushButton_Apply);

        line_3 = new QFrame(centralWidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);

        horizontalLayout_2->addWidget(line_3);

        checkBox_TimeStretch = new QCheckBox(centralWidget);
        checkBox_TimeStretch->setObjectName(QString::fromUtf8("checkBox_TimeStretch"));
        checkBox_TimeStretch->setEnabled(false);
        checkBox_TimeStretch->setLayoutDirection(Qt::LeftToRight);
        checkBox_TimeStretch->setChecked(true);

        horizontalLayout_2->addWidget(checkBox_TimeStretch);

        checkBox_PitchCorrection = new QCheckBox(centralWidget);
        checkBox_PitchCorrection->setObjectName(QString::fromUtf8("checkBox_PitchCorrection"));
        checkBox_PitchCorrection->setEnabled(false);
        checkBox_PitchCorrection->setLayoutDirection(Qt::LeftToRight);
        checkBox_PitchCorrection->setChecked(true);

        horizontalLayout_2->addWidget(checkBox_PitchCorrection);

        pushButton_TimestretchOptions = new QPushButton(centralWidget);
        pushButton_TimestretchOptions->setObjectName(QString::fromUtf8("pushButton_TimestretchOptions"));
        pushButton_TimestretchOptions->setEnabled(false);

        horizontalLayout_2->addWidget(pushButton_TimestretchOptions);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);


        verticalLayout_3->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_Slice = new QPushButton(centralWidget);
        pushButton_Slice->setObjectName(QString::fromUtf8("pushButton_Slice"));
        pushButton_Slice->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(pushButton_Slice->sizePolicy().hasHeightForWidth());
        pushButton_Slice->setSizePolicy(sizePolicy1);
        QIcon icon19;
        icon19.addFile(QString::fromUtf8("../../../../usr/share/icons/oxygen/22x22/actions/transform-move.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_Slice->setIcon(icon19);

        horizontalLayout->addWidget(pushButton_Slice);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setSizeConstraint(QLayout::SetDefaultConstraint);
        label_7 = new QLabel(centralWidget);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_7);

        comboBox_DetectMethod = new QComboBox(centralWidget);
        comboBox_DetectMethod->setObjectName(QString::fromUtf8("comboBox_DetectMethod"));
        comboBox_DetectMethod->setEnabled(false);

        horizontalLayout_3->addWidget(comboBox_DetectMethod);

        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_3->addWidget(label_4);

        comboBox_WindowSize = new QComboBox(centralWidget);
        comboBox_WindowSize->setObjectName(QString::fromUtf8("comboBox_WindowSize"));
        comboBox_WindowSize->setEnabled(false);

        horizontalLayout_3->addWidget(comboBox_WindowSize);

        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_3->addWidget(label_5);

        comboBox_HopSize = new QComboBox(centralWidget);
        comboBox_HopSize->setObjectName(QString::fromUtf8("comboBox_HopSize"));
        comboBox_HopSize->setEnabled(false);

        horizontalLayout_3->addWidget(comboBox_HopSize);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_4->addWidget(label);

        lcdNumber_Threshold = new QLCDNumber(centralWidget);
        lcdNumber_Threshold->setObjectName(QString::fromUtf8("lcdNumber_Threshold"));
        lcdNumber_Threshold->setEnabled(false);
        lcdNumber_Threshold->setMaximumSize(QSize(16777215, 40));
        lcdNumber_Threshold->setFrameShadow(QFrame::Sunken);
        lcdNumber_Threshold->setSmallDecimalPoint(true);
        lcdNumber_Threshold->setNumDigits(5);
        lcdNumber_Threshold->setSegmentStyle(QLCDNumber::Flat);
        lcdNumber_Threshold->setProperty("value", QVariant(0.3));

        horizontalLayout_4->addWidget(lcdNumber_Threshold);

        horizontalSlider_Threshold = new QSlider(centralWidget);
        horizontalSlider_Threshold->setObjectName(QString::fromUtf8("horizontalSlider_Threshold"));
        horizontalSlider_Threshold->setEnabled(false);
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

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);


        verticalLayout->addLayout(horizontalLayout_4);


        horizontalLayout->addLayout(verticalLayout);


        verticalLayout_3->addLayout(horizontalLayout);

        waveGraphicsView = new WaveGraphicsView(centralWidget);
        waveGraphicsView->setObjectName(QString::fromUtf8("waveGraphicsView"));

        verticalLayout_3->addWidget(waveGraphicsView);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1051, 24));
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
        MainWindow->addToolBar(Qt::RightToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menuHelp->menuAction());
        menuFile->addAction(actionOpen_Project);
        menuFile->addAction(actionSave_Project);
        menuFile->addAction(actionSave_As);
        menuFile->addAction(actionClose_Project);
        menuFile->addSeparator();
        menuFile->addAction(actionImport_Audio_File);
        menuFile->addAction(actionExport_As);
        menuFile->addSeparator();
        menuFile->addAction(actionQuit);
        menuOptions->addAction(actionOptions);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(actionSelect_All);
        menuEdit->addAction(actionSelect_None);
        menuEdit->addAction(actionDelete);
        menuEdit->addSeparator();
        menuEdit->addAction(actionAdd_Slice_Point);
        menuEdit->addSeparator();
        menuEdit->addAction(actionApply_Gain);
        menuEdit->addAction(actionApply_Gain_Ramp);
        menuEdit->addAction(actionNormalise);
        menuEdit->addAction(actionEnvelope);
        menuEdit->addAction(actionJoin);
        menuEdit->addAction(actionSplit);
        menuEdit->addAction(actionReverse);
        menuHelp->addAction(actionHelp);
        menuHelp->addSeparator();
        menuHelp->addAction(actionAbout);
        mainToolBar->addAction(actionOpen_Project);
        mainToolBar->addAction(actionSave_Project);
        mainToolBar->addAction(actionClose_Project);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionImport_Audio_File);
        mainToolBar->addAction(actionExport_As);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionUndo);
        mainToolBar->addAction(actionRedo);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionMove);
        mainToolBar->addAction(actionSelect);
        mainToolBar->addAction(actionAudition);
        mainToolBar->addAction(actionDelete);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionZoom_Original);
        mainToolBar->addAction(actionZoom_Out);
        mainToolBar->addAction(actionZoom_In);

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
        actionOptions->setText(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionOptions->setToolTip(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionOpen_Project->setText(QApplication::translate("MainWindow", "Open Project", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionOpen_Project->setToolTip(QApplication::translate("MainWindow", "Open Project", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionOpen_Project->setShortcut(QApplication::translate("MainWindow", "Ctrl+O", 0, QApplication::UnicodeUTF8));
        actionSave_Project->setText(QApplication::translate("MainWindow", "Save Project", 0, QApplication::UnicodeUTF8));
        actionSave_Project->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0, QApplication::UnicodeUTF8));
        actionClose_Project->setText(QApplication::translate("MainWindow", "Close Project", 0, QApplication::UnicodeUTF8));
        actionExport_As->setText(QApplication::translate("MainWindow", "Export As...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionExport_As->setToolTip(QApplication::translate("MainWindow", "Export As...", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionExport_As->setShortcut(QApplication::translate("MainWindow", "Ctrl+E", 0, QApplication::UnicodeUTF8));
        actionUndo->setText(QApplication::translate("MainWindow", "Undo", 0, QApplication::UnicodeUTF8));
        actionUndo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Z", 0, QApplication::UnicodeUTF8));
        actionRedo->setText(QApplication::translate("MainWindow", "Redo", 0, QApplication::UnicodeUTF8));
        actionRedo->setShortcut(QApplication::translate("MainWindow", "Ctrl+Shift+Z", 0, QApplication::UnicodeUTF8));
        actionAdd_Slice_Point->setText(QApplication::translate("MainWindow", "Add Slice Point", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionAdd_Slice_Point->setToolTip(QApplication::translate("MainWindow", "Add Slice Point", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionAdd_Slice_Point->setShortcut(QApplication::translate("MainWindow", "Ins", 0, QApplication::UnicodeUTF8));
        actionDelete->setText(QApplication::translate("MainWindow", "Delete", 0, QApplication::UnicodeUTF8));
        actionDelete->setShortcut(QApplication::translate("MainWindow", "Del", 0, QApplication::UnicodeUTF8));
        actionReverse->setText(QApplication::translate("MainWindow", "Reverse", 0, QApplication::UnicodeUTF8));
        actionReverse->setShortcut(QApplication::translate("MainWindow", "R", 0, QApplication::UnicodeUTF8));
        actionEnvelope->setText(QApplication::translate("MainWindow", "Envelope", 0, QApplication::UnicodeUTF8));
        actionHelp->setText(QApplication::translate("MainWindow", "Help", 0, QApplication::UnicodeUTF8));
        actionAbout->setText(QApplication::translate("MainWindow", "About", 0, QApplication::UnicodeUTF8));
        actionJoin->setText(QApplication::translate("MainWindow", "Join", 0, QApplication::UnicodeUTF8));
        actionJoin->setShortcut(QApplication::translate("MainWindow", "J", 0, QApplication::UnicodeUTF8));
        actionSelect_All->setText(QApplication::translate("MainWindow", "Select All", 0, QApplication::UnicodeUTF8));
        actionSelect_All->setShortcut(QApplication::translate("MainWindow", "Ctrl+A", 0, QApplication::UnicodeUTF8));
        actionSelect_None->setText(QApplication::translate("MainWindow", "Select None", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSelect_None->setToolTip(QApplication::translate("MainWindow", "Select None", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionApply_Gain->setText(QApplication::translate("MainWindow", "Apply Gain", 0, QApplication::UnicodeUTF8));
        actionApply_Gain->setShortcut(QApplication::translate("MainWindow", "G", 0, QApplication::UnicodeUTF8));
        actionNormalise->setText(QApplication::translate("MainWindow", "Normalise", 0, QApplication::UnicodeUTF8));
        actionNormalise->setShortcut(QApplication::translate("MainWindow", "N", 0, QApplication::UnicodeUTF8));
        actionZoom_In->setText(QApplication::translate("MainWindow", "Zoom In", 0, QApplication::UnicodeUTF8));
        actionZoom_In->setShortcut(QApplication::translate("MainWindow", "Ctrl++", 0, QApplication::UnicodeUTF8));
        actionZoom_Out->setText(QApplication::translate("MainWindow", "Zoom Out", 0, QApplication::UnicodeUTF8));
        actionZoom_Out->setShortcut(QApplication::translate("MainWindow", "Ctrl+-", 0, QApplication::UnicodeUTF8));
        actionZoom_Original->setText(QApplication::translate("MainWindow", "Zoom Original", 0, QApplication::UnicodeUTF8));
        actionZoom_Original->setShortcut(QApplication::translate("MainWindow", "Ctrl+0", 0, QApplication::UnicodeUTF8));
        actionMove->setText(QApplication::translate("MainWindow", "Move", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionMove->setToolTip(QApplication::translate("MainWindow", "Move", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionMove->setShortcut(QApplication::translate("MainWindow", "1", 0, QApplication::UnicodeUTF8));
        actionSelect->setText(QApplication::translate("MainWindow", "Select", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionSelect->setToolTip(QApplication::translate("MainWindow", "Select", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionSelect->setShortcut(QApplication::translate("MainWindow", "2", 0, QApplication::UnicodeUTF8));
        actionSplit->setText(QApplication::translate("MainWindow", "Split", 0, QApplication::UnicodeUTF8));
        actionSplit->setShortcut(QApplication::translate("MainWindow", "S", 0, QApplication::UnicodeUTF8));
        actionAudition->setText(QApplication::translate("MainWindow", "Audition", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        actionAudition->setToolTip(QApplication::translate("MainWindow", "Audition", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        actionAudition->setShortcut(QApplication::translate("MainWindow", "3", 0, QApplication::UnicodeUTF8));
        actionSave_As->setText(QApplication::translate("MainWindow", "Save As...", 0, QApplication::UnicodeUTF8));
        actionApply_Gain_Ramp->setText(QApplication::translate("MainWindow", "Apply Gain Ramp", 0, QApplication::UnicodeUTF8));
        actionApply_Gain_Ramp->setShortcut(QApplication::translate("MainWindow", "Shift+G", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Original BPM:", 0, QApplication::UnicodeUTF8));
        pushButton_CalcBPM->setText(QApplication::translate("MainWindow", "Calc", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "New BPM:", 0, QApplication::UnicodeUTF8));
        label_JackSync->setText(QApplication::translate("MainWindow", "JACK Sync", 0, QApplication::UnicodeUTF8));
        pushButton_Apply->setText(QApplication::translate("MainWindow", "Apply", 0, QApplication::UnicodeUTF8));
        checkBox_TimeStretch->setText(QApplication::translate("MainWindow", "Time Stretch", 0, QApplication::UnicodeUTF8));
        checkBox_PitchCorrection->setText(QApplication::translate("MainWindow", "Pitch Correction", 0, QApplication::UnicodeUTF8));
        pushButton_TimestretchOptions->setText(QApplication::translate("MainWindow", "Stretch Options...", 0, QApplication::UnicodeUTF8));
        pushButton_Slice->setText(QApplication::translate("MainWindow", "Slice", 0, QApplication::UnicodeUTF8));
        label_7->setText(QApplication::translate("MainWindow", "Detection Method:", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Window Size:", 0, QApplication::UnicodeUTF8));
        label_5->setText(QApplication::translate("MainWindow", "Hop Size:", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Threshold:", 0, QApplication::UnicodeUTF8));
        pushButton_FindOnsets->setText(QApplication::translate("MainWindow", "Find Onsets", 0, QApplication::UnicodeUTF8));
        pushButton_FindBeats->setText(QApplication::translate("MainWindow", "Find Beats", 0, QApplication::UnicodeUTF8));
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
