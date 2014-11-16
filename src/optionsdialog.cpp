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

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "globals.h"
#include "messageboxes.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcessEnvironment>
#include <QDebug>


//==================================================================================================
// Public:

OptionsDialog::OptionsDialog( AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    m_UI( new Ui::OptionsDialog ),
    m_deviceManager( deviceManager ),
    m_stretcherOptions( RubberBandStretcher::DefaultOptions )
{
    // Setup user interface
    m_UI->setupUi( this );


    // Get available audio backends (ALSA, JACK, etc.)
    const OwnedArray<AudioIODeviceType>& audioBackendTypes = m_deviceManager.getAvailableDeviceTypes();


    // Populate audio backend combo box
    for ( int i = 0; i < audioBackendTypes.size(); ++i )
    {
        const QString backendName = audioBackendTypes.getUnchecked( i )->getTypeName().toRawUTF8();
        m_UI->comboBox_AudioBackend->addItem( backendName );
    }


    // Select current audio backend
    const QString backendName = m_deviceManager.getCurrentAudioDeviceType().toRawUTF8();
    const int index = m_UI->comboBox_AudioBackend->findText( backendName );

    m_UI->comboBox_AudioBackend->setCurrentIndex( index );
    on_comboBox_AudioBackend_activated( index ); // This will update all the other widgets


    // Paths
    setTempDirPath();
}



OptionsDialog::~OptionsDialog()
{
    delete m_UI;
}



void OptionsDialog::setCurrentTab( const Tab tab )
{
    m_UI->tabWidget->setCurrentIndex( tab );
}



bool OptionsDialog::isRealtimeModeEnabled() const
{
    return m_UI->radioButton_RealTime->isChecked();
}



void OptionsDialog::setStretcherOptions( const RubberBandStretcher::Options options )
{
    if ( options & RubberBandStretcher::OptionProcessRealTime )
    {
        m_UI->radioButton_RealTime->click();
    }
    else // RubberBandStretcher::OptionProcessOffline
    {
        m_UI->radioButton_Offline->click();
    }


    if ( options & RubberBandStretcher::OptionStretchPrecise )
    {
        m_UI->radioButton_Precise->click();
    }
    else // RubberBandStretcher::OptionStretchElastic
    {
        m_UI->radioButton_Elastic->click();
    }


    if ( options & RubberBandStretcher::OptionTransientsMixed )
    {
        m_UI->radioButton_Mixed->click();
    }
    else if ( options & RubberBandStretcher::OptionTransientsSmooth )
    {
        m_UI->radioButton_Smooth->click();
    }
    else // RubberBandStretcher::OptionTransientsCrisp
    {
        m_UI->radioButton_Crisp->click();
    }


    if ( options & RubberBandStretcher::OptionPhaseIndependent )
    {
        m_UI->radioButton_Independent->click();
    }
    else // RubberBandStretcher::OptionPhaseLaminar
    {
        m_UI->radioButton_Laminar->click();
    }


    if ( options & RubberBandStretcher::OptionWindowShort )
    {
        m_UI->radioButton_Short->click();
    }
    else if ( options & RubberBandStretcher::OptionWindowLong )
    {
        m_UI->radioButton_Long->click();
    }
    else // RubberBandStretcher::OptionWindowStandard
    {
        m_UI->radioButton_Standard->click();
    }


    if ( options & RubberBandStretcher::OptionFormantPreserved )
    {
        m_UI->radioButton_Preserved->click();
    }
    else // RubberBandStretcher::OptionFormantShifted
    {
        m_UI->radioButton_Shifted->click();
    }


    if ( options & RubberBandStretcher::OptionPitchHighQuality )
    {
        m_UI->radioButton_HighQuality->click();
    }
    else if ( options & RubberBandStretcher::OptionPitchHighConsistency )
    {
        m_UI->radioButton_HighConsistency->click();
    }
    else // RubberBandStretcher::OptionPitchHighSpeed
    {
        m_UI->radioButton_HighSpeed->click();
    }
}



bool OptionsDialog::isJackSyncEnabled() const
{
    return m_UI->checkBox_JackSync->isChecked();
}



void OptionsDialog::enableJackSync()
{
    m_UI->checkBox_JackSync->setChecked( true );
}



//==================================================================================================
// Protected:

void OptionsDialog::changeEvent( QEvent* event )
{
    QDialog::changeEvent( event );

    switch ( event->type() )
    {
    case QEvent::LanguageChange:
        m_UI->retranslateUi( this );
        break;
    default:
        break;
    }
}



void OptionsDialog::showEvent( QShowEvent* event )
{
    // If the dialog is not being maximised, i.e. it has not previoulsy been minimised...
    if ( ! event->spontaneous() )
    {
        // Get current audio settings and store them so that any changes
        // can be reverted if the user later clicks "Cancel"
        m_deviceManager.getAudioDeviceSetup( m_originalConfig );

        const QString backendName = m_deviceManager.getCurrentAudioDeviceType().toRawUTF8();
        m_originalBackendIndex = m_UI->comboBox_AudioBackend->findText( backendName );


        if ( m_UI->checkBox_MidiInputTestTone->isChecked() )
        {
            setUpMidiInputTestSynth();
        }
    }

    QDialog::showEvent( event );
}



void OptionsDialog::closeEvent( QCloseEvent* event )
{
    tearDownMidiInputTestSynth();
    event->accept();
}



//==================================================================================================
// Private:

void OptionsDialog::setTempDirPath()
{
    // Set default temp dir
    QString tempDirPath = QDir::tempPath();

    // Try to load paths config file
    ScopedPointer<XmlElement> docElement;
    docElement = XmlDocument::parse( File( PATHS_CONFIG_FILE_PATH ) );

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

    // Set up validator
    m_directoryValidator = new DirectoryValidator();
    m_UI->lineEdit_TempDir->setValidator( m_directoryValidator );

    QObject::connect( m_directoryValidator, SIGNAL( isValid(bool) ),
                      this, SLOT( displayDirValidityText(bool) ) );

    m_UI->lineEdit_TempDir->setText( tempDirPath );

    // Create a sub-dir with a unique name; there may be multiple instances of Shuriken running
    QDir tempDir( tempDirPath );

    QString userName = QProcessEnvironment::systemEnvironment().value( "USER", "user" );

    QString pid;
    pid.setNum( QCoreApplication::applicationPid() );

    QString subDirName( "shuriken-" );
    subDirName.append( userName );
    subDirName.append( "-" );
    subDirName.append( pid );

    bool isSuccessful = tempDir.mkdir( subDirName );

    if ( isSuccessful )
    {
        m_tempDirPath = tempDir.absoluteFilePath( subDirName );
    }
}



bool OptionsDialog::isJackAudioEnabled() const
{
    return ( m_deviceManager.getCurrentAudioDeviceType() == "JACK" );
}



void OptionsDialog::updateAudioDeviceComboBox()
{
    AudioIODeviceType* const audioBackendType = m_deviceManager.getCurrentDeviceTypeObject();

    if ( audioBackendType != NULL )
    {
        // Get available audio devices
        audioBackendType->scanForDevices();
        const StringArray audioDeviceNames( audioBackendType->getDeviceNames( false ) );

        // Populate combo box
        m_UI->comboBox_AudioDevice->clear();

        for ( int i = 0; i < audioDeviceNames.size(); ++i )
        {
            m_UI->comboBox_AudioDevice->addItem( audioDeviceNames[i].toRawUTF8() );
        }

        m_UI->comboBox_AudioDevice->addItem( getNoDeviceString() );

        // Select current audio device
        AudioIODevice* const currentAudioDevice = m_deviceManager.getCurrentAudioDevice();

        if ( currentAudioDevice != NULL )
        {
            const int index = audioBackendType->getIndexOfDevice( currentAudioDevice, false );
            m_UI->comboBox_AudioDevice->setCurrentIndex( index );
        }
    }
}



void OptionsDialog::updateOutputChannelComboBox()
{
    m_UI->comboBox_OutputChannels->clear();

    if ( isJackAudioEnabled() )
    {
        m_UI->comboBox_OutputChannels->setVisible( false );
        m_UI->label_OutputChannels->setVisible( false );
    }
    else
    {
        m_UI->comboBox_OutputChannels->setVisible( true );
        m_UI->label_OutputChannels->setVisible( true );

        AudioIODevice* const currentAudioDevice = m_deviceManager.getCurrentAudioDevice();

        if ( currentAudioDevice != NULL )
        {
            m_UI->comboBox_OutputChannels->setEnabled( true );

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

                m_UI->comboBox_OutputChannels->addItem( channelNames[i].toRawUTF8(), channels.toInt64() );
            }
        }
        else
        {
            m_UI->comboBox_OutputChannels->addItem( getNoDeviceString() );
            m_UI->comboBox_OutputChannels->setEnabled( false );
        }
    }
}



