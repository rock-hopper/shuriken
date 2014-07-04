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

#include "audiosetupdialog.h"
#include "ui_audiosetupdialog.h"
#include "globals.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QDebug>


//==================================================================================================
// Public:

AudioSetupDialog::AudioSetupDialog( AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    mUI( new Ui::AudioSetupDialog ),
    mDeviceManager( deviceManager ),
    mStretcherOptions( RubberBandStretcher::DefaultOptions )
{
    // Setup user interface
    mUI->setupUi( this );


    // Get available audio backends (ALSA, JACK, etc.)
    const OwnedArray<AudioIODeviceType>& audioBackendTypes = mDeviceManager.getAvailableDeviceTypes();


    // Populate audio backend combo box
    for ( int i = 0; i < audioBackendTypes.size(); ++i )
    {
        mUI->comboBox_AudioBackend->addItem( audioBackendTypes.getUnchecked( i )->getTypeName().toRawUTF8() );
    }


    // Select current audio backend
    const QString backendName = mDeviceManager.getCurrentAudioDeviceType().toRawUTF8();
    const int index = mUI->comboBox_AudioBackend->findText( backendName );
    mUI->comboBox_AudioBackend->setCurrentIndex( index ); // This will also update all the other widgets


    // Paths
    ScopedPointer<XmlElement> docElement;
    docElement = XmlDocument::parse( File( PATHS_CONFIG_FILE_PATH ) );

    QString tempDirPath = QDir::tempPath();

    if ( docElement != NULL )
    {
        if ( docElement->hasTagName( "paths" ) )
        {
            forEachXmlChildElement( *docElement, elem )
            {
                if ( elem->hasTagName( "temp_dir" ) )
                {
                    tempDirPath = elem->getStringAttribute( "path" ).toRawUTF8();
                }
            }
        }
    }

    mDirectoryValidator = new DirectoryValidator();
    mUI->lineEdit_TempDir->setValidator( mDirectoryValidator );

    QObject::connect( mDirectoryValidator, SIGNAL( isValid(bool) ),
                      this, SLOT( displayDirValidityText(bool) ) );

    mUI->lineEdit_TempDir->setText( tempDirPath );
}



AudioSetupDialog::~AudioSetupDialog()
{
    delete mUI;
}



void AudioSetupDialog::setCurrentTab( const Tab tab )
{
    mUI->tabWidget->setCurrentIndex( tab );
}



bool AudioSetupDialog::isRealtimeModeEnabled() const
{
    return mUI->radioButton_RealTime->isChecked();
}



void AudioSetupDialog::enableRealtimeMode( const bool isEnabled )
{
    mUI->radioButton_RealTime->setChecked( isEnabled );
    emit realtimeModeToggled( isEnabled );
}



bool AudioSetupDialog::isJackSyncEnabled() const
{
    return mUI->checkBox_JackSync->isChecked();
}



QString AudioSetupDialog::getTempDirPath() const
{
    QDir parentDir( mUI->lineEdit_TempDir->text() );

    // Set a unique name for the temp dir, there may be multiple instances of Shuriken running

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString userName = env.value( "USER", "user" );

    QString pid;
    pid.setNum( QCoreApplication::applicationPid() );

    QString tempDirName( "shuriken-" );
    tempDirName.append( userName );
    tempDirName.append( "-" );
    tempDirName.append( pid );

    bool isTempDirValid = false;

    if ( parentDir.exists( tempDirName ) )
    {
        isTempDirValid = true;
    }
    else
    {
        isTempDirValid = parentDir.mkdir( tempDirName );
    }

    if ( isTempDirValid )
    {
        return parentDir.absoluteFilePath( tempDirName );
    }
    else
    {
        return QString();
    }
}



//==================================================================================================
// Protected:

void AudioSetupDialog::changeEvent( QEvent* event )
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



void AudioSetupDialog::showEvent( QShowEvent* event )
{
    // If the dialog is not being maximised, i.e. it has not previoulsy been minimised...
    if ( ! event->spontaneous() )
    {
        // Get current audio settings and store them so that any
        // changes can be reverted if the user later clicks "Cancel"
        mDeviceManager.getAudioDeviceSetup( mOriginalConfig );


        if ( mUI->checkBox_MidiInputTestTone->isChecked() )
        {
            setUpMidiInputTestSynth();
        }
    }

    QDialog::showEvent( event );
}



void AudioSetupDialog::closeEvent( QCloseEvent* event )
{
    tearDownMidiInputTestSynth();
    event->accept();
}



//==================================================================================================
// Private:

bool AudioSetupDialog::isJackAudioEnabled() const
{
    return ( mDeviceManager.getCurrentAudioDeviceType() == "JACK" );
}



void AudioSetupDialog::updateAudioDeviceComboBox()
{
    AudioIODeviceType* const audioBackendType = mDeviceManager.getCurrentDeviceTypeObject();

    if ( audioBackendType != NULL )
    {
        // Get available audio devices
        audioBackendType->scanForDevices();
        const StringArray audioDeviceNames( audioBackendType->getDeviceNames( false ) );

        // Populate combo box
        mUI->comboBox_AudioDevice->clear();

        for ( int i = 0; i < audioDeviceNames.size(); ++i )
        {
            mUI->comboBox_AudioDevice->addItem( audioDeviceNames[i].toRawUTF8() );
        }

        mUI->comboBox_AudioDevice->addItem( getNoDeviceString() );

        // Select current audio device
        AudioIODevice* const currentAudioDevice = mDeviceManager.getCurrentAudioDevice();

        if ( currentAudioDevice != NULL )
        {
            const int index = audioBackendType->getIndexOfDevice( currentAudioDevice, false );
            mUI->comboBox_AudioDevice->setCurrentIndex( index );
        }
    }
}



void AudioSetupDialog::updateOutputChannelComboBox()
{
    mUI->comboBox_OutputChannels->clear();

    if ( isJackAudioEnabled() )
    {
        mUI->comboBox_OutputChannels->setVisible( false );
        mUI->label_OutputChannels->setVisible( false );
    }
    else
    {
        mUI->comboBox_OutputChannels->setVisible( true );
        mUI->label_OutputChannels->setVisible( true );

        AudioIODevice* const currentAudioDevice = mDeviceManager.getCurrentAudioDevice();

        if ( currentAudioDevice != NULL )
        {
            mUI->comboBox_OutputChannels->setEnabled( true );

            StringArray channelNames;
            channelNames = currentAudioDevice->getOutputChannelNames();
            StringArray channelPairNames;

            for ( int i = 0; i < channelNames.size(); i += 2 )
            {
                const String& name = channelNames[i];

                if ( i + 1 >= channelNames.size() )
                    channelPairNames.add( name.trim() );
                else
                    channelPairNames.add( getNameForChannelPair( name, channelNames[i + 1] ) );
            }

            channelNames = channelPairNames;

            int startBit;
            const int numBits = 2;
            const bool shouldBeSet = true;

            for ( int i = 0; i < channelNames.size(); ++i )
            {
                // Each set bit represents an audio channel
                startBit = i * 2;
                BigInteger channels;
                channels.setRange( startBit, numBits, shouldBeSet );

                mUI->comboBox_OutputChannels->addItem( channelNames[i].toRawUTF8(), channels.toInt64() );
            }
        }
        else
        {
            mUI->comboBox_OutputChannels->addItem( getNoDeviceString() );
            mUI->comboBox_OutputChannels->setEnabled( false );
        }
    }
}



void AudioSetupDialog::updateSampleRateComboBox()
{
    mUI->comboBox_SampleRate->clear();

    AudioIODevice* const currentAudioDevice = mDeviceManager.getCurrentAudioDevice();

    if ( currentAudioDevice != NULL )
    {
        mUI->comboBox_SampleRate->setEnabled( true );

        const Array<double> sampleRates( currentAudioDevice->getAvailableSampleRates() );

        for ( int i = 0; i < sampleRates.size(); ++i )
        {
            const int sampleRate = roundToInt( sampleRates[i] );
            mUI->comboBox_SampleRate->addItem( QString::number( sampleRate ) + " Hz", sampleRate );
        }

        // Select current sample rate
        const int currentSampleRate = roundToInt( currentAudioDevice->getCurrentSampleRate() );
        const int index = mUI->comboBox_SampleRate->findData( currentSampleRate );
        mUI->comboBox_SampleRate->setCurrentIndex( index );
    }
    else
    {
        mUI->comboBox_SampleRate->addItem( getNoDeviceString() );
        mUI->comboBox_SampleRate->setEnabled( false );
    }

    if ( isJackAudioEnabled() )
    {
        mUI->comboBox_SampleRate->setEnabled( false );
    }
}



void AudioSetupDialog::updateBufferSizeComboBox()
{
    mUI->comboBox_BufferSize->clear();

    AudioIODevice* const currentAudioDevice = mDeviceManager.getCurrentAudioDevice();

    if ( currentAudioDevice != NULL )
    {
        mUI->comboBox_BufferSize->setEnabled( true );

        const Array<int> bufferSizes( currentAudioDevice->getAvailableBufferSizes() );
        double currentRate = currentAudioDevice->getCurrentSampleRate();

        if ( currentRate == 0 )
            currentRate = 48000.0;

        const char format = 'f'; // See QString "Argument Formats" for details
        const int precision = 1;

        for ( int i = 0; i < bufferSizes.size(); ++i )
        {
            const int bufferSize = bufferSizes[i];
            mUI->comboBox_BufferSize->addItem( QString::number( bufferSize )
                                              + " samples ("
                                              + QString::number( bufferSize * 1000.0 / currentRate, format, precision )
                                              + " ms)",
                                              bufferSize );
        }

        // Select current buffer size
        const int index = mUI->comboBox_BufferSize->findData( currentAudioDevice->getCurrentBufferSizeSamples() );
        mUI->comboBox_BufferSize->setCurrentIndex( index );
    }
    else
    {
        mUI->comboBox_BufferSize->addItem( getNoDeviceString() );
        mUI->comboBox_BufferSize->setEnabled( false );
    }

    if ( isJackAudioEnabled() )
    {
        mUI->comboBox_BufferSize->setEnabled( false );
    }
}



void AudioSetupDialog::updateMidiInputListWidget()
{
    mUI->listWidget_MidiInput->clear();

    const int numMidiDevices = MidiInput::getDevices().size();

    if ( numMidiDevices > 0 )
    {
        mUI->listWidget_MidiInput->setEnabled( true );

        for ( int i = 0; i < numMidiDevices; ++i )
        {
            const String midiInputName( MidiInput::getDevices()[ i ] );

            if ( midiInputName != "jackmidi" )
            {
                QListWidgetItem* const listItem = new QListWidgetItem();

                listItem->setText( midiInputName.toRawUTF8() );
                listItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );

                if ( mDeviceManager.isMidiInputEnabled( midiInputName ) )
                {
                    listItem->setCheckState( Qt::Checked );
                }
                else
                {
                    listItem->setCheckState( Qt::Unchecked );
                }

                mUI->listWidget_MidiInput->addItem( listItem );
            }
            else
            {
                if ( mDeviceManager.isMidiInputEnabled( "jackmidi" ) )
                {
                    // Prevent user from enabling ALSA MIDI inputs as they conflict with JACK MIDI inputs
                    mUI->listWidget_MidiInput->setEnabled( false );
                }
            }
        }
    }
    else
    {
        // No MIDI
        mUI->listWidget_MidiInput->addItem( getNoDeviceString() );
        mUI->listWidget_MidiInput->setEnabled( false );
    }
}



