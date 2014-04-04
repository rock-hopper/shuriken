Shuriken Beat Slicer
====================

Shuriken is an open-source beat slicer which leverages the power of aubio's beat and onset detection algorithms. A simple Qt4 interface makes it easy to slice up drum loops and assign samples to MIDI keys, while JUCE takes care of handling audio and MIDI behind the scenes.

Shuriken can either be built using Qt Creator or the supplied build script. To use the build script, simply open a terminal in Shuriken's root directory and enter:

    ./build

or to clean the project:

    ./build clean

You must have qmake-qt4 and aubio installed on your system. fftw3 is recommended but not required.

This project is still in the early stages of development. Come back later!
