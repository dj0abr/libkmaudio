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
* libkmaudio_getDevices_linux.cpp
* like libkmaudio_getDevices.cpp, but uses libsoundio under Linux
* to get the device list. Portaudio does not work under Linux because
* it does not support pulseaudio. Therefore the linux functions
* use libsoundio
*
*/

#ifndef WIN32 // Linux

#include "libkmaudio.h"

int scan_devices();

int kmaudio_getDeviceList()
{
    if (soundio == NULL)
    {
        printf("kmaudio_getDeviceList: soundio not initialized\n");
        return -1;
    }

    soundio_flush_events(soundio);      // to get actual data

    if (scan_devices() == -1)    // read devices
    {
        printf("cannot read audio devices\n");
        return -1;
    }

    io_buildUdpAudioList();

	return 0;
}

static void get_channel_layout(const struct SoundIoChannelLayout* layout)
{
    if (layout->name)
    {
        if (strstr(layout->name, "ereo"))
            devlist[devanz].stereo_mono = 2;
        if (strstr(layout->name, "ono"))
            devlist[devanz].stereo_mono = 1;
    }
}

int print_device(struct SoundIoDevice* device)
{
    if (!device->probe_error)
    {
        // ignore if exists
        for (int i = 0; i < devanz; i++)
            if (!strcmp(device->id, devlist[i].id)) return 0;

        if (strstr(device->name, "onitor")) return 0;

        strncpy(devlist[devanz].id, device->id, sizeof(devlist[0].id) - 1);
        devlist[devanz].id[sizeof(devlist[0].id) - 1] = 0;
        strncpy(devlist[devanz].name, device->name, sizeof(devlist[0].name) - 1);
        devlist[devanz].name[sizeof(devlist[0].name) - 1] = 0;

        for (int i = 0; i < device->layout_count; i++)
            get_channel_layout(&device->layouts[i]);

        int min = 999999, max = 0;
        for (int i = 0; i < device->sample_rate_count; i++)
        {
            struct SoundIoSampleRateRange* range = &device->sample_rates[i];
            if (range->min < min)
                min = range->min;

            if (range->max > max)
                max = range->max;
        }
        if (min <= 44100) devlist[devanz].supports_44100 = 1;
        if (max >= 48000) devlist[devanz].supports_48000 = 1;
        if (devlist[devanz].supports_44100 == 0 && devlist[devanz].supports_48000 == 0) return 0;
        return 1;
    }
    return 0;
}

int scan_devices()
{
    devanz = 0;
    for (int i = 0; i < soundio_input_device_count(soundio); i++)
    {
        struct SoundIoDevice* device = soundio_get_input_device(soundio, i);
        if (print_device(device) == 1)
        {
            //printf("%d %d ====CAP:\nid:<%s>\nname:<%s>\n", i,devanz,device->id, device->name);
            devlist[devanz].in_out = 0;
            devlist[devanz].index = devanz;
            devanz++;
        }
        soundio_device_unref(device);
    }

    for (int i = 0; i < soundio_output_device_count(soundio); i++)
    {
        struct SoundIoDevice* device = soundio_get_output_device(soundio, i);
        if (print_device(device) == 1)
        {
            //printf("====PB :\nid:<%s>\nname:<%s>\n", device->id, device->name);
            devlist[devanz].in_out = 1;
            devlist[devanz].index = devanz;
            devanz++;
        }
        soundio_device_unref(device);
    }
    return 0;
}

#endif // ndef WIN32