void OptionsDialog::updateSampleRateComboBox()
{
    m_UI->comboBox_SampleRate->clear();

    AudioIODevice* const currentAudioDevice = m_deviceManager.getCurrentAudioDevice();

    if ( currentAudioDevice != NULL )
    {
        m_UI->comboBox_SampleRate->setEnabled( true );

        const Array<double> sampleRates( currentAudioDevice->getAvailableSampleRates() );

        for ( int i = 0; i < sampleRates.size(); ++i )
        {
            const int sampleRate = roundToInt( sampleRates[i] );
            m_UI->comboBox_SampleRate->addItem( QString::number( sampleRate ) + " Hz", sampleRate );
        }

        // Select current sample rate
        const int currentSampleRate = roundToInt( currentAudioDevice->getCurrentSampleRate() );
        const int index = m_UI->comboBox_SampleRate->findData( currentSampleRate );
        m_UI->comboBox_SampleRate->setCurrentIndex( index );
    }
    else
    {
        m_UI->comboBox_SampleRate->addItem( getNoDeviceString() );
        m_UI->comboBox_SampleRate->setEnabled( false );
    }

    if ( isJackAudioEnabled() )
    {
        m_UI->comboBox_SampleRate->setEnabled( false );
    }
}



void OptionsDialog::updateBufferSizeComboBox()
{
    m_UI->comboBox_BufferSize->clear();

    AudioIODevice* const currentAudioDevice = m_deviceManager.getCurrentAudioDevice();

    if ( currentAudioDevice != NULL )
    {
        m_UI->comboBox_BufferSize->setEnabled( true );

        const Array<int> bufferSizes( currentAudioDevice->getAvailableBufferSizes() );
        double currentRate = currentAudioDevice->getCurrentSampleRate();

        if ( currentRate == 0 )
            currentRate = 48000.0;

        const char format = 'f'; // See QString "Argument Formats" for details
        const int precision = 1;

        for ( int i = 0; i < bufferSizes.size(); ++i )
        {
            const int bufferSize = bufferSizes[i];
            m_UI->comboBox_BufferSize->addItem( QString::number( bufferSize )
                                              + " samples ("
                                              + QString::number( bufferSize * 1000.0 / currentRate, format, precision )
                                              + " ms)",
                                              bufferSize );
        }

        // Select current buffer size
        const int index = m_UI->comboBox_BufferSize->findData( currentAudioDevice->getCurrentBufferSizeSamples() );
        m_UI->comboBox_BufferSize->setCurrentIndex( index );
    }
    else
    {
        m_UI->comboBox_BufferSize->addItem( getNoDeviceString() );
        m_UI->comboBox_BufferSize->setEnabled( false );
    }

    if ( isJackAudioEnabled() )
    {
        m_UI->comboBox_BufferSize->setEnabled( false );
    }
}



