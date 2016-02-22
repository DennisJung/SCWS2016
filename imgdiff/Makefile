
CC=gcc
CFLAGS=-Wall -g #-O2

LIBS = -ljpeg -lm -lrt
LDFLAGS = ${LIBS}


all: seq opencl

.PHONY: all seq opencl clean


seq: imgdiff_seq

imgdiff_seq: imgdiff_seq.o libjpeg_wrapper.o imgdiff_main.o
	${CC} $^ -o $@ ${LDFLAGS}


opencl: imgdiff_opencl

imgdiff_opencl: imgdiff_opencl.o libjpeg_wrapper.o imgdiff_main.o
	${CC} $^ -o $@ ${LDFLAGS} -lOpenCL


clean:
	rm -f imgdiff imgdiff_main.o imgdiff_seq.o imgdiff_opencl.o libjpeg_wrapper.o imgdiff_seq imgdiff_opencl
