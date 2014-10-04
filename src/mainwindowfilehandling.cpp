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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopWidget>
#include "commands.h"
#include "globals.h"
#include "zipper.h"
#include "messageboxes.h"
#include "textfilehandler.h"
#include "akaifilehandler.h"
#include "midifilehandler.h"
#include "confirmbpmdialog.h"
#include <QDebug>


//==================================================================================================
// Public:

void MainWindow::openProject( const QString filePath )
{
    if ( mOptionsDialog == NULL )
    {
        return;
    }

    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    if ( tempDirPath.isEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Temp directory is invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        return;
    }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    Zipper::decompress( filePath, tempDirPath );

    const QFileInfo zipFileInfo( filePath );
    const QString projectName = zipFileInfo.baseName();

    const QDir tempDir( tempDirPath );
    QDir projTempDir = tempDir.absoluteFilePath( projectName );

    const QString xmlFilePath = projTempDir.absoluteFilePath( "shuriken.xml" );
    TextFileHandler::ProjectSettings settings;

    const bool isSuccessful = TextFileHandler::readProjectXmlFile( xmlFilePath, settings );

    if ( isSuccessful )
    {
        closeProject();

        bool isOkToContinue = true;

        // Try to load the audio files
        foreach ( QString fileName, settings.audioFileNames )
        {
            const QString audioFilePath = projTempDir.absoluteFilePath( fileName );
            SharedSampleBuffer sampleBuffer = mFileHandler.getSampleData( audioFilePath );

            if ( ! sampleBuffer.isNull() )
            {
                mSampleBufferList << sampleBuffer;
            }
            else
            {
                isOkToContinue = false;
                break;
            }
        }

        // Try to read the audio file header info
        {
            const QString audioFilePath = projTempDir.absoluteFilePath( settings.audioFileNames.first() );
            mSampleHeader = mFileHandler.getSampleHeader( audioFilePath );
        }

        if ( mSampleHeader.isNull() )
        {
            isOkToContinue = false;
        }

        // If the audio files were read successfully
        if ( isOkToContinue )
        {
            // Deal with sample ranges - provides backward compatibility with older save file format
            if ( ! settings.sampleRangeList.isEmpty() )
            {
                const int numChans = mSampleHeader->numChans;

                QList<SharedSampleBuffer> tempSampleBuffers;

                foreach ( SharedSampleRange range, settings.sampleRangeList )
                {
                    SharedSampleBuffer sampleBuffer( new SampleBuffer( numChans, range->numFrames ) );

                    for ( int chanNum = 0; chanNum < numChans; chanNum++ )
                    {
                        sampleBuffer->copyFrom( chanNum, 0, *mSampleBufferList.first().data(), chanNum, range->startFrame, range->numFrames );
                    }

                    tempSampleBuffers << sampleBuffer;
                }

                mSampleBufferList = tempSampleBuffers;
            }

            // Only one sample buffer - waveform has not been sliced
            if ( mSampleBufferList.size() == 1 )
            {
                const SharedWaveformItem item = mUI->waveGraphicsView->createWaveform( mSampleBufferList.first(),
                                                                                       mSampleHeader );
                connectWaveformToMainWindow( item );

                setUpSampler();

                enableUI();

                if ( ! settings.slicePointFrameNums.isEmpty() )
                {
                    QUndoCommand* parentCommand = new QUndoCommand();

                    foreach ( int frameNum, settings.slicePointFrameNums )
                    {
                        new AddSlicePointItemCommand( frameNum, mUI->waveGraphicsView, mUI->pushButton_Slice, parentCommand );
                    }
                    mUndoStack.push( parentCommand );
                }
            }
            else // Multiple sample buffers - waveform has been sliced
            {
                const QList<SharedWaveformItem> waveformItemList = mUI->waveGraphicsView->createWaveforms( mSampleBufferList, mSampleHeader );

                foreach ( SharedWaveformItem item, waveformItemList )
                {
                    connectWaveformToMainWindow( item );
                }

                setUpSampler();

                enableUI();
                mUI->actionAdd_Slice_Point->setEnabled( false );
                mUI->pushButton_FindBeats->setEnabled( false );
                mUI->pushButton_FindOnsets->setEnabled( false );
            }

            mAppliedBPM = settings.appliedBpm;

            mOptionsDialog->setStretcherOptions( settings.options );

            if ( settings.isJackSyncChecked )
            {
                mOptionsDialog->enableJackSync();
            }

            mUI->checkBox_TimeStretch->setChecked( settings.isTimeStretchChecked );
            mUI->checkBox_PitchCorrection->setChecked( settings.isPitchCorrectionChecked );

            if ( settings.originalBpm > 0.0 )
            {
                mUI->doubleSpinBox_OriginalBPM->setValue( settings.originalBpm );
            }
            if ( settings.newBpm > 0.0 )
            {
                mUI->doubleSpinBox_NewBPM->setValue( settings.newBpm );
            }

            {
                const int i = mUI->comboBox_TimeSigNumerator->findText( QString::number( settings.timeSigNumerator ) );
                mUI->comboBox_TimeSigNumerator->setCurrentIndex( i );
            }
            {
                const int i = mUI->comboBox_TimeSigDenominator->findText( QString::number( settings.timeSigDenominator ) );
                mUI->comboBox_TimeSigDenominator->setCurrentIndex( i );
            }
            mUI->spinBox_Length->setValue( settings.length );
            mUI->comboBox_Units->setCurrentIndex( settings.units );

            updateSnapLoopMarkersComboBox();

            // Clean up temp dir
            File( projTempDir.absolutePath().toLocal8Bit().data() ).deleteRecursively();

            mCurrentProjectFilePath = filePath;

            mUI->statusBar->showMessage( tr("Project: ") + projectName );

            mIsProjectOpen = true;

            QApplication::restoreOverrideCursor();
        }
        else // Error loading audio files
        {
            mSampleBufferList.clear();
            mSampleHeader.clear();

            QApplication::restoreOverrideCursor();

            MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
        }
    }
}



