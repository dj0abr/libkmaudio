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
* libkmaudio_init.cpp ... initialize portaudio (Windows only)
*
*/

#include "libkmaudio.h"

void kmaudio_close();

int keeprunning = 1;    // to stop callbacks at program end

int kmaudio_init()
{
    kmaudio_close();
    sleep_ms(100);

	printf("libkmaudio_init\n");

    keeprunning = 1;
    init_pipes();   // init fifo

#ifdef WIN32
    int err = Pa_Initialize();
    if (err != paNoError)
    {
        printf("ERROR: Pa_Initialize returned 0x%x\n", err);
        return -1;
    }

    printf("PortAudio version: 0x%08X\n", Pa_GetVersion());
#else
    return kmaudio_init_linux();
#endif

	return 0;
}

void kmaudio_close()
{
	printf("libkmaudio_close\n");

#ifdef WIN32
    for (int i = 0; i < devanz; i++)
    {
        if (devlist[i].capStream != NULL)
        {
            Pa_CloseStream(devlist[i].capStream);
            devlist[i].capStream = NULL;
        }
        if (devlist[i].pbStream != NULL)
        {
            Pa_CloseStream(devlist[i].pbStream);
            devlist[i].pbStream = NULL;
        }
    }
    Pa_Terminate();

#else
    kmaudio_close_linux();
#endif
    keeprunning = 0;
}