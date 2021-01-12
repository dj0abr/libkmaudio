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
* libkmaudio_getDevices.cpp
* read audio device list via portaudio (which is only used for Windows)
* prepare a device name string which can be read by another program
* in order to present the devices to use user for selection
*
*/

#include "libkmaudio.h"

void io_buildUdpAudioList();

DEVLIST devlist[MAXDEVICES];
int devanz = 0;

#ifdef WIN32

static double standardSampleRates[] = {
8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 
22050.0, 24000.0, 32000.0, 44100.0, 48000.0, 
88200.0, 96000.0, 192000.0, -1 };

void PrintSupportedStandardSampleRates(
    const PaStreamParameters* inputParameters,
    const PaStreamParameters* outputParameters)
{
    static double standardSampleRates[] = {
        8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0,
        44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1 /* negative terminated  list */
    };
    int     i, printCount;
    PaError err;

    printCount = 0;
    for (i = 0; standardSampleRates[i] > 0; i++)
    {
        err = Pa_IsFormatSupported(inputParameters, outputParameters, standardSampleRates[i]);
        if (err == paFormatIsSupported)
        {
            if (printCount == 0)
            {
                printf("\npc0: %8.2f\n", standardSampleRates[i]);
                printCount = 1;
            }
            else if (printCount == 4)
            {
                printf("\npc4: %8.2f\n", standardSampleRates[i]);
                printCount = 1;
            }
            else
            {
                printf("\npce: %8.2f\n", standardSampleRates[i]);
                ++printCount;
            }
        }
    }
    printf("\n");
    
}

int kmaudio_getDeviceList()
{
    int numDevices = Pa_GetDeviceCount();
    if (numDevices < 0)
    {
        printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
        return -1;
    }

    printf("%d Devices found\n", numDevices);

    for (int i = 0; i < numDevices; i++)
    {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        const PaHostApiInfo *ai = Pa_GetHostApiInfo(deviceInfo->hostApi);
        
        // Windows: use WASAPI devices only
        if (strstr(ai->name, "WASAPI") != NULL)
        {
            
            devlist[devanz].devnum = i;
            snprintf(devlist[devanz].name, MAXDEVNAMELENGTH - 1, "%s", deviceInfo->name);
            printf("------%s-------\n", deviceInfo->name);

            devlist[devanz].inputParameters.device = i;
            devlist[devanz].inputParameters.channelCount = deviceInfo->maxInputChannels;
            devlist[devanz].inputParameters.sampleFormat = paFloat32;
            devlist[devanz].inputParameters.suggestedLatency = 0;
            devlist[devanz].inputParameters.hostApiSpecificStreamInfo = NULL;

            devlist[devanz].outputParameters.device = i;
            devlist[devanz].outputParameters.channelCount = deviceInfo->maxOutputChannels;
            devlist[devanz].outputParameters.sampleFormat = paFloat32;
            devlist[devanz].outputParameters.suggestedLatency = 0;
            devlist[devanz].outputParameters.hostApiSpecificStreamInfo = NULL;

            if (devlist[devanz].inputParameters.channelCount > 0 && devlist[devanz].outputParameters.channelCount > 0)
                devlist[devanz].in_out = 2;
            else if (devlist[devanz].inputParameters.channelCount > 0)
                devlist[devanz].in_out = 0;
            else if (devlist[devanz].outputParameters.channelCount > 0)
                devlist[devanz].in_out = 1;

            for (int j = 0; standardSampleRates[j] > 0; j++)
            {
                PaError err = 0;
                if (devlist[devanz].inputParameters.channelCount > 0 && devlist[devanz].outputParameters.channelCount > 0)
                    err = Pa_IsFormatSupported(&devlist[devanz].inputParameters, &devlist[devanz].outputParameters, standardSampleRates[j]);
                if (devlist[devanz].inputParameters.channelCount > 0)
                    err = Pa_IsFormatSupported(&devlist[devanz].inputParameters, NULL, standardSampleRates[j]);
                if (devlist[devanz].outputParameters.channelCount > 0)
                    err = Pa_IsFormatSupported(NULL, &devlist[devanz].outputParameters, standardSampleRates[j]);

                if (err == paFormatIsSupported)
                {
                    if (j == 8) devlist[devanz].supports_44100 = 1;
                    if (j == 9) devlist[devanz].supports_48000 = 1;
                }
            }
            devlist[devanz].index = devanz;
            devanz++;
        }
    }

    io_buildUdpAudioList();

    printf("Devices found:\n");
    for (int i = 0; i < devanz; i++)
    {
        printf("Portaudio ID: %d\n", devlist[i].devnum);
        printf("Name: %s\n", devlist[i].name);
        printf("Cap/PB: %d\n", devlist[i].in_out);
        printf("Channels: i:%d o:%d\n", devlist[i].inputParameters.channelCount, devlist[i].outputParameters.channelCount);
        printf("SR 44100: %d\n", devlist[i].supports_44100);
        printf("SR 48000: %d\n", devlist[i].supports_48000);
    }

    return 0;
}

