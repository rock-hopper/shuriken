/*
  ==============================================================================
  
  This file is part of the dRowAudio JUCE module
  Copyright 2004-12 by dRowAudio.
  
  ------------------------------------------------------------------------------
 
  dRowAudio can be redistributed and/or modified under the terms of the GNU General
  Public License (Version 2), as published by the Free Software Foundation.
  A copy of the license is included in the module distribution, or can be found
  online at www.gnu.org/licenses.
  
  dRowAudio is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
  
  ==============================================================================
*/


#ifdef __DROWAUDIO_JUCEHEADER__
    /*  When you add this cpp file to your project, you mustn't include it in a file where you've
        already included any other headers - just put it inside a file on its own, possibly with your config
        flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
        header files that the compiler may be using.
     */
#error "Incorrect use of DROWAUDIO cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

//#include "../juce_core/native/juce_BasicNativeHeaders.h"

#if JUCE_MAC || JUCE_IOS
    #import <Foundation/Foundation.h>
    #import <AudioToolbox/AudioToolbox.h>
#endif

#if JUCE_IOS
    #import <AVFoundation/AVFoundation.h>
    #import <MediaPlayer/MediaPlayer.h>
#endif

#include "dRowAudio.h"

#include "audio/soundtouch/SoundTouch_Source.cpp"

namespace drow {

// Audio
#include "audio/dRowAudio_SoundTouchProcessor.cpp"
#include "audio/dRowAudio_SoundTouchAudioSource.cpp"

#include "audio/dRowAudio_FilteringAudioSource.cpp"
#include "audio/dRowAudio_ReversibleAudioSource.cpp"
#include "audio/dRowAudio_LoopingAudioSource.cpp"

#include "audio/dRowAudio_PitchDetector.cpp"

#include "audio/dRowAudio_AudioUtilityUnitTests.cpp"

#include "audio/dRowAudio_Buffer.cpp"
#include "audio/dRowAudio_EnvelopeFollower.cpp"
#include "audio/dRowAudio_SampleRateConverter.cpp"

#include "audio/filters/dRowAudio_BiquadFilter.cpp"
#include "audio/filters/dRowAudio_OnePoleFilter.cpp"

// maths
#include "maths/dRowAudio_MathsUnitTests.cpp"

// streams
#include "streams/dRowAudio_MemoryInputSource.cpp"

}
