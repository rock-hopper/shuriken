/*
  ==============================================================================

  This file is part of the dRowAudio JUCE module
  Copyright 2004-13 by dRowAudio.

  ------------------------------------------------------------------------------

  dRowAudio is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
  SOFTWARE.

  ==============================================================================
*/

#ifndef _DROWAUDIOHEADER_H_
#define _DROWAUDIOHEADER_H_

/**
  @mainpage dRowAudio - A JUCE module for high level audio application development.

  @details

  dRowAudio is a 3rd party JUCE module designed for rapid audio application
  development. It contains classes for audio processing and gui elements.
  Additionally there are several wrappers around 3rd party libraries including
  cURL, FFTReal and SoundTouch. dRowAudio is written in the strict JUCE style,
  closely following the style guide set out at [JUCE Coding Standards][1].

  dRowAudio is hosted on Github at [https://github.com/drowaudio/drowaudio][2]

  The online documentation is at [http://drowaudio.co.uk/docs/][3]

  A wiki with feature overview and demo screenshots can be found at
  [http://www.rawmaterialsoftware.com/wiki/index.php/DRowAudio][4]

  ## Platforms

  All platforms supported by JUCE are also supported by dRowAudio. Currently these
  platforms include:

  - **Windows**: Applications and VST/RTAS/NPAPI/ActiveX plugins can be built 
  using MS Visual Studio. The results are all fully compatible with Windows
  XP, Vista or Windows 7.

  - **Mac OS X**: Applications and VST/AudioUnit/RTAS/NPAPI plugins with Xcode.

  - **GNU/Linux**: Applications and plugins can be built for any kernel 2.6 or
  later.

  - **iOS**: Native iPhone and iPad apps.

  - **Android**: Supported.

  ## Prerequisites

  This documentation assumes that the reader has a working knowledge of JUCE.

  ## External Modules

  In order to use the cURL classes you will need to link to the cURL library.
  This is included as part of Mac OSX, for Windows there pre-built 32-bit binaries
  or you can download the library yourself for the most recent version.
  Instructions for linkage are detailed on the [dRowAudio wiki][4].

  Although some aspects of dRowAudio rely on other 3rd party modules such as
  [SoundTouch][5] and [FFTReal][6], these are included as part of the module so
  no external linking is required. Their use should be transparent to the user.

  ## Integration

  dRowAudio requires recent versions of JUCE. It won't work with versions 2.28 or
  earlier. To use the library it is necessary to first download JUCE to a
  location where your development environment can find it. Or, you can use your
  existing installation of JUCE.

  To use the module simply include it, or a symbolic link to it, in your
  juce/modules folder. Simply them run the Introjucer as normal and tick the
  dRowAudio module. Config flags are provided to disable some functionality if
  not required.

  ## License

  Copyright (C) 2013 by David Rowland ([e-mail][0])

  dRowAudio is provided under the terms of The MIT License (MIT):
 
  Permission is hereby granted, free of charge, to any person obtaining a copy of
  this software and associated documentation files (the "Software"), to deal in the
  Software without restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
  Software, and to permit persons to whom the Software is furnished to do so,
  subject to the following conditions:
 
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
 
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
  FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
  COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 
  Some portions of the software including but not limited to [SoundTouch][5] and
  [FFTReal][6] are included with in the repository but released under separate
  licences. Please see the individual source files for details.
 
  [0]: mailto:dave@drowaudio.co.uk "David Rowland (Email)"
  [1]: http://www.rawmaterialsoftware.com/wiki/index.php/Coding_Standards
  [2]: https://github.com/drowaudio/drowaudio
  [3]: http://drowaudio.co.uk/docs/
  [4]: http://www.rawmaterialsoftware.com/wiki/index.php/DRowAudio
  [5]: http://www.surina.net/soundtouch/index.html
  [6]: http://ldesoras.free.fr/prod.html
  [7]: http://www.gnu.org/licenses/gpl-2.0.html
  [8]: http://www.opensource.org/licenses/mit-license.html "The MIT License"
 
  @author David Rowland (<a href="mailto:dave@drowaudio.co.uk">email</a>)
  @version 1.0
  @date 2008
  @copyright Copyright (C) 2013 by David Rowland.
  @copyright Provided under the [The MIT License][5]
*/

//=============================================================================
/** Config: DROWAUDIO_USE_FFTREAL
    Enables the FFTReal library. By default this is enabled except on the Mac
    where the Accelerate framework is preferred. However, if you do explicity 
    enable this setting fftreal can be used for testing purposes.
 */
#ifndef DROWAUDIO_USE_FFTREAL
    #if (! JUCE_MAC)
        #define DROWAUDIO_USE_FFTREAL 0
    #endif
#endif

/** Config: DROWAUDIO_USE_SOUNDTOUCH
    Enables the SoundTouch library and the associated SoundTouch classes for
    independant pitch and tempo scaling. By default this is enabled.
 */
#ifndef DROWAUDIO_USE_SOUNDTOUCH
    #define DROWAUDIO_USE_SOUNDTOUCH 1
