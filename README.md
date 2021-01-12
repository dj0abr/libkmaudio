this is work in progress. Basically it works but needs some improvements.

# libkmaudio: very simple to use audio library for Linux and Windows

I was looking for an audio soultion working on both, Linux and Windows.
It was impossible to find a reliable open source solution.
If you have no problem using a commercial closed source library, then I recommend "bass"
which worked nicely for me, but I need open source to fulfil the requirements of GPL V3.

# My solution:

portaudio: works fine under Windows. Does not work under Linux because it does not support
pulseaudio and takes all these cryptical Alsa device names.

libsoundio: works well under Linux and supports pulseaudio, uses excellent device names.
But has problems under Windows, distortions and stuttering, so not usable on Windows.

So I wrote this library which uses portaudio for windows and libsoundio for Linux putting
all in a very simple to use library.

# Restrictions

this library was written to be used with SDR receivers/transmitters or for signal analysis purposes. Therefore it supports 1 channel and two sampe rates (44100 and 48000).
However, the 1-channel limit is just the fifo. So if someone needs stereo, adjust the routines which write/read the fifo.

# highlights

a capture or playback stream can be opened for every soundcard available in the computer.
Each stream gets its own fifo. So we do not have to work with streams, we just put our samples into a fifo or read samples from a fifo. Except the soundcard name we do not need
to know anything about sound, all is done by this library.

# Windows

compatible with Visual Studio. The Windows-Shared Libraries must be copied to the path
of the executable file.

# Linux

run "make" to compile the library. The shared libraries (supplied for PC 64bit or ARM 64 bit) must be copied to the folder where Linux searches for shared libraries on your computer. To find this folder you can use this command:
find /usr -name libc.so | head -1

# Documentation

There are only very few functions needed. They are described in libkmaudio.h

# Example

An example how to init the sound system, record from microphone and play back to loadspeaker can be found in libkmaudio.cpp