void OptionsDialog::updateMidiInputListWidget()
{
    m_UI->listWidget_MidiInput->clear();

    const int numMidiDevices = MidiInput::getDevices().size();

    if ( numMidiDevices > 0 )
    {
        m_UI->listWidget_MidiInput->setEnabled( true );

        for ( int i = 0; i < numMidiDevices; ++i )
        {
            const String midiDeviceName( MidiInput::getDevices()[ i ] );

            if ( ! isJackMidiDevice( midiDeviceName ) )
            {
                QListWidgetItem* const listItem = new QListWidgetItem();

                listItem->setText( midiDeviceName.toRawUTF8() );
                listItem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );

                if ( m_deviceManager.isMidiInputEnabled( midiDeviceName ) )
                {
                    listItem->setCheckState( Qt::Checked );
                }
                else
                {
                    listItem->setCheckState( Qt::Unchecked );
                }

                m_UI->listWidget_MidiInput->addItem( listItem );
            }
            else
            {
                if ( m_deviceManager.isMidiInputEnabled( "jackmidi" ) || m_deviceManager.isMidiInputEnabled( "a2jmidid" ) )
                {
                    // Prevent user from enabling ALSA MIDI inputs as they conflict with JACK MIDI inputs
                    m_UI->listWidget_MidiInput->setEnabled( false );
                }
            }
        }
    }
    else
    {
        // No MIDI
        m_UI->listWidget_MidiInput->addItem( getNoDeviceString() );
        m_UI->listWidget_MidiInput->setEnabled( false );
    }
}



