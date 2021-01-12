CXXFLAGS = -Wall -O3 -std=c++0x -Wno-write-strings -Wno-narrowing
LDFLAGS = -lpthread -lrt -lsndfile -lasound -lm -lfftw3 -lfftw3_threads -lliquid -lsoundio
OBJ = libkmaudio.o  libkmaudio_fifo.o  libkmaudio_getDevices.o  libkmaudio_getDevices_Linux.o libkmaudio_init.o libkmaudio_init_linux.o libkmaudio_interface.o libkmaudio_capture_linux.o libkmaudio_playback_linux.o libkmaudio_resampler.o

default: $(OBJ)
	g++ $(CXXFLAGS) -o libkmaudio $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o
	rm -rf ./Debug/*
	rm -rf ./Release/*
	rmdir Debug
	rmdir Release
	