#endif //WIN32

// find a device in devlist
// returns: list index or -1 if error
int searchDevice(char* devname, int io)
{
    for (int i = 0; i < devanz; i++)
    {
        printf("%d:<%s>\n", devlist[i].in_out, devlist[i].name);
        if (strcmp(devname, devlist[i].name) == 0 && (devlist[i].in_out == io || devlist[i].in_out == 2))
            return i;
    }
    return -1;
}

// choose physical, real sample rate for a device
// returns: 0=ok, -1=error: no sample rate supported
int getRealSamprate(int idx)
{
    if (devlist[idx].requested_samprate == 44100)
    {
        if (devlist[idx].supports_44100)		devlist[idx].real_samprate = 44100;
        else if (devlist[idx].supports_48000)	devlist[idx].real_samprate = 48000;
        else return -1;
    }

    else if (devlist[idx].requested_samprate == 48000)
    {
        if (devlist[idx].supports_48000)		devlist[idx].real_samprate = 48000;
        else if (devlist[idx].supports_44100)	devlist[idx].real_samprate = 44100;
        else return -1;
    }

    return 0;
}

// build string of audio device name, to be sent to application as response to Broadcast search
// starting with PB devices, sperarator ^, capture devices
// separator between devices: ~
// the first character is 0 or 1 and does not belong to the device name
// it shows if this device was sucessfully started and is currently running (="1")
#define MAXDEVSTRLEN (MAXDEVICES * (MAXDEVNAMELENGTH + 2) + 10)
uint8_t io_devstring[MAXDEVSTRLEN];

void io_buildUdpAudioList()
{
    memset(io_devstring, 0, sizeof(io_devstring));
    io_devstring[0] = ' ';     // placeholder for ID for this UDP message

    // playback devices
    for (int i = 0; i < devanz; i++)
    {
        if (strlen((char *)io_devstring) > MAXDEVSTRLEN)
        {
            printf("io_devstring too small:%d / %d. Serious error, abort program\n", MAXDEVSTRLEN, (int)strlen((char*)io_devstring));
            exit(0);
        }
        if (devlist[i].in_out == 1)
        {
            strcat((char*)io_devstring, devlist[i].working?"1":"0");
            strcat((char*)io_devstring, devlist[i].name);
            strcat((char*)io_devstring, "~");    // audio device separator
        }
    }

    strcat((char*)(io_devstring + 1), "^");    // PB, CAP separator

    // capture devices
    for (int i = 0; i < devanz; i++)
    {
        if (strlen((char*)io_devstring) > MAXDEVSTRLEN)
        {
            printf("io_devstring too small:%d / %d. Serious error, abort program\n", MAXDEVSTRLEN, (int)strlen((char*)io_devstring));
            exit(0);
        }
        if (devlist[i].in_out == 0)
        {
            strcat((char*)io_devstring, devlist[i].working ? "1" : "0");
            strcat((char*)io_devstring, devlist[i].name);
            strcat((char*)io_devstring, "~");    // audio device separator
        }
    }

    //printf("<%s>\n", (char *)io_devstring);

    io_devstring[0] = 3;   // ID for this UDP message
}

uint8_t* io_getAudioDevicelist(int* len)
{
    *len = strlen((char*)(io_devstring + 1)) + 1;
    return io_devstring;
}