void OptionsDialog::disableAllWidgets()
{
    m_UI->comboBox_AudioBackend->addItem( getNoDeviceString() );
    m_UI->comboBox_AudioDevice->addItem( getNoDeviceString() );
    m_UI->comboBox_OutputChannels->addItem( getNoDeviceString() );
    m_UI->comboBox_SampleRate->addItem( getNoDeviceString() );
    m_UI->comboBox_BufferSize->addItem( getNoDeviceString() );
    m_UI->listWidget_MidiInput->addItem( getNoDeviceString() );

    m_UI->comboBox_AudioBackend->setEnabled( false );
    m_UI->comboBox_AudioDevice->setEnabled( false );
    m_UI->comboBox_OutputChannels->setEnabled( false );
    m_UI->comboBox_SampleRate->setEnabled( false );
    m_UI->comboBox_BufferSize->setEnabled( false );
    m_UI->listWidget_MidiInput->setEnabled( false );
}



void OptionsDialog::setUpMidiInputTestSynth()
{
    m_synthAudioSource = new SynthAudioSource();
    m_audioSourcePlayer.setSource( m_synthAudioSource );
    m_deviceManager.addAudioCallback( &m_audioSourcePlayer );
    m_deviceManager.addMidiInputCallback( String::empty, &(m_synthAudioSource->m_midiCollector) );
}



void OptionsDialog::tearDownMidiInputTestSynth()
{
    m_audioSourcePlayer.setSource( NULL );
    m_deviceManager.removeMidiInputCallback( String::empty, &(m_synthAudioSource->m_midiCollector) );
    m_deviceManager.removeAudioCallback( &m_audioSourcePlayer );
}



void OptionsDialog::setJackMidiInput( const String deviceName )
{
    const StringArray deviceNames = MidiInput::getDevices();

    if ( deviceName.startsWith( "JACK" ) && deviceName.contains( "MIDI" ) )
    {
        // Disable ALSA MIDI inputs as they conflict with JACK MIDI inputs
        for ( int i = 0; i < deviceNames.size(); ++i )
        {
            if ( isJackMidiDevice( deviceNames[i] ) )
            {
                m_deviceManager.setMidiInputEnabled( deviceNames[i], true );
            }
            else
            {
                m_deviceManager.setMidiInputEnabled( deviceNames[i], false );
            }
        }
    }
    else
    {
        // Disable JACK MIDI inputs
        for ( int i = 0; i < deviceNames.size(); ++i )
        {
            if ( isJackMidiDevice( deviceNames[i] ) )
            {
                m_deviceManager.setMidiInputEnabled( deviceNames[i], false );
            }
        }
    }
}



void OptionsDialog::enableStretcherOptions( const RubberBandStretcher::Options options )
{
    m_stretcherOptions |= options;
}



void OptionsDialog::disableStretcherOptions( const RubberBandStretcher::Options options )
{
    m_stretcherOptions &= ~options;
}



void OptionsDialog::saveConfig()
{
    // Save audio setup config
    ScopedPointer<XmlElement> stateXml( m_deviceManager.createStateXml() );

    if ( stateXml != NULL )
    {
        File audioConfigFile( AUDIO_CONFIG_FILE_PATH );
        audioConfigFile.create();
        stateXml->writeToFile( audioConfigFile, String::empty );
    }

    // Save paths config
    XmlElement docElement( "paths" );

    const QString tempDir = m_UI->lineEdit_TempDir->text();

    XmlElement* tempDirElem = new XmlElement( "temp_dir" );
    tempDirElem->setAttribute( "path", tempDir.toLocal8Bit().data() );
    docElement.addChildElement( tempDirElem );

    File pathsConfigFile( PATHS_CONFIG_FILE_PATH );
    pathsConfigFile.create();
    docElement.writeToFile( pathsConfigFile, String::empty );
}



//==================================================================================================
// Private Static:

String OptionsDialog::getNameForChannelPair( const String& name1, const String& name2 )
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



