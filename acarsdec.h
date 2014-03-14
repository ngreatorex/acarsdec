/*
 *  Copyright (c) 2014 Thierry Leconte (f4dwv)
 *
 *   
 *   This code is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#define WITH_RTL 1
#define WITH_SNDFILE 1
#define WITH_ALSA 1
#define USE_SSE2


#define MAXNBCHANNELS 4


typedef float sample_t;

typedef struct mskblk_s {
	struct mskblk_s *prev;
	int chn;
	time_t t;
	unsigned char txt[240];
	int len;
	unsigned char crc[2];
	int lvl;
} msgblk_t;

typedef struct {
	int chn;

#ifdef WITH_RTL
	float AMFreq;
	float AMPhi;
#endif
	int Infs;
	sample_t *InBuff;
	int lenIn;

	float MskPhi;
	float MskFreq,MskDf;
	float Mska,MskA,MskB;
	float Mskdc,Mskdcf;

	float MskClk,Mskdphi;
	unsigned int   MskS;

	sample_t  *I,*Q;
	float *h;
	int flen,idx;

	unsigned char outbits;
	int	nbits;

	enum { WSYN, SYN2, SOH1, TXT, CRC1,CRC2, END } Acarsstate;
	msgblk_t *blk;

} channel_t;

extern channel_t channel[MAXNBCHANNELS];
extern unsigned int  nbch;
extern unsigned long wrkmask;

extern int inpmode;
extern int verbose;
extern int outtype;
extern int airflt;
extern int gain;
extern int ppm;

extern pthread_mutex_t datamtx;
extern pthread_cond_t datawcd;

#ifdef WITH_ALSA
extern int initAlsa(char **argv,int optind);
extern int getAlsaSample(void);
#endif
#ifdef WITH_SNDFILE
extern int initSoundfile(char **argv,int optind);
extern int getSoundfileSample(void);
#endif
#ifdef WITH_RTL
extern int initRTL(char **argv,int optind);
extern int getRTLSample(void);
extern void demodAM(channel_t *);
#endif
extern int  initMsk(channel_t *);
extern void demodMsk(channel_t *);

extern int  initAcars(channel_t *);
extern void decodeAcars(channel_t *);