//==================================================================================================
// Private:

void MainWindow::exportAs( const QString tempDirPath,
                           const QString outputDirPath,
                           const QString samplesDirPath,
                           const QString fileName,
                           const int exportType,
                           const int sndFileFormat,
                           const int outputSampleRate,
                           const int numSamplesToExport )
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const QDir outputDir( outputDirPath );

    const bool isExportTypeAudioFiles = exportType & ExportDialog::EXPORT_AUDIO_FILES;
    const bool isExportTypeH2Drumkit  = exportType & ExportDialog::EXPORT_H2DRUMKIT;
    const bool isExportTypeSFZ        = exportType & ExportDialog::EXPORT_SFZ;
    const bool isExportTypeAkaiPgm    = exportType & ExportDialog::EXPORT_AKAI_PGM;
    const bool isExportTypeMidiFile   = exportType & ExportDialog::EXPORT_MIDI_FILE;

    if ( isExportTypeAudioFiles )
    {
        outputDir.mkdir( fileName );
    }

    QStringList audioFileNames;
    bool isSuccessful = true;

    // Export audio files
    if ( isExportTypeAudioFiles )
    {
        for ( int i = 0; i < numSamplesToExport; i++ )
        {
            QString audioFileName = fileName;

            if ( isExportTypeAkaiPgm && audioFileName.size() > 14 )
            {
                audioFileName.resize( 14 );
            }

            if ( mExportDialog->getNumberingStyle() == ExportDialog::NUMBERING_PREFIX )
            {
                audioFileName.prepend( QString::number( i + 1 ).rightJustified( 2, '0' ) );
            }
            else // SUFFIX
            {
                audioFileName.append( QString::number( i + 1 ).rightJustified( 2, '0' ) );
            }

            const QString path = mFileHandler.saveAudioFile( samplesDirPath,
                                                             audioFileName,
                                                             mSampleBufferList.at( i ),
                                                             mSampleHeader->sampleRate,
                                                             outputSampleRate,
                                                             sndFileFormat );

            if ( ! path.isEmpty() )
            {
                if ( isExportTypeAkaiPgm )
                {
                    audioFileNames << audioFileName;    // File base name, no extension
                }
                else
                {
                    audioFileNames << QFileInfo( path ).fileName(); // File name including extension
                }
            }
            else
            {
                isSuccessful = false;
                break;
            }
        }
    }

    // Export Hydrogen drumkit
    if ( isSuccessful && isExportTypeH2Drumkit )
    {
        TextFileHandler::createH2DrumkitXmlFile( samplesDirPath, fileName, audioFileNames );
#ifdef LINUX
        const QString cdCommand  = "cd '" + outputDirPath + "'";
        const QString tarCommand = "tar --create --gzip --file '" + fileName + ".h2drumkit' '" + fileName + "'";

        const QString command = cdCommand + " && " + tarCommand;

        system( command.toLocal8Bit().data() );
#endif
        File( samplesDirPath.toLocal8Bit().data() ).deleteRecursively();
    }
    // Export SFZ
    else if ( isSuccessful && isExportTypeSFZ )
    {
        const QString sfzFilePath = outputDir.absoluteFilePath( fileName + ".sfz" );
        const QString samplesDirName = QFileInfo( samplesDirPath ).fileName();

        TextFileHandler::createSFZFile( sfzFilePath, samplesDirName, audioFileNames );
    }
    // Export Akai PGM
    else if ( isSuccessful && isExportTypeAkaiPgm )
    {
        const int modelID = mExportDialog->getAkaiModelID();

        switch ( modelID )
        {
        case AkaiModelID::MPC1000_ID:
            AkaiFileHandler::writePgmFileMPC1000( audioFileNames, fileName, samplesDirPath, tempDirPath );
            break;
        case AkaiModelID::MPC500_ID:
            AkaiFileHandler::writePgmFileMPC500( audioFileNames, fileName, samplesDirPath, tempDirPath );
            break;
        default:
            break;
        }
    }

    // Export MIDI file
    if ( isSuccessful && isExportTypeMidiFile )
    {
        const qreal bpm = mUI->doubleSpinBox_OriginalBPM->value();
        const ConfirmBpmDialog::TimeSigNumerator numerator = (ConfirmBpmDialog::TimeSigNumerator) mUI->comboBox_TimeSigNumerator->currentIndex();
        const ConfirmBpmDialog::TimeSigDenominator denominator = (ConfirmBpmDialog::TimeSigDenominator) mUI->comboBox_TimeSigDenominator->currentIndex();

        QApplication::restoreOverrideCursor();

        ConfirmBpmDialog dialog( bpm, numerator, denominator );
        const int result = dialog.exec();

        if ( result == QDialog::Accepted )
        {
            QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

            const qreal bpm = dialog.getBpm();
            const int timeSigNumerator = dialog.getTimeSigNumerator();
            const int timeSigDenominator = dialog.getTimeSigDenominator();
            const MidiFileHandler::MidiFileType type = (MidiFileHandler::MidiFileType) mExportDialog->getMidiFileType();

            if ( isExportTypeAkaiPgm )
            {
                MidiFileHandler::SaveMidiFile( fileName, samplesDirPath, mSampleBufferList, numSamplesToExport, mSampleHeader->sampleRate, bpm, timeSigNumerator, timeSigDenominator, type );
            }
            else
            {
                MidiFileHandler::SaveMidiFile( fileName, outputDirPath, mSampleBufferList, numSamplesToExport, mSampleHeader->sampleRate, bpm, timeSigNumerator, timeSigDenominator, type );
            }
        }
    }

    QApplication::restoreOverrideCursor();

    if ( ! isSuccessful )
    {
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



void MainWindow::saveProject( const QString filePath )
{
    if ( mOptionsDialog == NULL )
    {
        return;
    }

    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    if ( tempDirPath.isEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Temp directory is invalid!"),
                                         tr("This operation needs to save temporary files, please change \"Temp Dir\" in options") );
        return;
    }

    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

    const QString zipFileName = QFileInfo( filePath ).fileName();
    const QString projectName = QFileInfo( filePath ).baseName();

    QDir tempDir( tempDirPath );

    // Should not happen, but just as a precaution...
    if ( tempDir.exists( projectName ) )
    {
        tempDir.rename( projectName, projectName + ".bak" );
    }

    tempDir.mkdir( projectName );

    QDir projTempDir( tempDir.absoluteFilePath( projectName ) );

    const QString zipFilePath = tempDir.absoluteFilePath( zipFileName );

    const QString xmlFilePath = projTempDir.absoluteFilePath( "shuriken.xml" );

    bool isOkToContinue = true;
    QStringList audioFileNames;

    for ( int i = 0; i < mSampleBufferList.size(); i++ )
    {
        const QString audioFilePath = mFileHandler.saveAudioFile( projTempDir.absolutePath(),
                                                                  "audio" + QString::number( i ),
                                                                  mSampleBufferList.at( i ),
                                                                  mSampleHeader->sampleRate,
                                                                  mSampleHeader->sampleRate,
                                                                  AudioFileHandler::SAVE_FORMAT );

        if ( ! audioFilePath.isEmpty() )
        {
            audioFileNames << QFileInfo( audioFilePath ).fileName();
        }
        else
        {
            isOkToContinue = false;
            break;
        }
    }

    if ( isOkToContinue )
    {
        TextFileHandler::ProjectSettings settings;

        settings.projectName = projectName;

        if ( mOptionsDialog->isRealtimeModeEnabled() )
        {
            settings.originalBpm = mUI->doubleSpinBox_OriginalBPM->value();
            settings.newBpm = mUI->doubleSpinBox_NewBPM->value();
            settings.appliedBpm = mAppliedBPM;
        }
        else
        {
            settings.originalBpm = mAppliedBPM;
            settings.newBpm = mAppliedBPM;
            settings.appliedBpm = mAppliedBPM;
        }

        settings.isTimeStretchChecked = mUI->checkBox_TimeStretch->isChecked();
        settings.isPitchCorrectionChecked = mUI->checkBox_PitchCorrection->isChecked();
        settings.options = mOptionsDialog->getStretcherOptions();
        settings.isJackSyncChecked = mOptionsDialog->isJackSyncEnabled();

        settings.timeSigNumerator = mUI->comboBox_TimeSigNumerator->currentText().toInt();
        settings.timeSigDenominator = mUI->comboBox_TimeSigDenominator->currentText().toInt();
        settings.length = mUI->spinBox_Length->value();
        settings.units = mUI->comboBox_Units->currentIndex();

        settings.audioFileNames = audioFileNames;

        settings.slicePointFrameNums = mUI->waveGraphicsView->getSlicePointFrameNums();

        TextFileHandler::createProjectXmlFile( xmlFilePath, settings );

        Zipper::compress( projTempDir.absolutePath(), zipFilePath );

        QFile::remove( filePath );
        QFile::copy( zipFilePath, filePath );
        QFile::remove( zipFilePath );
        File( projTempDir.absolutePath().toLocal8Bit().data() ).deleteRecursively();

        mUndoStack.setClean();

        mCurrentProjectFilePath = filePath;

        QApplication::restoreOverrideCursor();
    }
    else // An error occurred while writing the audio files
    {
        QApplication::restoreOverrideCursor();
        MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
    }
}



