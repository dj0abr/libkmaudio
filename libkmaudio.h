#pragma once

#ifdef WIN32
// ignore senseless warnings invented by M$ to confuse developers
#pragma warning( disable : 4091 )
#pragma warning( disable : 4003 )
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <wchar.h>

#ifdef WIN32
#include "Winsock2.h"
#include "io.h"
#include <Windows.h>
#include <iostream>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <Shlobj.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "portaudio.h"
#include "pa_win_wasapi.h"

#pragma comment(lib, "portaudio_x86.lib")
#pragma comment(lib, "libliquid.lib")
#else
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/file.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pwd.h>
#include <math.h>
#include "soundio.h"
#endif

#include "liquid.h"




#define MAXDEVICES	100
#define MAXDEVNAMELENGTH	80

typedef struct _DEVLIST_ {
	int index = 0;				// index to this list
	int devnum = -1;			// port audio device number
	char name[MAXDEVNAMELENGTH] = { 0 };// real name
	int in_out = 0;				// 0=capture device, 1=playback device, 2=both
	int supports_44100 = 0;		// 1 if supported
	int supports_48000 = 0;		// 1 if supported
	int requested_samprate = 0; // sample rate requested by caller
	int real_samprate = 0;		// real sample rate of the device
	int working = 0;			// 0=not running, 1=initialized and working
#ifdef WIN32 // Windows using portaudio
	PaStreamParameters inputParameters;
	PaStreamParameters outputParameters;
	PaStream	*capStream = NULL;
	PaStream	*pbStream = NULL;
#else // Linux using libsoundio
	struct SoundIoDevice* io_pb_device = NULL;
	struct SoundIoDevice* io_cap_device = NULL;
	struct SoundIoInStream* instream = NULL;
	struct SoundIoOutStream* outstream = NULL;
	int    stereo_mono = 2;    // 1=mono, 2=stereo
	char id[1000] = { 0 };
#endif
} DEVLIST;

/*
* initialize the audio library, create required processes
* call only once when your application starts
* returns: 0 = OK, -1 = error
*/
int kmaudio_init();

/*
* closes and frees all resources
* call when application stops
*/
void kmaudio_close();

/*
* read a list of all available audio devices into devlist
* returns: 0=OK, -1 if error
*/
int kmaudio_getDeviceList();

/*
* starts a capturing stream from devname with samprate
* returns: id of the capture stream or -1 = error
*/
int kmaudio_startCapture(char* devname, int samprate);

/*
* starts a playback stream to devname with samprate
* returns: id of the playback stream or -1 = error
*/

int kmaudio_startPlayback(char* devname, int samprate);

/* 
* plays len samples from psamp to device id
* returns: 0=ok, -1=error
* id ... device id returned by kmaudio_startPlayback
* psamp ... float array of length len with the audio data (mono)
* len ... number of float values in psamp
*/
int kmaudio_playsamples(int id, float* psamp, int len);

/*
* reads len samples from device id into psamp
* returns: 0=ok, -1=error
* id ... device id returned by kmaudio_startCapture
* psamp ... float array of length len getting the audio data (mono)
* len ... number of float values to write into psamp
* wait ... 1=wait for data, 0=return if not enough data available (in this case psamp will return 0,0,0...)
*/
int kmaudio_readsamples(int id, float* psamp, int len, int wait);

/*
* reads the names of detected sound devices
* *len...length of the returned string
* returns: pointer to device string
* Format of the device string:
* Name of playback devices, followed by ~
* separator ^
* Name of capture devices, followed by ~
* these names are used for calls to kmaudio_startCapture and kmaudio_startPlayback
* to select the device
*/
uint8_t* io_getAudioDevicelist(int* len);

// functions for internal use
int searchDevice(char* devname, int io);
void measure_speed_bps(int len);
void sleep_ms(int ms);
void io_write_fifo(int pipenum, float sample);
void io_fifo_clear(int pipenum);
void init_pipes();
int io_read_fifo_num(int pipenum, float* data, int num);
int io_read_fifo(int pipenum, float* data);
int getRealSamprate(int idx);
int io_fifo_elems_avail(int pipenum);
void sleep_ms(int ms);
void io_buildUdpAudioList();
void resampler_create(int devidx);
float* resample(int id, float* psamp, int len, int* pnewlen);

extern DEVLIST devlist[MAXDEVICES];
extern int devanz;
extern int keeprunning;

#ifndef WIN32 // Linux
int kmaudio_init_linux();
void kmaudio_close_linux();
char* getDevID(char* devname, int io, int* pidx);

extern struct SoundIo* soundio;
#endif // ndef WIN32