void AudioSetupDialog::disableAllWidgets()
{
    mUI->comboBox_AudioBackend->addItem( getNoDeviceString() );
    mUI->comboBox_AudioDevice->addItem( getNoDeviceString() );
    mUI->comboBox_OutputChannels->addItem( getNoDeviceString() );
    mUI->comboBox_SampleRate->addItem( getNoDeviceString() );
    mUI->comboBox_BufferSize->addItem( getNoDeviceString() );
    mUI->listWidget_MidiInput->addItem( getNoDeviceString() );

    mUI->comboBox_AudioBackend->setEnabled( false );
    mUI->comboBox_AudioDevice->setEnabled( false );
    mUI->comboBox_OutputChannels->setEnabled( false );
    mUI->comboBox_SampleRate->setEnabled( false );
    mUI->comboBox_BufferSize->setEnabled( false );
    mUI->listWidget_MidiInput->setEnabled( false );
}



void AudioSetupDialog::setUpMidiInputTestSynth()
{
    mSynthAudioSource = new SynthAudioSource();
    mAudioSourcePlayer.setSource( mSynthAudioSource );
    mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
    mDeviceManager.addMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
}



void AudioSetupDialog::tearDownMidiInputTestSynth()
{
    mAudioSourcePlayer.setSource( NULL );
    mDeviceManager.removeMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
    mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );
}



