
CFLAGS=-Ofast -msse2 -ftree-vectorize -funroll-loops -pthread
LDLIBS= -lm -lsndfile -lasound -lrtlsdr -pthread

# BEWARE on non SSE2 processor, undefined USE_SSE2 in acarsdec.h

acarsdec:	main.o acars.o msk.o soundfile.o alsa.o rtl.o
	$(CC) main.o acars.o msk.o soundfile.o alsa.o rtl.o -o $@ $(LDLIBS)

main.o:	main.c acarsdec.h
acars.o:	acars.c acarsdec.h syndrom.h
msk.o:	msk.c acarsdec.h
viterbi.o:	viterbi.c acarsdec.h
rtl.o:	rtl.c acarsdec.h
soundfile.o:	soundfile.c acarsdec.h
alsa.o:	alsa.c acarsdec.h

clean:
	@rm *.o acarsdec