bool OptionsDialog::isJackMidiDevice( const String deviceName )
{
    if ( deviceName == "jackmidi" || deviceName == "a2jmidid" )
    {
        return true;
    }
    return false;
}



//==================================================================================================
// Private Slots:

void OptionsDialog::accept()
{
    tearDownMidiInputTestSynth();

    saveConfig();

    QDialog::accept();
}



void OptionsDialog::reject()
{
    tearDownMidiInputTestSynth();

    m_UI->comboBox_AudioBackend->setCurrentIndex( m_originalBackendIndex );
    on_comboBox_AudioBackend_activated( m_originalBackendIndex ); // This will update all the other widgets

    m_deviceManager.setAudioDeviceSetup( m_originalConfig, true );

    updateAudioDeviceComboBox();
    updateOutputChannelComboBox();
    updateSampleRateComboBox();
    updateBufferSizeComboBox();

    QDir tempDir( m_tempDirPath );
    tempDir.cdUp();
    m_UI->lineEdit_TempDir->setText( tempDir.absolutePath() );

    QDialog::reject();
}



void OptionsDialog::displayDirValidityText( const bool isValid )
{
    if ( isValid )
    {
        m_UI->label_DirValidity->clear();
    }
    else
    {
        m_UI->label_DirValidity->setText( tr("Dir doesn't exist") );
    }
}



//====================
// "Audio Setup" tab:

void OptionsDialog::on_comboBox_AudioBackend_activated( const int index )
{
    // Set audio backend
    AudioIODeviceType* const audioBackendType = m_deviceManager.getAvailableDeviceTypes()[ index ];
    const String audioBackendName = audioBackendType->getTypeName();
    m_deviceManager.setCurrentAudioDeviceType( audioBackendName, true );

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );

    // Set current settings again simply to get any error message that may be produced
    String error = m_deviceManager.setAudioDeviceSetup( config, true );

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
        m_UI->checkBox_JackSync->setEnabled( true );
    }
    else
    {
        m_UI->checkBox_JackSync->setEnabled( false );
        m_UI->checkBox_JackSync->setChecked( false );
    }

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void OptionsDialog::on_comboBox_AudioDevice_activated( const QString deviceName )
{
    QByteArray charArray = deviceName.toLocal8Bit();
    const String outputDeviceName = charArray.data();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );
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
        error = m_deviceManager.setAudioDeviceSetup( config, true );

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
        MessageBoxes::showWarningDialog( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void OptionsDialog::on_pushButton_TestTone_clicked()
{
    m_deviceManager.playTestSound();
}



void OptionsDialog::on_comboBox_OutputChannels_activated( const int index )
{
    const int64 channelBits = m_UI->comboBox_OutputChannels->itemData( index ).toLongLong();
    const BigInteger channels( channelBits );

    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );
    String error;

    if ( channels != config.outputChannels )
    {
        config.useDefaultOutputChannels = false;
        config.outputChannels = channels;

        error = m_deviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Error when trying to set output channels!"), error.toRawUTF8() );
    }
}



void OptionsDialog::on_comboBox_SampleRate_activated( const int index )
{
    const int sampleRate = m_UI->comboBox_SampleRate->itemData( index ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( sampleRate != config.sampleRate )
    {
        config.sampleRate = sampleRate;
        error = m_deviceManager.setAudioDeviceSetup( config, true );

        // Update "Buffer Size" combo box to reflect the change in latency due to the new sample rate
        updateBufferSizeComboBox();
    }

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Error when trying to set sample rate!"), error.toRawUTF8() );
    }
}