void MainWindow::saveProjectDialog()
{
    // Save file dialog
    const QString filter = "Shuriken Project (*.shuriken)";
    QString selectedFilter;
    QString filePath = QFileDialog::getSaveFileName( this, tr("Save Project"), mLastOpenedProjDir, filter, &selectedFilter );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QFileInfo projectFile( filePath );
        bool isFileNameChanged = false;

        if ( projectFile.completeSuffix() != "shuriken" )
        {
            QDir dir = projectFile.absoluteDir();
            QString newFileName = projectFile.baseName().append( ".shuriken" );

            filePath = dir.absoluteFilePath( newFileName );
            projectFile.setFile( filePath );

            isFileNameChanged = true;
        }

        bool isOkToSave = true;

        if ( projectFile.exists() )
        {
            int buttonClicked = QMessageBox::Ok;

            if ( isFileNameChanged )
            {
                buttonClicked = MessageBoxes::showQuestionDialog( tr( "Overwrite existing file?" ),
                                                                  tr( "The file " ) + filePath + tr( " already exists.\n\nDo you want to overwrite this file?" ),
                                                                  QMessageBox::Ok | QMessageBox::Cancel );
            }

            if ( buttonClicked == QMessageBox::Ok )
            {
                isOkToSave = QFile::remove( filePath );

                if ( ! isOkToSave )
                {
                    MessageBoxes::showWarningDialog( tr( "Could not save project!" ),
                                                     tr( "The file \"" ) + filePath + tr( "\" could not be overwritten" ) );
                }
            }
            else // QMessageBox::Cancel
            {
                isOkToSave = false;
            }
        }

        if ( isOkToSave )
        {
            QFileInfo parentDir( projectFile.absolutePath() );

            if ( parentDir.isWritable() )
            {
                saveProject( filePath );
                mLastOpenedProjDir = projectFile.absolutePath();
            }
            else
            {
                MessageBoxes::showWarningDialog( tr( "Could not save project!" ),
                                                 tr( "Permission to write file denied" ) );
            }
        }
    }
}



