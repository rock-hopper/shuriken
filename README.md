Shuriken Beat Slicer
====================

Shuriken is an open source beat slicer which harnesses the power of aubio's onset detection algorithms and Rubber Band's time stretching capabilities. A simple Qt4 interface makes it easy to slice up drum loops, assign individual drum hits to MIDI keys, and change the tempo of loops in real-time. JUCE takes care of handling audio and MIDI behind the scenes.

Shuriken can either be built using Qt Creator or the supplied build script. To use the build script, simply open a terminal in Shuriken's root directory and enter:

    ./build (or optionally ./build debug)

To clean the project:

    ./build clean

You must have qmake-qt4, and the aubio (>=0.4.0) and rubberband (>=1.3) dev files installed on your system.