void AudioSetupDialog::setJackMidiInput( const String deviceName )
{
    if ( deviceName.startsWith( "JACK" ) && deviceName.contains( "MIDI" ) )
    {
        // Disable ALSA MIDI inputs as they conflict with JACK MIDI inputs
        const StringArray deviceNames = MidiInput::getDevices();
        for ( int i = 0; i < deviceNames.size(); ++i )
        {
            if ( deviceNames[ i ] != "jackmidi")
            {
                mDeviceManager.setMidiInputEnabled( deviceNames[ i ], false );
            }
        }

        mDeviceManager.setMidiInputEnabled( "jackmidi", true );
    }
    else
    {
        mDeviceManager.setMidiInputEnabled( "jackmidi", false );
    }
}



void AudioSetupDialog::enableStretcherOptions( const RubberBandStretcher::Options options )
{
    mStretcherOptions |= options;
}



void AudioSetupDialog::disableStretcherOptions( const RubberBandStretcher::Options options )
{
    mStretcherOptions &= ~options;
}



//==================================================================================================
// Private Static:

void AudioSetupDialog::showWarningBox( const QString text, const QString infoText )
{
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Warning );
    msgBox.setText( text );
    msgBox.setInformativeText( infoText );
    msgBox.exec();
}



String AudioSetupDialog::getNameForChannelPair( const String& name1, const String& name2 )
{
    String commonBit;

    for ( int j = 0; j < name1.length(); ++j )
    {
        if ( name1.substring( 0, j ).equalsIgnoreCase( name2.substring( 0, j ) ) )
            commonBit = name1.substring( 0, j );
    }

    // Make sure we only split the name at a space, because otherwise, things
    // like "input 11" + "input 12" would become "input 11 + 2"
    while ( commonBit.isNotEmpty() && ! CharacterFunctions::isWhitespace( commonBit.getLastCharacter() ) )
    {
        commonBit = commonBit.dropLastCharacters( 1 );
    }

    return name1.trim() + " + " + name2.substring( commonBit.length() ).trim();
}



