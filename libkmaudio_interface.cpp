
#include "libkmaudio.h"

/*
* reads len samples from device id into psamp
* returns: number of values written to psamp , -1=error
* id ... device id returned by kmaudio_startCapture
* psamp ... float array of length len getting the audio data (mono)
* len ... number of float values to write into psamp
* wait ... 1=wait for data, 0=return if not enough data available (in this case psamp will return 0,0,0...)
* 
* if resampling is required the number of returned samples may differ from the number of requested samples
* it can be larger, so the buffer psamp should be larger than "len" by factor 1.1
*/

int kmaudio_readsamples(int id, float* psamp, int len, int wait)
{
	int to = 0;
	while (wait)
	{
		int e = io_fifo_elems_avail(id);
		if (e >= len) break;
		sleep_ms(10);
		if (++to >= 200)	// give up after 2s
			return -1;
		if (keeprunning == 0) return -1;
	}

	if (devlist[id].requested_samprate == devlist[id].real_samprate)
	{
		// data rate is ok, take samples as is
		return io_read_fifo_num(id, psamp, len);
	}

	// resampling is required
	int num = io_read_fifo_num(id, psamp, len);
	if (num == 0) return 0;
	
	int newlen = 0;
	float *f = resample(id, psamp, len, &newlen);
	for (int i = 0; i < newlen; i++)
		psamp[i] = f[i];

	return newlen;
}

/*
* plays len samples from psamp to device id
* returns: 0=ok, -1=error
* id ... device id returned by kmaudio_startPlayback
* psamp ... float array of length len with the audio data (mono)
* len ... number of float values in psamp
*/

int kmaudio_playsamples(int id, float* psamp, int len)
{
	// check if resampling is required
	if (devlist[id].requested_samprate == devlist[id].real_samprate)
	{
		// sampling rate is ok, just play samples
		for (int i = 0; i < len; i++)
			io_write_fifo(id, psamp[i]);
		return 0;
	}
	
	// resampling is required

	int newlen = 0;
	float *f = resample(id, psamp, len, &newlen);
	for (int i = 0; i < newlen; i++)
		io_write_fifo(id, f[i]);

	return 0;
}