#endif

/** Config: DROWAUDIO_USE_CURL
    Enables the cURL library and the associated network classes. By default
    this is enabled.
 */
#ifndef DROWAUDIO_USE_CURL
    #define DROWAUDIO_USE_CURL 0
#endif

//=============================================================================
#include <modules/juce_audio_basics/juce_audio_basics.h>
#include <modules/juce_audio_devices/juce_audio_devices.h>
#include <modules/juce_core/juce_core.h>
#include <modules/juce_data_structures/juce_data_structures.h>
#include <modules/juce_events/juce_events.h>

#if JUCE_MAC || JUCE_IOS
    #define Point CarbonDummyPointName
    #define Component CarbonDummyCompName
    #include <Accelerate/Accelerate.h>
    #undef Point
    #undef Component
#endif

#undef min
#undef max

#ifdef __clang__
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wunused-function"
#endif

//=============================================================================
// fftReal needs to be outside of the drow namespace
#if DROWAUDIO_USE_FFTREAL
    #include "audio/fft/fftreal/FFTReal.h"
#endif

// cURL needs to be outside of the drow namespace
//#if DROWAUDIO_USE_CURL
// #if JUCE_WINDOWS
//  #include "network/curl/include/curl/curl.h"
// #else
//  #include <curl/curl.h>
// #endif
//#endif

//=============================================================================
namespace drow {
using namespace juce;
using juce::int64;
using juce::uint32;
using juce::int32;
using juce::MemoryBlock;
using juce::UnitTest;
//using juce::Rectangle;

// Audio
#ifndef __DROWAUDIO_SOUNDTOUCHPROCESSOR_H__
    #include "audio/dRowAudio_SoundTouchProcessor.h"
#endif

#ifndef __DROWAUDIO_SOUNDTOUCHAUDIOSOURCE_H__
    #include "audio/dRowAudio_SoundTouchAudioSource.h"
#endif

#ifndef __DROWAUDIO_FILTERINGAUDIOSOURCE_H__
    #include "audio/dRowAudio_FilteringAudioSource.h"
#endif

#ifndef __DROWAUDIO_REVERSIBLEAUDIOSOURCE_H__
    #include "audio/dRowAudio_ReversibleAudioSource.h"
#endif

#ifndef __DROWAUDIO_LOOPINGAUDIOSOURCE_H__
    #include "audio/dRowAudio_LoopingAudioSource.h"
#endif

#ifndef __DROWAUDIO_PITCH_H__
    #include "audio/dRowAudio_Pitch.h"
#endif

#ifndef __DROWAUDIO_PITCHDETECTOR_H__
    #include "audio/dRowAudio_PitchDetector.h"
#endif
    
#ifndef __DROWAUDIO_AUDIOUTILITY_H__
    #include "audio/dRowAudio_AudioUtility.h"
#endif

#ifndef __DROWAUDIO_FIFOBUFFER_H__
    #include "audio/dRowAudio_FifoBuffer.h"
#endif

#ifndef __DROWAUDIO_BUFFER_H__
    #include "audio/dRowAudio_Buffer.h"
#endif

#ifndef __DROWAUDIO_ENVELOPEFOLLOWER_H__
    #include "audio/dRowAudio_EnvelopeFollower.h"
#endif

#ifndef __DROWAUDIO_SAMPLERATECONVERTER_H__
    #include "audio/dRowAudio_SampleRateConverter.h"
#endif

#ifndef __DROWAUDIO_BIQUADFILTER_H__
    #include "audio/filters/dRowAudio_BiquadFilter.h"
#endif

#ifndef __DROWAUDIO_ONEPOLEFILTER_H__
    #include "audio/filters/dRowAudio_OnePoleFilter.h"
#endif

// maths
#ifndef __DROWAUDIO_MATHSUTILITIES_H__
    #include "maths/dRowAudio_MathsUtilities.h"
#endif

#ifndef __DROWAUDIO_CUMULATIVEMOVINGAVERAGE_H__
    #include "maths/dRowAudio_CumulativeMovingAverage.h"
#endif 

#ifndef __DROWAUDIO_BEZIERCURVE_H__
    #include "maths/dRowAudio_BezierCurve.h"
#endif

// streams
#ifndef __DROWAUDIO_MEMORYINPUTSOURCE_H__
    #include "streams/dRowAudio_MemoryInputSource.h"
#endif

// Utility
#ifndef __DROWAUDIO_CONSTANTS_H__
    #include "utility/dRowAudio_Constants.h"
#endif

#ifndef __DROWAUDIO_DEBUGOBJECT_H__
    #include "utility/dRowAudio_DebugObject.h"
#endif

#ifndef __DROWAUDIO_UTILITY_H__
    #include "utility/dRowAudio_Utility.h"
#endif

#ifndef __DROWAUDIO_XMLHELPERS_H__
    #include "utility/dRowAudio_XmlHelpers.h"
#endif

}

#ifdef __clang__
 #pragma clang diagnostic pop
#endif

#endif //_DROWAUDIOHEADER_H_
