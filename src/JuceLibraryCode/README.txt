# The JUCE Library

JUCE (Jules' Utility Class Extensions) is an all-encompassing 
C++ framework for developing cross-platform software.

It contains pretty much everything you're likely to need to create
most applications, and is particularly well-suited for building 
highly-customised GUIs, and for handling graphics and sound.

Most JUCE modules are shared under the GNU Public Licence 
(GPLv2, v3, and the AGPLv3). This means that the code can 
be freely copied and distributed, and costs nothing to use 
in other GPL applications. One module (the juce_core module) 
is permissively licensed under the ISC.

For more information, visit the website:
http://www.juce.com

---------------------------------------------------------------------------------------

The following files contain modifications to the original JUCE code:

  modules/juce_audio_devices/native/juce_linux_JackAudio.cpp
  modules/juce_audio_devices/native/juce_linux_Midi.cpp
  modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.h
  modules/juce_audio_devices/audio_io/juce_AudioDeviceManager.cpp
  modules/juce_core/system/juce_SystemStats.cpp

The original unmodified files are also present and have ".orig" appended to the filename.



The following file is not part of the JUCE library but does contian modified JUCE code:

  modules/juce_audio_devices/native/linux_midi.h
  
  
  
The following file is not part of the JUCE library:

  modules/juce_audio_devices/native/relay_libjack.cpp


  
To the extent possible under law, Andrew M Taylor <a.m.taylor303@gmail.com> has waived all 
copyright and related or neighboring rights to the modifications to the original JUCE code.
<https://creativecommons.org/publicdomain/zero/1.0/>