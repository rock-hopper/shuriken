/*
  This file is part of Shuriken Beat Slicer.

  Copyright (C) 2014 Andrew M Taylor <a.m.taylor303@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program. If not, see <https://www.gnu.org/licenses/>
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "exportdialog.h"
#include "ui_exportdialog.h"
#include "akaifilehandler.h"
#include <QFileDialog>
#include <QDebug>
#include "messageboxes.h"


//==================================================================================================
// Public:

ExportDialog::ExportDialog( QWidget* parent ) :
    QDialog( parent ),
    mUI( new Ui::ExportDialog ),
    mLastOpenedExportDir( QDir::homePath() )
{
    // Set up user interface
    mUI->setupUi( this );


    // Set up input validators
    mDirectoryValidator = new DirectoryValidator();
    mUI->lineEdit_OutputDir->setValidator( mDirectoryValidator );

    QObject::connect( mDirectoryValidator, SIGNAL( isValid(bool) ),
                      this, SLOT( displayDirValidityText(bool) ) );

    QObject::connect( mDirectoryValidator, SIGNAL( isValid(bool) ),
                      mUI->pushButton_Create, SLOT( setDisabled(bool) ) );

    QObject::connect( mUI->lineEdit_OutputDir, SIGNAL( textChanged(QString) ),
                      this, SLOT( enableOkButtonIfInputValid() ) );

    QObject::connect( mUI->lineEdit_FileName, SIGNAL( textChanged(QString) ),
                      this, SLOT( enableOkButtonIfInputValid() ) );


    // Populate combo boxes
    mUI->radioButton_AudioFiles->click();


    QStringList akaiModelTextList;
    QList<int> akaiModelDataList;

    akaiModelTextList << "MPC 1000/2500" << "MPC 500";
    akaiModelDataList << AkaiModelID::MPC1000_ID << AkaiModelID::MPC500_ID;

    for ( int i = 0; i < akaiModelTextList.size(); i++ )
    {
        mUI->comboBox_Model->addItem( akaiModelTextList[ i ], akaiModelDataList[ i ] );
    }

    mUI->label_Model->setVisible( false );
    mUI->comboBox_Model->setVisible( false );


    QStringList sampleRateTextList;
    QList<int> sampleRateDataList;

    sampleRateTextList << "Keep Same" << "8,000 Hz" << "11,025 Hz" << "16,000 Hz"<< "22,050 Hz" << "32,000 Hz" << "44,100 Hz" << "48,000 Hz" << "88,200 Hz" << "96,000 Hz" << "192,000 Hz";
    sampleRateDataList << SAMPLE_RATE_KEEP_SAME << 8000 << 11025 << 16000 << 22050 << 32000 << 44100 << 48000 << 88200 << 96000 << 192000;

    for ( int i = 0; i < sampleRateTextList.size(); i++ )
    {
        mUI->comboBox_SampleRate->addItem( sampleRateTextList[ i ], sampleRateDataList[ i ] );
    }


    QStringList midiFileTextList;
    midiFileTextList << "Don't Export" << "Export" << "Export Only";

    mUI->comboBox_MidiFile->addItems( midiFileTextList );
}



ExportDialog::~ExportDialog()
{
    delete mUI;
}



QString ExportDialog::getOutputDirPath() const
{
    return mUI->lineEdit_OutputDir->text();
}



int ExportDialog::getExportType() const
{
    int exportType;

    if ( mUI->comboBox_MidiFile->currentText() == "Export Only" )
    {
        exportType = EXPORT_MIDI_FILE;
    }
    else if ( mUI->radioButton_AudioFiles->isChecked() )
    {
        exportType = EXPORT_AUDIO_FILES;
    }
    else if ( mUI->radioButton_H2Drumkit->isChecked() )
    {
        exportType = EXPORT_H2DRUMKIT | EXPORT_AUDIO_FILES;
    }
    else if ( mUI->radioButton_SFZ->isChecked() )
    {
        exportType = EXPORT_SFZ | EXPORT_AUDIO_FILES;
    }
    else if ( mUI->radioButton_Akai->isChecked() )
    {
        exportType = EXPORT_AKAI_PGM | EXPORT_AUDIO_FILES;
    }

    if ( mUI->comboBox_MidiFile->currentText() == "Export" )
    {
        exportType |= EXPORT_MIDI_FILE;
    }

    return exportType;
}



int ExportDialog::getMidiFileType() const
{
    if ( mUI->radioButton_MidiType0->isChecked() )
    {
        return 0;
    }
    else // mUI->radioButton_MidiType1->isChecked()
    {
        return 1;
    }
}



QString ExportDialog::getFileName() const
{
    return mUI->lineEdit_FileName->text();
}



ExportDialog::NumberingStyle ExportDialog::getNumberingStyle() const
{
    if ( mUI->radioButton_Prefix->isChecked() )
    {
        return NUMBERING_PREFIX;
    }
    else
    {
        return NUMBERING_SUFFIX;
    }
}



bool ExportDialog::isOverwriteEnabled() const
{
    return mUI->checkBox_Overwrite->isChecked();
}



int ExportDialog::getSndFileFormat() const
{
    const int index = mUI->comboBox_Encoding->currentIndex();
    const int encoding = mUI->comboBox_Encoding->itemData( index ).toInt();

    QString formatName = mUI->comboBox_Format->currentText();
    int format = 0;

    if ( formatName.contains( "WAV" ) )
    {
        format = SF_FORMAT_WAV;
    }
    else if ( formatName == "AIFF" )
    {
        format = SF_FORMAT_AIFF;
    }
    else if ( formatName == "AU")
    {
        format = SF_FORMAT_AU;
    }
    else if ( formatName == "FLAC" )
    {
        format = SF_FORMAT_FLAC;
    }
    else if ( formatName == "Ogg" )
    {
        format = SF_FORMAT_OGG;
    }
    else
    {
        qDebug() << "Unknown format: " << formatName;
    }

    return format | encoding;
}



int ExportDialog::getSampleRate() const
{
    const int index = mUI->comboBox_SampleRate->currentIndex();
    const int sampleRate = mUI->comboBox_SampleRate->itemData( index ).toInt();

    return sampleRate;
}



int ExportDialog::getAkaiModelID() const
{
    const int index = mUI->comboBox_Model->currentIndex();
    const int modelID = mUI->comboBox_Model->itemData( index ).toInt();

    return modelID;
}



//==================================================================================================
// Protected:

void ExportDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        mUI->retranslateUi( this );
        break;
    default:
        break;
    }
}



void ExportDialog::showEvent( QShowEvent* event )
{
    // If the dialog is not being maximised, i.e. it has not previoulsy been minimised...
    if ( ! event->spontaneous() )
    {
        mUI->lineEdit_OutputDir->setText( mLastOpenedExportDir );
        mUI->lineEdit_FileName->clear();

        resize( 502, 282 );
    }

    QDialog::showEvent( event );
}



//==================================================================================================
// Private:

void ExportDialog::setPlatformFileNameValidator()
{
    QString pattern;

#ifdef LINUX
    pattern = "[^/\\s]+"; // Match any character except forward slash and white space
#endif

    if ( mUI->lineEdit_FileName->validator() != NULL )
    {
        const QValidator* validator = mUI->lineEdit_FileName->validator();
        const QRegExpValidator* regExpValidator = static_cast<const QRegExpValidator*>( validator );

        if ( regExpValidator->regExp().pattern() != pattern )
        {
            mUI->lineEdit_FileName->setValidator( new QRegExpValidator( QRegExp(pattern), this ) );
        }
    }
    else
    {
        mUI->lineEdit_FileName->setValidator( new QRegExpValidator( QRegExp(pattern), this ) );
    }
}



void ExportDialog::enableMidiFileTypeRadioButtons()
{
    if ( mUI->comboBox_MidiFile->currentText() == "Export" || mUI->comboBox_MidiFile->currentText() == "Export Only" )
    {
        foreach ( QAbstractButton* button, mUI->buttonGroup_Midi->buttons() )
        {
            button->setEnabled( true );
        }
    }
}



void ExportDialog::disableMidiFileTypeRadioButtons()
{
    foreach ( QAbstractButton* button, mUI->buttonGroup_Midi->buttons() )
    {
        button->setEnabled( false );
    }
}



//==================================================================================================
// Private Slots:

void ExportDialog::displayDirValidityText( const bool isValid )
{
    if ( isValid )
    {
        mUI->label_DirValidity->setVisible( false );
    }
    else
    {
        mUI->label_DirValidity->setText( tr("Dir doesn't exist") );
        mUI->label_DirValidity->setVisible( true );
    }
}



void ExportDialog::enableOkButtonIfInputValid()
{
    QPushButton* okButton = mUI->buttonBox->button( QDialogButtonBox::Ok );

    if ( mUI->lineEdit_OutputDir->hasAcceptableInput() && mUI->lineEdit_FileName->hasAcceptableInput() )
    {
        okButton->setEnabled( true );
    }
    else
    {
        okButton->setEnabled( false );
    }
}



void ExportDialog::on_pushButton_Choose_clicked()
{
    const QString dirPath = QFileDialog::getExistingDirectory( this, tr("Choose Output Directory"), mLastOpenedExportDir );

    // If user didn't click Cancel
    if ( ! dirPath.isEmpty() )
    {
        mUI->lineEdit_OutputDir->setText( dirPath );
        mLastOpenedExportDir = dirPath;
    }
}



void ExportDialog::on_pushButton_Create_clicked()
{
    const QString path = mUI->lineEdit_OutputDir->text();
    File newDir;

    if ( QFileInfo( path ).isAbsolute() )
    {
        newDir = File( path.toLocal8Bit().data() );
    }
    else
    {
        newDir = File::getCurrentWorkingDirectory().getChildFile( path.toLocal8Bit().data() );
    }

    const Result result = newDir.createDirectory();

    if ( result.wasOk() )
    {
        mUI->pushButton_Create->setEnabled( false );
        mUI->label_DirValidity->setVisible( false );
        enableOkButtonIfInputValid();
    }
    else
    {
        QString errorInfo( result.getErrorMessage().toUTF8() );
        MessageBoxes::showWarningDialog( tr( "Couldn't create directory!" ), errorInfo );
    }
}



void ExportDialog::on_comboBox_Format_currentIndexChanged( const QString text )
{
    QStringList encodingTextList;
    QList<int> encodingDataList;
    int index = 0;

    if ( text == "WAV")
    {
        encodingTextList << "Unsigned 8 bit PCM" << "Signed 16 bit PCM" << "Signed 24 bit PCM" << "Signed 32 bit PCM" << "32 bit float" << "64 bit double" << "u-law encoding" << "A-law encoding";
        encodingDataList << SF_FORMAT_PCM_U8 << SF_FORMAT_PCM_16 << SF_FORMAT_PCM_24 << SF_FORMAT_PCM_32 << SF_FORMAT_FLOAT << SF_FORMAT_DOUBLE << SF_FORMAT_ULAW << SF_FORMAT_ALAW;
        index = 1; // Signed 16 bit PCM
    }
    else if ( text == "AIFF" )
    {
        encodingTextList << "Unsigned 8 bit PCM" << "Signed 8 bit PCM" << "Signed 16 bit PCM" << "Signed 24 bit PCM" << "Signed 32 bit PCM" << "32 bit float" << "64 bit double" << "u-law encoding" << "A-law encoding";
        encodingDataList << SF_FORMAT_PCM_U8 << SF_FORMAT_PCM_S8 << SF_FORMAT_PCM_16 << SF_FORMAT_PCM_24 << SF_FORMAT_PCM_32 << SF_FORMAT_FLOAT << SF_FORMAT_DOUBLE << SF_FORMAT_ULAW << SF_FORMAT_ALAW;
        index = 2; // Signed 16 bit PCM
    }
    else if ( text == "AU")
    {
        encodingTextList << "Signed 8 bit PCM" << "Signed 16 bit PCM" << "Signed 24 bit PCM" << "Signed 32 bit PCM" << "32 bit float" << "64 bit double" << "u-law encoding" << "A-law encoding";
        encodingDataList << SF_FORMAT_PCM_S8 << SF_FORMAT_PCM_16 << SF_FORMAT_PCM_24 << SF_FORMAT_PCM_32 << SF_FORMAT_FLOAT << SF_FORMAT_DOUBLE << SF_FORMAT_ULAW << SF_FORMAT_ALAW;
        index = 1; // Signed 16 bit PCM
    }
    else if ( text == "FLAC" )
    {
        encodingTextList << "Signed 8 bit PCM" << "Signed 16 bit PCM" << "Signed 24 bit PCM";
        encodingDataList << SF_FORMAT_PCM_S8 << SF_FORMAT_PCM_16 << SF_FORMAT_PCM_24;
        index = 1; // Signed 16 bit PCM
    }
    else if ( text == "Ogg" )
    {
        encodingTextList << "Vorbis";
        encodingDataList << SF_FORMAT_VORBIS;
    }
    else if ( text == "PGM, WAV" )
    {
        encodingTextList << "16 bit PCM, 44,100 Hz";
        encodingDataList << ( SF_FORMAT_PCM_16 | SF_ENDIAN_LITTLE );
    }
    else
    {
        qDebug() << "Unknown format: " << text;
    }

    mUI->comboBox_Encoding->clear();

    for ( int i = 0; i < encodingTextList.size(); i++ )
    {
        mUI->comboBox_Encoding->addItem( encodingTextList[ i ], encodingDataList[ i ] );
    }

    mUI->comboBox_Encoding->setCurrentIndex( index );
}



void ExportDialog::on_radioButton_AudioFiles_clicked()
{
    mUI->label_FileName->setText( tr( "File Name(s):" ) );

    foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
    {
        button->setEnabled( true );
    }

    QStringList fileFormatTextList;
    fileFormatTextList << "WAV" << "AIFF" << "AU" << "FLAC" << "Ogg";
    mUI->comboBox_Format->clear();
    mUI->comboBox_Format->addItems( fileFormatTextList );

    mUI->label_Model->setVisible( false );
    mUI->comboBox_Model->setVisible( false );

    mUI->label_SampleRate->setVisible( true );
    mUI->comboBox_SampleRate->setVisible( true );

    mUI->lineEdit_FileName->clear();
    setPlatformFileNameValidator();

    const int index = mUI->comboBox_SampleRate->findData( SAMPLE_RATE_KEEP_SAME );
    mUI->comboBox_SampleRate->setCurrentIndex( index );

    enableMidiFileTypeRadioButtons();
}



void ExportDialog::on_radioButton_H2Drumkit_clicked()
{
    mUI->label_FileName->setText( tr( "Kit Name:" ) );

    foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
    {
        button->setEnabled( false );
    }

    mUI->radioButton_Suffix->setChecked( true );

    QStringList fileFormatTextList;
    fileFormatTextList << "FLAC" << "WAV" << "AIFF" << "AU";
    mUI->comboBox_Format->clear();
    mUI->comboBox_Format->addItems( fileFormatTextList );

    mUI->label_Model->setVisible( false );
    mUI->comboBox_Model->setVisible( false );

    mUI->label_SampleRate->setVisible( true );
    mUI->comboBox_SampleRate->setVisible( true );

    mUI->lineEdit_FileName->clear();
    setPlatformFileNameValidator();

    const int index = mUI->comboBox_SampleRate->findData( SAMPLE_RATE_KEEP_SAME );
    mUI->comboBox_SampleRate->setCurrentIndex( index );

    enableMidiFileTypeRadioButtons();
}



void ExportDialog::on_radioButton_SFZ_clicked()
{
    mUI->label_FileName->setText( tr( "SFZ Name:" ) );

    foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
    {
        button->setEnabled( false );
    }

    mUI->radioButton_Suffix->setChecked( true );

    QStringList fileFormatTextList;
    fileFormatTextList << "WAV" << "FLAC" << "Ogg";
    mUI->comboBox_Format->clear();
    mUI->comboBox_Format->addItems( fileFormatTextList );

    mUI->label_Model->setVisible( false );
    mUI->comboBox_Model->setVisible( false );

    mUI->label_SampleRate->setVisible( true );
    mUI->comboBox_SampleRate->setVisible( true );

    mUI->lineEdit_FileName->clear();
    setPlatformFileNameValidator();

    const int index = mUI->comboBox_SampleRate->findData( SAMPLE_RATE_KEEP_SAME );
    mUI->comboBox_SampleRate->setCurrentIndex( index );

    enableMidiFileTypeRadioButtons();
}



void ExportDialog::on_radioButton_Akai_clicked()
{
    mUI->label_FileName->setText( tr( "PGM Name:" ) );

    foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
    {
        button->setEnabled( false );
    }

    mUI->radioButton_Suffix->setChecked( true );

    QStringList fileFormatTextList;
    fileFormatTextList << "PGM, WAV";
    mUI->comboBox_Format->clear();
    mUI->comboBox_Format->addItems( fileFormatTextList );

    mUI->label_SampleRate->setVisible( false );
    mUI->comboBox_SampleRate->setVisible( false );

    mUI->label_Model->setVisible( true );
    mUI->comboBox_Model->setVisible( true );

    mUI->lineEdit_FileName->clear();

    const QRegExp regexp( AkaiFileHandler::getFileNameRegExpMPC1000() );
    mUI->lineEdit_FileName->setValidator( new QRegExpValidator( regexp, this ) );

    const int index = mUI->comboBox_SampleRate->findData( 44100 );
    mUI->comboBox_SampleRate->setCurrentIndex( index );

    disableMidiFileTypeRadioButtons();

    mUI->radioButton_MidiType1->setChecked( true );
}



void ExportDialog::on_comboBox_MidiFile_activated( const QString text )
{
    if ( text == "Export Only" )
    {
        foreach ( QAbstractButton* button, mUI->buttonGroup_Export->buttons() )
        {
            button->setEnabled( false );
        }

        foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
        {
            button->setEnabled( false );
        }

        mUI->comboBox_Encoding->setEnabled( false );
        mUI->comboBox_Format->setEnabled( false );
        mUI->comboBox_Model->setEnabled( false );
        mUI->comboBox_SampleRate->setEnabled( false );

        enableMidiFileTypeRadioButtons();
    }
    else
    {
        foreach ( QAbstractButton* button, mUI->buttonGroup_Export->buttons() )
        {
            button->setEnabled( true );
        }

        foreach ( QAbstractButton* button, mUI->buttonGroup_Numbering->buttons() )
        {
            button->setEnabled( true );
        }

        mUI->comboBox_Encoding->setEnabled( true );
        mUI->comboBox_Format->setEnabled( true );
        mUI->comboBox_Model->setEnabled( true );
        mUI->comboBox_SampleRate->setEnabled( true );

        if ( mUI->radioButton_Akai->isChecked() )
        {
            disableMidiFileTypeRadioButtons();
            mUI->radioButton_MidiType1->setChecked( true );
        }
        else if ( text == "Don't Export")
        {
            disableMidiFileTypeRadioButtons();
        }
        else
        {
            enableMidiFileTypeRadioButtons();
        }
    }
}