void MainWindow::openProjectDialog()
{
    // Open file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr( "Open Project" ), mLastOpenedProjDir,
                                                           tr( "Shuriken Project (*.shuriken)" ) );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        openProject( filePath );
        mLastOpenedProjDir = QFileInfo( filePath ).absolutePath();
    }
}



void MainWindow::importAudioFileDialog()
{
    // Open file dialog
    const QString filePath = QFileDialog::getOpenFileName( this, tr("Import Audio File"), mLastOpenedImportDir,
                                                           tr("All Files (*.*)") );

    // If user didn't click "Cancel"
    if ( ! filePath.isEmpty() )
    {
        QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

        const QFileInfo fileInfo( filePath );
        const QString fileName = fileInfo.fileName();

        mLastOpenedImportDir = fileInfo.absolutePath();

        SharedSampleBuffer sampleBuffer = mFileHandler.getSampleData( filePath );
        SharedSampleHeader sampleHeader = mFileHandler.getSampleHeader( filePath );

        if ( sampleBuffer.isNull() || sampleHeader.isNull() )
        {
            QApplication::restoreOverrideCursor();
            MessageBoxes::showWarningDialog( mFileHandler.getLastErrorTitle(), mFileHandler.getLastErrorInfo() );
        }
        else
        {
            closeProject();

            mSampleBufferList << sampleBuffer;
            mSampleHeader = sampleHeader;

            const SharedWaveformItem item = mUI->waveGraphicsView->createWaveform( sampleBuffer, sampleHeader );
            connectWaveformToMainWindow( item );

            setUpSampler();

            enableUI();

            updateSnapLoopMarkersComboBox();

            // Set status bar message
            {
                const QString channels = sampleHeader->numChans == 1 ? "Mono" : "Stereo";

                const QString bits = QString::number( sampleHeader->bitsPerSample ) + " bits";

                const QString rate = QString::number( sampleHeader->sampleRate ) + " Hz";

                QString message = fileName + ", " + channels + ", " + bits + ", " + rate + ", " + sampleHeader->format;
                mUI->statusBar->showMessage( message );
            }

            mIsProjectOpen = true;

            QApplication::restoreOverrideCursor();
        }
    }
}