void OptionsDialog::on_comboBox_BufferSize_activated( const int index )
{
    const int bufferSize = m_UI->comboBox_BufferSize->itemData( index ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    m_deviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( bufferSize != config.bufferSize )
    {
        config.bufferSize = bufferSize;
        error = m_deviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        MessageBoxes::showWarningDialog( tr("Error when trying to set buffer size!"), error.toRawUTF8() );
    }
}



void OptionsDialog::on_listWidget_MidiInput_itemClicked( QListWidgetItem* item )
{
    QByteArray charArray = item->text().toLocal8Bit();
    const String midiInputName( charArray.data() );

    if ( item->checkState() == Qt::Checked )
    {
        m_deviceManager.setMidiInputEnabled( midiInputName, true );
    }
    else
    {
        m_deviceManager.setMidiInputEnabled( midiInputName, false );
    }
}



void OptionsDialog::on_checkBox_MidiInputTestTone_clicked( const bool isChecked )
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

void OptionsDialog::on_radioButton_Offline_clicked()
{
    foreach ( QAbstractButton* button, m_UI->buttonGroup_Timestretch->buttons() )
    {
        button->setEnabled( true );
    }
    m_UI->radioButton_Elastic->setChecked( true );

    foreach ( QAbstractButton* button, m_UI->buttonGroup_PitchShifting->buttons() )
    {
        button->setEnabled( false );
    }
    m_UI->radioButton_HighSpeed->setChecked( true );

    m_UI->checkBox_JackSync->setEnabled( false );
    m_UI->checkBox_JackSync->setChecked( false );

    disableStretcherOptions( RubberBandStretcher::OptionProcessRealTime );
    disableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );

    emit realtimeModeToggled( false );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_RealTime_clicked()
{
    foreach ( QAbstractButton* button, m_UI->buttonGroup_Timestretch->buttons() )
    {
        button->setEnabled( false );
    }
    m_UI->radioButton_Precise->setChecked( true );

    foreach ( QAbstractButton* button, m_UI->buttonGroup_PitchShifting->buttons() )
    {
        button->setEnabled( true );
    }
    m_UI->radioButton_HighConsistency->setChecked( true );

    if ( isJackAudioEnabled() )
    {
        m_UI->checkBox_JackSync->setEnabled( true );
    }
    else
    {
        m_UI->checkBox_JackSync->setEnabled( false );
        m_UI->checkBox_JackSync->setChecked( false );
    }

    enableStretcherOptions( RubberBandStretcher::OptionProcessRealTime );
    enableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );

    emit realtimeModeToggled( true );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Elastic_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    emit stretchOptionChanged( RubberBandStretcher::OptionStretchElastic );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Precise_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionStretchPrecise );
    emit stretchOptionChanged( RubberBandStretcher::OptionStretchPrecise );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Crisp_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsMixed | RubberBandStretcher::OptionTransientsSmooth );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsCrisp );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Mixed_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsSmooth );
    enableStretcherOptions( RubberBandStretcher::OptionTransientsMixed );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsMixed );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Smooth_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionTransientsMixed );
    enableStretcherOptions( RubberBandStretcher::OptionTransientsSmooth );
    emit transientsOptionChanged( RubberBandStretcher::OptionTransientsSmooth );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Laminar_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPhaseIndependent );
    emit phaseOptionChanged( RubberBandStretcher::OptionPhaseLaminar );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Independent_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionPhaseIndependent );
    emit phaseOptionChanged( RubberBandStretcher::OptionPhaseIndependent );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Standard_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowShort | RubberBandStretcher::OptionWindowLong );
    emit windowOptionChanged();
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Short_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowLong );
    enableStretcherOptions( RubberBandStretcher::OptionWindowShort );
    emit windowOptionChanged();
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Long_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionWindowShort );
    enableStretcherOptions( RubberBandStretcher::OptionWindowLong );
    emit windowOptionChanged();
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Shifted_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionFormantPreserved );
    emit formantOptionChanged( RubberBandStretcher::OptionFormantShifted );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_Preserved_clicked()
{
    enableStretcherOptions( RubberBandStretcher::OptionFormantPreserved );
    emit formantOptionChanged( RubberBandStretcher::OptionFormantPreserved );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_HighSpeed_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality | RubberBandStretcher::OptionPitchHighConsistency );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighSpeed );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_HighQuality_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighQuality );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_radioButton_HighConsistency_clicked()
{
    disableStretcherOptions( RubberBandStretcher::OptionPitchHighQuality );
    enableStretcherOptions( RubberBandStretcher::OptionPitchHighConsistency );
    emit pitchOptionChanged( RubberBandStretcher::OptionPitchHighConsistency );
    emit timeStretchOptionsChanged();
}



void OptionsDialog::on_checkBox_JackSync_toggled( const bool isChecked )
{
    emit jackSyncToggled( isChecked );
    emit timeStretchOptionsChanged();
}



//====================
// "Paths" tab:

void OptionsDialog::on_pushButton_ChooseTempDir_clicked()
{
    const QString dir = QFileDialog::getExistingDirectory( this, tr("Choose Directory"), "/",
                                                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    m_UI->lineEdit_TempDir->setText( dir );
}