//==================================================================================================
// Private Slots:

void AudioSetupDialog::accept()
{
    tearDownMidiInputTestSynth();
    QDialog::accept();
}



void AudioSetupDialog::reject()
{
    tearDownMidiInputTestSynth();

    mDeviceManager.setAudioDeviceSetup( mOriginalConfig, true );

    updateAudioDeviceComboBox();
    updateAudioDeviceComboBox();
    updateOutputChannelComboBox();
    updateSampleRateComboBox();
    updateBufferSizeComboBox();

    QDialog::reject();
}



void AudioSetupDialog::displayDirValidityText( const bool isValid )
{
    if ( isValid )
    {
        mUI->label_DirValidity->clear();
    }
    else
    {
        mUI->label_DirValidity->setText( tr("Dir doesn't exist") );
    }
}



//====================
// "Audio Setup" tab:

void AudioSetupDialog::on_comboBox_AudioBackend_currentIndexChanged( const int index )
{
    // Set audio backend
    AudioIODeviceType* const audioBackendType = mDeviceManager.getAvailableDeviceTypes()[ index ];
    const String audioBackendName = audioBackendType->getTypeName();
    mDeviceManager.setCurrentAudioDeviceType( audioBackendName, true );

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );

    // Set current settings again simply to get any error message that may be produced
    String error = mDeviceManager.setAudioDeviceSetup( config, true );

    // If this is a JACK audio device then also enable JACK MIDI if required
    setJackMidiInput( config.outputDeviceName );

    // When a new audio backend is selected all widgets should be updated
    updateAudioDeviceComboBox();
    updateOutputChannelComboBox();
    updateSampleRateComboBox();
    updateBufferSizeComboBox();
    updateMidiInputListWidget();

    if ( isJackAudioEnabled() && isRealtimeModeEnabled() )
    {
        mUI->checkBox_JackSync->setEnabled( true );
    }
    else
    {
        mUI->checkBox_JackSync->setEnabled( false );
        mUI->checkBox_JackSync->setChecked( false );
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::on_comboBox_AudioDevice_activated( const QString deviceName )
{
    QByteArray charArray = deviceName.toLocal8Bit();
    const String outputDeviceName = charArray.data();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( outputDeviceName != config.outputDeviceName )
    {
        QByteArray charArray = getNoDeviceString().toLocal8Bit();
        if ( outputDeviceName != charArray.data() )
        {
            config.outputDeviceName = outputDeviceName;
        }
        else
        {
            config.outputDeviceName = String::empty;
        }

        config.useDefaultOutputChannels = true;
        error = mDeviceManager.setAudioDeviceSetup( config, true );

        // If this is a JACK audio device then also enable JACK MIDI if required
        setJackMidiInput( outputDeviceName );

        // When a new audio device is selected the following widgets should be updated
        updateOutputChannelComboBox();
        updateSampleRateComboBox();
        updateBufferSizeComboBox();
        updateMidiInputListWidget();
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::on_pushButton_TestTone_clicked()
{
    mDeviceManager.playTestSound();
}



void AudioSetupDialog::on_comboBox_OutputChannels_activated( const int index )
{
    const int64 channelBits = mUI->comboBox_OutputChannels->itemData( index ).toLongLong();
    const BigInteger channels( channelBits );

    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    if ( channels != config.outputChannels )
    {
        config.useDefaultOutputChannels = false;
        config.outputChannels = channels;

        error = mDeviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to set output channels!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::on_comboBox_SampleRate_activated( const int index )
{
    const int sampleRate = mUI->comboBox_SampleRate->itemData( index ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( sampleRate != config.sampleRate )
    {
        config.sampleRate = sampleRate;
        error = mDeviceManager.setAudioDeviceSetup( config, true );

        // Update "Buffer Size" combo box to reflect the change in latency due to the new sample rate
        updateBufferSizeComboBox();
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to set sample rate!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::on_comboBox_BufferSize_activated( const int index )
{
    const int bufferSize = mUI->comboBox_BufferSize->itemData( index ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( bufferSize != config.bufferSize )
    {
        config.bufferSize = bufferSize;
        error = mDeviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to set buffer size!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::on_listWidget_MidiInput_itemClicked( QListWidgetItem* item )
{
    QByteArray charArray = item->text().toLocal8Bit();
    const String midiInputName( charArray.data() );

    if ( item->checkState() == Qt::Checked )
    {
        mDeviceManager.setMidiInputEnabled( midiInputName, true );
    }
    else
    {
        mDeviceManager.setMidiInputEnabled( midiInputName, false );
    }
}



void AudioSetupDialog::on_buttonBox_clicked( QAbstractButton* button )
{
    QDialogButtonBox::StandardButton stdButton = mUI->buttonBox->standardButton( button );

    if ( stdButton == QDialogButtonBox::Save )
    {
        // Save audio setup config
        ScopedPointer<XmlElement> stateXml( mDeviceManager.createStateXml() );

        if ( stateXml != NULL )
        {
            File audioConfigFile( AUDIO_CONFIG_FILE_PATH );
            audioConfigFile.create();
            stateXml->writeToFile( audioConfigFile, String::empty );
        }

        // Save paths config
        XmlElement docElement( "paths" );

        QString tempDir = mUI->lineEdit_TempDir->text();

        XmlElement* tempDirElem = new XmlElement( "temp_dir" );
        tempDirElem->setAttribute( "path", tempDir.toLocal8Bit().data() );
        docElement.addChildElement( tempDirElem );

        File pathsConfigFile( PATHS_CONFIG_FILE_PATH );
        pathsConfigFile.create();
        docElement.writeToFile( pathsConfigFile, String::empty );
    }
}



void AudioSetupDialog::on_checkBox_MidiInputTestTone_clicked( const bool isChecked )
{
    if ( isChecked )
    {
        setUpMidiInputTestSynth();
    }
    else
    {
        tearDownMidiInputTestSynth();
    }
}



//====================
// "Time Stretch" tab:

void AudioSetupDialog::on_radioButton_Offline_clicked()
{
    foreach ( QAbstractButton* button, mUI->buttonGroup_Timestretch->buttons() )
    {
        button->setEnabled( true );
    }
    mUI->radioButton_Elastic->setChecked( true );

    foreach ( QAbstractButton* button, mUI->buttonGroup_PitchShifting->buttons() )
    {
        button->setEnabled( false );
    }
    mUI->radioButton_HighSpeed->setChecked( true );

    mUI->checkBox_JackSync->setEnabled( false );
    mUI->checkBox_JackSync->setChecked( false );

    disableStretcherOptions( RubberBandStretcher::OptionProcessRealTime );
    disableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );

    emit realtimeModeToggled( false );
}



void AudioSetupDialog::on_radioButton_RealTime_clicked()
{
    foreach ( QAbstractButton* button, mUI->buttonGroup_Timestretch->buttons() )
    {
        button->setEnabled( false );
    }
    mUI->radioButton_Precise->setChecked( true );

    foreach ( QAbstractButton* button, mUI->buttonGroup_PitchShifting->buttons() )
    {
        button->setEnabled( true );
    }
    mUI->radioButton_HighConsistency->setChecked( true );

    if ( isJackAudioEnabled() )
    {
        mUI->checkBox_JackSync->setEnabled( true );
    }
    else
    {
        mUI->checkBox_JackSync->setEnabled( false );
        mUI->checkBox_JackSync->setChecked( false );
    }

    enableStretcherOptions( RubberBandStretcher::OptionProcessRealTime );
    enableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );

    emit realtimeModeToggled( true );
}



void AudioSetupDialog::on_radioButton_Elastic_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    emit stretchOptionChanged( RubberBandStretcher::OptionStretchElastic );
}



void AudioSetupDialog::on_radioButton_Precise_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    emit stretchOptionChanged( RubberBandStretcher::OptionStretchPrecise );
}



void AudioSetupDialog::on_radioButton_Crisp_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsMixed | RubberBandStretcher::OptionTransientsSmooth );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsCrisp );
}



void AudioSetupDialog::on_radioButton_Mixed_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsSmooth );
    enableStretcherOptions( RubberBandStretcher::OptionTransientsMixed );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsMixed );
}



void AudioSetupDialog::on_radioButton_Smooth_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsMixed );
    enableStretcherOptions( RubberBandStretcher::OptionTransientsSmooth );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsSmooth );
}