void MainWindow::exportAsDialog()
{
    if ( mExportDialog == NULL || mOptionsDialog == NULL )
    {
        return;
    }

    QPoint pos = mExportDialog->pos();

    if ( pos.x() < 0 )
        pos.setX( 0 );
    if ( pos.y() < 0 )
        pos.setY( 0 );

    mExportDialog->move( pos );

    const int result = mExportDialog->exec();

    if ( result != QDialog::Accepted )
    {
        return;
    }

    const QString tempDirPath = mOptionsDialog->getTempDirPath();

    const QString outputDirPath = mExportDialog->getOutputDirPath();
    const QDir outputDir( outputDirPath );

    const QString fileName = mExportDialog->getFileName();

    const QString samplesDirPath = outputDir.absoluteFilePath( fileName );

    const bool isOverwriteEnabled = mExportDialog->isOverwriteEnabled();

    const int exportType = mExportDialog->getExportType();

    const bool isExportTypeAudioFiles = exportType & ExportDialog::EXPORT_AUDIO_FILES;
    const bool isExportTypeH2Drumkit  = exportType & ExportDialog::EXPORT_H2DRUMKIT;
    const bool isExportTypeSFZ        = exportType & ExportDialog::EXPORT_SFZ;
    const bool isExportTypeAkaiPgm    = exportType & ExportDialog::EXPORT_AKAI_PGM;
    const bool isExportTypeMidiFile   = exportType & ExportDialog::EXPORT_MIDI_FILE;

    const int sndFileFormat = mExportDialog->getSndFileFormat();

    int outputSampleRate = mExportDialog->getSampleRate();

    if ( outputSampleRate == ExportDialog::SAMPLE_RATE_KEEP_SAME )
    {
        outputSampleRate = mSampleHeader->sampleRate;
    }

    int numSamplesToExport = mSampleBufferList.size();

    QStringList fullFileNamesList;

    if ( isExportTypeH2Drumkit )
    {
        fullFileNamesList << fileName + ".h2drumkit";
    }
    else if ( isExportTypeSFZ )
    {
        fullFileNamesList << fileName + ".sfz";
    }
    else if ( isExportTypeAkaiPgm )
    {
        fullFileNamesList << fileName + AkaiFileHandler::getFileExtension();
    }
    else if ( isExportTypeMidiFile )
    {
        fullFileNamesList << fileName + MidiFileHandler::getFileExtension();
    }


    // Carry out some basic checks before exporting


    if ( ! QFileInfo( outputDirPath ).isWritable() )
    {
        MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                         tr( "Permission to write file(s) denied" ) );
        return;
    }

    if ( isExportTypeAudioFiles && QFileInfo( samplesDirPath ).exists() )
    {
        if ( isOverwriteEnabled )
        {
            const int buttonClicked = MessageBoxes::showQuestionDialog( tr( "Delete contents of directory?" ),
                                                                        tr( "Do you want to delete the contents of \"" ) + samplesDirPath + tr( "\" before continuing?" )
                                                                        + tr( "\n\nThis operation cannot be undone"),
                                                                        QMessageBox::Ok | QMessageBox::Cancel );
            if ( buttonClicked == QMessageBox::Ok )
            {
                File( samplesDirPath.toLocal8Bit().data() ).deleteRecursively();
            }
            else
            {
                return;
            }
        }
        else
        {
            MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                             tr( "Tried to create \"" ) + samplesDirPath + tr( "\" but this directory already exists" ) );
            return;
        }
    }

    foreach ( QString fullFileName, fullFileNamesList )
    {
        if ( outputDir.exists( fullFileName ) )
        {
            if ( isOverwriteEnabled )
            {
                QFile::remove( outputDir.absoluteFilePath( fullFileName ) );
            }
            else
            {
                MessageBoxes::showWarningDialog( tr( "Cannot export project!" ),
                                                 "\"" + fullFileName + tr( "\" already exists" ) );
                return;
            }
        }
    }

    if ( isExportTypeAkaiPgm )
    {
        const int modelID = mExportDialog->getAkaiModelID();

        const int numPads = AkaiFileHandler::getNumPads( modelID );

        if ( numSamplesToExport > numPads )
        {
            numSamplesToExport = numPads;

            MessageBoxes::showWarningDialog( tr( "Cannot export all samples!" ),
                                             tr( "Too many samples for this Akai model, only the first " ) +
                                             QString::number( numPads ) + tr( " will be exported" ) );
        }
    }


    // Checks complete, now do export
    exportAs( tempDirPath, outputDirPath, samplesDirPath, fileName, exportType, sndFileFormat, outputSampleRate, numSamplesToExport );
}
