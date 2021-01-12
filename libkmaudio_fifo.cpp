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
* libkmaudio_fifo.cpp ... thread safe FIFOs, used to interface the
* audio callback routines to the other program
*
*/

#include "libkmaudio.h"

#define NUM_OF_PIPES 20

#ifdef WIN32
CRITICAL_SECTION crit_sec[NUM_OF_PIPES];
#define LOCK(pn)	EnterCriticalSection(&(crit_sec[pn]))
void UNLOCK(int pn)
{
    if (&(crit_sec[pn]) != NULL)
        LeaveCriticalSection(&(crit_sec[pn]));
}
#else
pthread_mutex_t crit_sec[NUM_OF_PIPES];
#define LOCK(pn)	pthread_mutex_lock(&(crit_sec[pn]))
#define UNLOCK(pn)	pthread_mutex_unlock(&(crit_sec[pn]))
#endif

#define AUDIO_FIFOFLEN 48000

int io_wridx[NUM_OF_PIPES];
int io_rdidx[NUM_OF_PIPES];
float io_buffer[NUM_OF_PIPES][AUDIO_FIFOFLEN];

void init_pipes()
{
    // init pipes only once
    static int f = 1;
    if (f)
    {
        f = 0;
        for (int i = 0; i < NUM_OF_PIPES; i++)
        {
#ifdef WIN32
            if (&(crit_sec[i]) != NULL) DeleteCriticalSection(&(crit_sec[i]));
            InitializeCriticalSection(&(crit_sec[i]));
#else
            if (&(crit_sec[i]) != NULL) pthread_mutex_destroy(&(crit_sec[i]));
            pthread_mutex_init(&(crit_sec[i]), NULL);
#endif
        }
    }

    for (int i = 0; i < NUM_OF_PIPES; i++)
        io_fifo_clear(i);
}

// write one sample into the fifo
// ignore data if the fifo is full
void io_write_fifo(int pipenum, float sample)
{
    LOCK(pipenum);
    if (((io_wridx[pipenum] + 1) % AUDIO_FIFOFLEN) == io_rdidx[pipenum])
    {
        //printf("cap fifo full\n");
        UNLOCK(pipenum);
        return;
    }
    
    io_buffer[pipenum][io_wridx[pipenum]] = sample;
    if (++io_wridx[pipenum] >= AUDIO_FIFOFLEN) io_wridx[pipenum] = 0;
    UNLOCK(pipenum);
}

int io_read_fifo(int pipenum, float* data)
{
    LOCK(pipenum);

    if (io_rdidx[pipenum] == io_wridx[pipenum])
    {
        // Fifo empty, no data available
        UNLOCK(pipenum);
        return 0;
    }

    *data = io_buffer[pipenum][io_rdidx[pipenum]];
    if (++io_rdidx[pipenum] >= AUDIO_FIFOFLEN) io_rdidx[pipenum] = 0;
    UNLOCK(pipenum);

    return 1;
}

// read num elements
// if num elems not avail, return all what fifo has stored
int io_read_fifo_num(int pipenum, float* data, int num)
{
    LOCK(pipenum);

    int elemInFifo = (io_wridx[pipenum] + AUDIO_FIFOFLEN - io_rdidx[pipenum]) % AUDIO_FIFOFLEN;

    if (num > elemInFifo)
    {
        // Fifo not enough data available
        //printf("only %d elements available\n", elemInFifo);
        UNLOCK(pipenum);
        return 0;
    }

    //if (num > elemInFifo) num = elemInFifo;

    for (int i = 0; i < num; i++)
    {
        *data++ = io_buffer[pipenum][io_rdidx[pipenum]];
        if (++io_rdidx[pipenum] >= AUDIO_FIFOFLEN) io_rdidx[pipenum] = 0;
    }
    UNLOCK(pipenum);

    return num;
}

void io_fifo_clear(int pipenum)
{
    io_wridx[pipenum] = io_rdidx[pipenum] = 0;
}

int io_fifo_freespace(int pipenum)
{
    int freebuf = 0;

    LOCK(pipenum);

    int elemInFifo = (io_wridx[pipenum] + AUDIO_FIFOFLEN - io_rdidx[pipenum]) % AUDIO_FIFOFLEN;
    freebuf = AUDIO_FIFOFLEN - elemInFifo;

    UNLOCK(pipenum);
    return freebuf;
}

int io_fifo_elems_avail(int pipenum)
{
    int elems = 0;

    LOCK(pipenum);
    elems = (io_wridx[pipenum] + AUDIO_FIFOFLEN - io_rdidx[pipenum]) % AUDIO_FIFOFLEN;
    UNLOCK(pipenum);

    return elems;
}
