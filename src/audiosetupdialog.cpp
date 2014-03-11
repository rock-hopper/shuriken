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
#include "simplesynth.h"
#include <QMessageBox>


//==================================================================================================
// Public:

AudioSetupDialog::AudioSetupDialog( AudioDeviceManager& deviceManager, QWidget* parent ) :
    QDialog( parent ),
    mUI( new Ui::AudioSetupDialog ),
    mDeviceManager( deviceManager )
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
    const QString currentAudioBackendName = mDeviceManager.getCurrentAudioDeviceType().toRawUTF8();
    const int index = mUI->comboBox_AudioBackend->findText( currentAudioBackendName );
    mUI->comboBox_AudioBackend->setCurrentIndex( index );
    setAudioBackend( index ); // This will also update all the other widgets

    // Connect signals and slots
    QObject::connect( mUI->comboBox_AudioBackend, SIGNAL(currentIndexChanged(int)),
                      this, SLOT(setAudioBackend(int)) );

    QObject::connect( mUI->comboBox_AudioDevice, SIGNAL(activated(int)),
                      this, SLOT(setAudioDevice(int)) );

    QObject::connect( mUI->comboBox_OutputChannels, SIGNAL(activated(int)),
                      this, SLOT(setOutputChannel(int)) );

    QObject::connect( mUI->comboBox_SampleRate, SIGNAL(activated(int)),
                      this, SLOT(setSampleRate(int)) );

    QObject::connect( mUI->comboBox_BufferSize, SIGNAL(activated(int)),
                      this, SLOT(setBufferSize(int)) );

    QObject::connect( mUI->listWidget_MidiInput, SIGNAL(itemClicked(QListWidgetItem*)),
                      this, SLOT(enableMidiInput(QListWidgetItem*)) );

    QObject::connect( mUI->pushButton_TestTone, SIGNAL(clicked()),
                      this, SLOT(playTestSound()) );


    mUI->label_MidiInputTestTone->setText( tr( "MIDI input test tone enabled" ) );
}



AudioSetupDialog::~AudioSetupDialog()
{
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
    if ( ! event->spontaneous() ) // If this dialog is not being maximised after previously being minimized...
    {
        // Set up MIDI input test synth
        mSynthAudioSource = new SynthAudioSource();
        mAudioSourcePlayer.setSource( mSynthAudioSource );
        mDeviceManager.addAudioCallback( &mAudioSourcePlayer );
        mDeviceManager.addMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
    }

    QDialog::showEvent( event );
}



void AudioSetupDialog::closeEvent( QCloseEvent* event )
{
    // Tear down MIDI input test synth
    mAudioSourcePlayer.setSource( NULL );
    mDeviceManager.removeMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
    mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );

    event->accept();
}



//==================================================================================================
// Private:

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

        mUI->comboBox_AudioDevice->addItem( getNoDeviceName() );

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

    if ( mDeviceManager.getCurrentAudioDeviceType() == "JACK" )
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

            for ( int i = 0; i < channelNames.size(); ++i )
            {
                mUI->comboBox_OutputChannels->addItem( channelNames[i].toRawUTF8() );
            }
        }
        else
        {
            mUI->comboBox_OutputChannels->addItem( getNoDeviceName() );
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
        mUI->comboBox_SampleRate->addItem( getNoDeviceName() );
        mUI->comboBox_SampleRate->setEnabled( false );
    }

    if ( mDeviceManager.getCurrentAudioDeviceType() == "JACK" )
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
        mUI->comboBox_BufferSize->addItem( getNoDeviceName() );
        mUI->comboBox_BufferSize->setEnabled( false );
    }

    if ( mDeviceManager.getCurrentAudioDeviceType() == "JACK" )
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
        mUI->listWidget_MidiInput->addItem( getNoDeviceName() );
        mUI->listWidget_MidiInput->setEnabled( false );
    }
}



