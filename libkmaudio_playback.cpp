/*
* Audio Library for Linux and Windows
* ===================================
* Author: DJ0ABR
*
* Author: Kurt Moraw, Ham radio: DJ0ABR, github: dj0abr
* License: GPL-3
*
* compilation:
* Windows ... Visual Studio
* Linux ... make
*
* Documentation see: libkmaudio.h
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* libkmaudio_playback.cpp ...
* starts a portaudio playback stream and a callback routine. Plays the
* audio samples coming via the fifo (Windows only)
*/

#include "libkmaudio.h"

#define FRAMES_PER_BUFFER 512

int playCallback(const void* inputBuffer,
	void* outputBuffer,
	unsigned long                   framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags           statusFlags,
	void* userData);

int kmaudio_startPlayback(char* devname, int samprate)
{
	int idx = searchDevice(devname, 1);
	if (idx == -1)
	{
		printf("Playback Device:<%s> not found\n", devname);
		return -1;
	}

	devlist[idx].working = 0;

	if (devlist[idx].pbStream != NULL)
	{
		Pa_CloseStream(devlist[idx].pbStream);
		devlist[idx].pbStream = NULL;
	}

	devlist[idx].requested_samprate = samprate;
	if (getRealSamprate(idx) == -1)
	{
		printf("Samplerate %d not supported by device:<%s>\n", samprate, devname);
		return -1;
	}

	if (devlist[idx].requested_samprate != devlist[idx].real_samprate)
		resampler_create(idx);

	PaError e = Pa_IsFormatSupported(&devlist[idx].outputParameters, NULL, (double)samprate);
	printf("Playback : err:%d device:%d PAdev:%d samprate: %f chans:%d\n", e, idx, devlist[idx].devnum, (double)samprate, devlist[idx].outputParameters.channelCount);

	devlist[idx].index = idx;

	int err = Pa_OpenStream(
		&devlist[idx].pbStream,
		NULL,
		&devlist[idx].outputParameters,
		(double)samprate,
		FRAMES_PER_BUFFER,
		paClipOff,
		playCallback,
		&(devlist[idx].index));

	if (err != paNoError)
	{
		printf("cannot open playback stream for device:<%s> %d\n", devname, err);
		return -1;
	}

	err = Pa_StartStream(devlist[idx].pbStream);
	if (err != paNoError)
	{
		printf("cannot start playback stream for device:<%s>\n", devname);
		return -1;
	}

	printf("Playback started sucessfully\n");
	devlist[idx].working = 1;
	return idx;
}

int playCallback(	const void* inputBuffer,
					void* outputBuffer,
					unsigned long                   framesPerBuffer,
					const PaStreamCallbackTimeInfo* timeInfo,
					PaStreamCallbackFlags           statusFlags,
					void* userData)
{
	float* rptr = (float*)outputBuffer;
	int devidx = *((int*)userData);
	int chans = devlist[devidx].outputParameters.channelCount;

	//measure_speed_bps(framesPerBuffer);

	float f[FRAMES_PER_BUFFER];
	memset(f, 0, sizeof(float) * FRAMES_PER_BUFFER);
	unsigned int num = io_read_fifo_num(devidx, f, framesPerBuffer);
	if (num < framesPerBuffer)
	{
		//printf("got %d from fifo, requested %d\n", num, framesPerBuffer);
	}

	for (unsigned int i = 0; i < framesPerBuffer; i++)
	{
		if (chans == 1) rptr[i] = f[i];
		else
		{
			rptr[i * 2] = f[i];
			rptr[i * 2 + 1] = f[i];
		}
	}

	// Prevent unused variable warnings
	(void)inputBuffer;
	(void)timeInfo;
	(void)statusFlags;

	if (keeprunning == 1)
		return paContinue;

	return paComplete;
}