void AudioSetupDialog::on_radioButton_Laminar_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPhaseIndependent );
    emit phaseOptionChanged( RubberBandStretcher::OptionPhaseLaminar );
}



void AudioSetupDialog::on_radioButton_Independent_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionPhaseIndependent );
    emit phaseOptionChanged( RubberBandStretcher::OptionPhaseIndependent );
}



void AudioSetupDialog::on_radioButton_Standard_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowShort | RubberBandStretcher::OptionWindowLong );
    emit windowOptionChanged();
}



void AudioSetupDialog::on_radioButton_Short_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowLong );
    enableStretcherOptions( RubberBandStretcher::OptionWindowShort );
    emit windowOptionChanged();
}



void AudioSetupDialog::on_radioButton_Long_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowShort );
    enableStretcherOptions( RubberBandStretcher::OptionWindowLong );
    emit windowOptionChanged();
}



void AudioSetupDialog::on_radioButton_Shifted_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionFormantPreserved );
    emit formantOptionChanged( RubberBandStretcher::OptionFormantShifted );
}



void AudioSetupDialog::on_radioButton_Preserved_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionFormantPreserved );
    emit formantOptionChanged( RubberBandStretcher::OptionFormantPreserved );
}



void AudioSetupDialog::on_radioButton_HighSpeed_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality | RubberBandStretcher::OptionPitchHighConsistency );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighSpeed );
}



void AudioSetupDialog::on_radioButton_HighQuality_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighQuality );
}



void AudioSetupDialog::on_radioButton_HighConsistency_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighConsistency );
}



void AudioSetupDialog::on_checkBox_JackSync_toggled( const bool isChecked )
{
    emit jackSyncToggled( isChecked );
}



//====================
// "Paths" tab:

void AudioSetupDialog::on_pushButton_ChooseTempDir_clicked()
{
    const QString dir = QFileDialog::getExistingDirectory( this, tr("Choose Directory"), "/",
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    mUI->lineEdit_TempDir->setText( dir );
}