void AudioSetupDialog::disableAllWidgets()
{
    mUI->comboBox_AudioBackend->addItem( getNoDeviceName() );
    mUI->comboBox_AudioDevice->addItem( getNoDeviceName() );
    mUI->comboBox_OutputChannels->addItem( getNoDeviceName() );
    mUI->comboBox_SampleRate->addItem( getNoDeviceName() );
    mUI->comboBox_BufferSize->addItem( getNoDeviceName() );
    mUI->listWidget_MidiInput->addItem( getNoDeviceName() );

    mUI->comboBox_AudioBackend->setEnabled( false );
    mUI->comboBox_AudioDevice->setEnabled( false );
    mUI->comboBox_OutputChannels->setEnabled( false );
    mUI->comboBox_SampleRate->setEnabled( false );
    mUI->comboBox_BufferSize->setEnabled( false );
    mUI->listWidget_MidiInput->setEnabled( false );
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

void AudioSetupDialog::setAudioBackend( const int currentIndex )
{
    // Set audio backend
    AudioIODeviceType* const audioBackendType = mDeviceManager.getAvailableDeviceTypes()[ currentIndex ];
    const String audioBackendName = audioBackendType->getTypeName();
    mDeviceManager.setCurrentAudioDeviceType( audioBackendName, true );

    // If the audio backend is JACK then also enable JACK MIDI if required
    if ( audioBackendName == "JACK" )
    {
        const int index = audioBackendType->getDefaultDeviceIndex( false );
        const String defaultDeviceName = audioBackendType->getDeviceNames( false )[ index ];

        if ( defaultDeviceName.contains( "MIDI" ) )
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

    // When a new audio backend is selected all widgets should be updated
    updateAudioDeviceComboBox();
    updateOutputChannelComboBox();
    updateSampleRateComboBox();
    updateBufferSizeComboBox();
    updateMidiInputListWidget();
}



void AudioSetupDialog::setAudioDevice( const int currentIndex )
{
    QByteArray charArray = mUI->comboBox_AudioDevice->itemText( currentIndex ).toLocal8Bit();
    const String newAudioDeviceName = charArray.data();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( newAudioDeviceName != config.outputDeviceName )
    {
        QByteArray charArray = getNoDeviceName().toLocal8Bit();
        if ( newAudioDeviceName != charArray.data() )
        {
            config.outputDeviceName = newAudioDeviceName;
        }
        else
        {
            config.outputDeviceName = String::empty;
        }

        config.useDefaultOutputChannels = true;
        error = mDeviceManager.setAudioDeviceSetup( config, true );

        // If this is a JACK audio device then also enable JACK MIDI if required
        if ( newAudioDeviceName.startsWith( "JACK" ) )
        {
            if ( newAudioDeviceName.contains( "MIDI" ) )
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



void AudioSetupDialog::setOutputChannel( const int currentIndex )
{
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    const int previousIndex = config.outputChannels.findNextSetBit( 0 ) / 2;

    if ( currentIndex != previousIndex )
    {
        config.useDefaultOutputChannels = false;
        config.outputChannels.clear();
        config.outputChannels.setRange( currentIndex * 2, 2, true );

        error = mDeviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::setSampleRate( const int currentIndex )
{
    const int newSampleRate = mUI->comboBox_SampleRate->itemData( currentIndex ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( newSampleRate != config.sampleRate )
    {
        config.sampleRate = newSampleRate;
        error = mDeviceManager.setAudioDeviceSetup( config, true );

        // Update "Buffer Size" combo box to reflect the change in latency due to the new sample rate
        updateBufferSizeComboBox();
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::setBufferSize( const int currentIndex )
{
    const int newBufferSize = mUI->comboBox_BufferSize->itemData( currentIndex ).toInt();

    // Get current audio settings
    AudioDeviceManager::AudioDeviceSetup config;
    mDeviceManager.getAudioDeviceSetup( config );
    String error;

    // Update audio settings
    if ( newBufferSize != config.bufferSize )
    {
        config.bufferSize = newBufferSize;
        error = mDeviceManager.setAudioDeviceSetup( config, true );
    }

    if ( error.isNotEmpty() )
    {
        showWarningBox( tr("Error when trying to open audio device!"), error.toRawUTF8() );
    }
}



void AudioSetupDialog::enableMidiInput( QListWidgetItem* listItem )
{
    QByteArray charArray = listItem->text().toLocal8Bit();
    const String midiInputName( charArray.data() );

    if ( listItem->checkState() == Qt::Checked )
    {
        mDeviceManager.setMidiInputEnabled( midiInputName, true );
    }
    else
    {
        mDeviceManager.setMidiInputEnabled( midiInputName, false );
    }
}



void AudioSetupDialog::playTestSound()
{
    mDeviceManager.playTestSound();
}



void AudioSetupDialog::accept()
{
    // Tear down MIDI input test synth
    mAudioSourcePlayer.setSource( NULL );
    mDeviceManager.removeMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
    mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );

    QDialog::accept();
}



void AudioSetupDialog::reject()
{
    // Tear down MIDI input test synth
    mAudioSourcePlayer.setSource( NULL );
    mDeviceManager.removeMidiInputCallback( String::empty, &(mSynthAudioSource->midiCollector) );
    mDeviceManager.removeAudioCallback( &mAudioSourcePlayer );

    QDialog::reject();
}
