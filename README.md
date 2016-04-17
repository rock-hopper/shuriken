Shuriken Beat Slicer
====================

Shuriken is an open source beat slicer which harnesses the power of aubio's onset detection algorithms and Rubber Band's time stretching capabilities. A simple Qt interface makes it easy to slice up drum loops, assign individual drum hits to MIDI keys, and change the tempo of loops in real-time. The JUCE library takes care of handling audio and MIDI behind the scenes.

Main features:

- onset and beat-of-the-bar detection (via aubio)
- calculate BPM
- sync BPM of a drum loop to JACK transport
- zero-crossing detection
- slice and un-slice waveform
- each audio slice automatically assigned to its own MIDI key
- move, delete, and copy & paste audio slices
- apply gain, gain ramp, normalise, and reverse
- amplitude envelope generator with controls for attack, release, and one-shot
- set the number of JACK outputs and route any audio slice to any output
- time stretching in offline or real-time mode (via rubberband)
- time stretch individual audio slices and quantise your beats
- NSM session management support
- LADISH L1 session management support
- ALSA and JACK MIDI
- import a wide range of audio file formats (via libsndfile and the built-in sndlib library)
- export audio slices in WAV, AIFF, AU, FLAC, or Ogg format, with options for encoding and sample rate
- export Hydrogen Drumkit, SFZ, Akai .pgm, or MIDI file

Shuriken can either be built using Qt Creator or the supplied build script. To use the build script, simply open a terminal in Shuriken's root directory and enter:

    ./build

If working with Qt Creator, use the build script to compile the static sndlib library first:

    ./build --sndlib

The build script also provides some other options:

    ./build --qt4 (configure with qmake-qt4 and compile against Qt4 libraries)
    ./build --debug
    ./build --clean
    ./build --help

You must have either qmake or qmake-qt4 installed as well as the aubio (>=0.4.1) and rubberband (>=1.3) dev files.
___

As noted above, Shuriken requires the latest incarnation of the aubio library which I've packaged for Ubuntu Precise and Trusty, and AVLinux 6:

https://launchpad.net/~rock-hopper/+archive/ubuntu/audiotools

https://dl.dropboxusercontent.com/u/23511236/libaubio4_0.4.1-1avl6_i386.deb
https://dl.dropboxusercontent.com/u/23511236/libaubio4-dev_0.4.1-1avl6_i386.deb
https://dl.dropboxusercontent.com/u/23511236/libaubio4-doc_0.4.1-1avl6_all.deb
https://dl.dropboxusercontent.com/u/23511236/libaubio4_0.4.1-1avl6.source.tar.gz

I'm aware that some software depends on the old aubio library so I've made sure there are no dependency issues with libaubio4 and the old libaubio2 package: the two sit side-by-side and applications which require the old aubio library are unaffected by the presence of the new.

I've also packaged the aubio4 dev files, which are needed to build Shuriken.  It isn't possible to have both the old libaubio-dev package and the new libaubio4-dev package installed at the same time, but it's easy enough to uninstall one and (re)install the other.
