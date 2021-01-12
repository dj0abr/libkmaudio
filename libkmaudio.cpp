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
* Usage Example: see main() in this file
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
* libkmaudio.cpp ... main() for test purposes only
* usually this library is linked to another program, in this case
* comment-out #define LIBTEST
*
*/

#include "libkmaudio.h"


/*
* main()
* for testing purposes only
* for library generation comment out: LIBTEST
*/

#define LIBTEST

#ifdef LIBTEST
int main()
{
    // initialize sound system
    // must be called once after program start
    // may be called during program run, this will reset the sound system
    // and should usually not be required
	kmaudio_init();
	
    // read list of devices
    // must be called only once after program start
    // if called during runtime and the user has connected/disconnected
    // devices the device list changes which may crash the system
	kmaudio_getDeviceList();
    
    // start capture and/or playback streams
    // Parameter: the device name and the sample rate (44100 or 48000 are supported)
    // these function return the fifo-index, which is used to access the data in the
    // coresponding fifo

	//int capidx = kmaudio_startCapture((char *)"Line 2 (Virtual Audio Cable)", 44100);
    //int capidx = kmaudio_startCapture((char*)"Mikrofon (USB Advanced Audio Device)", 48000);
    //int capidx = kmaudio_startCapture((char*)"USB Audio CODEC: - (hw:2,0)", 48000);
    //int capidx = kmaudio_startCapture((char*)"Mikrofon (1080P Webcam)", 44100);
    //int pbidx = kmaudio_startPlayback((char*)"Lautsprecher (USB Advanced Audio Device)", 48000);
    //int pbidx = kmaudio_startPlayback((char*)"USB Audio CODEC: - (hw:2,0)", 48000);

    //int capidx = kmaudio_startCapture((char*)"PCM2902 Audio Codec Analog Stereo", 48000);
    //int pbidx = kmaudio_startPlayback((char*)"PCM2902 Audio Codec Analog Stereo", 48000);
    int capidx = kmaudio_startCapture((char*)"USB Advanced Audio Device Analog Stereo", 48000);
    int pbidx = kmaudio_startPlayback((char*)"USB Advanced Audio Device Analog Stereo", 48000);

    float f[1100]; // 1.1 x need rate to have reserve for resampler
    while (1)
    {
        // make a loop: record from Mic and play to Speaker

        // read samples from the capture fifo
        int anz = kmaudio_readsamples(capidx, f, 1000, 0);
        if (anz > 0)
        {
            // if samples are available, send them to playback fifo
            //printf("write %d samples from %d to %d\n", anz, capidx, pbidx);
            kmaudio_playsamples(pbidx, f, anz);
        }
        else
        {
            // if no samples are available make a short break
            // this is important because it prevents excessive CPU usage
            sleep_ms(10);
        }
    }

    // free resources. This will never happen in this example
    // but should be done in the final program
	kmaudio_close();
	return 0;
}
#endif // LIBTEST

// diagonstic routines for development

#define MAXSPDARR   10
int spdarr[MAXSPDARR];
int spdarrbps[MAXSPDARR];

#ifdef _LINUX_
uint64_t getms()
{
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    uint64_t at = tv.tv_sec * 1000000 + tv.tv_usec;
    at = at / 1000;
    return at;
}
#endif

#ifdef WIN32
uint64_t getms()
{
    // get time in 100ns resolution
    FILETIME ft_now;
    GetSystemTimeAsFileTime(&ft_now);

    // convert to full 64 bit time
    uint64_t ll_now = (uint64_t)ft_now.dwLowDateTime + ((uint64_t)(ft_now.dwHighDateTime) << 32LL);

    // convert to Milliseconds
    ll_now /= (10 * 1000);      // still needs 64 bit integer

    return ll_now;
}
#else
uint64_t getms()
{
    struct timeval  tv;
 	gettimeofday(&tv, NULL);
    uint64_t at = tv.tv_sec * 1000000 + tv.tv_usec;
    at = at / 1000;
    return at;
}
#endif

void measure_speed_bps(int len)
{
    static uint64_t lasttim = 0;
    static int elems = 0;

    uint64_t tim = getms();
    int timespan = (int)(tim - lasttim);
    if (timespan < 0)
    {
        lasttim = tim;
        return;
    }

    elems += len;
    if (timespan < 1000) return;

    double dspd = elems;
    dspd = dspd * 1e3 / timespan;
    int speed = (int)dspd;

    // here we have number of elements after 1s
    printf(" ======================= %d bit/s\n", speed);

    elems = 0;
    lasttim = tim;
}

void sleep_ms(int ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}



