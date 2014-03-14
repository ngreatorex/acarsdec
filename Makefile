
CFLAGS=-Ofast -ftree-vectorize -funroll-loops -pthread
LDLIBS= -lm -lsndfile -lasound -lrtlsdr -pthread

# BEWARE on non SSE2 processor, undefined USE_SSE2 in acarsdec.h

acarsdec:	main.o acars.o udp.o msk.o soundfile.o alsa.o rtl.o
	$(CC) main.o acars.o udp.o msk.o soundfile.o alsa.o rtl.o -o $@ $(LDLIBS)

main.o:		main.c udp.h acarsdec.h
acars.o:	acars.c acarsdec.h syndrom.h acars.h
msk.o:		msk.c acarsdec.h
viterbi.o:	viterbi.c acarsdec.h
rtl.o:		rtl.c acarsdec.h
soundfile.o:	soundfile.c acarsdec.h
alsa.o:		alsa.c acarsdec.h
udp.o: 		udp.c udp.h acars.h


clean:
	@rm *.o acarsdec
