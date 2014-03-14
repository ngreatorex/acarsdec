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

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <getopt.h>
#include "acarsdec.h"

channel_t channel[MAXNBCHANNELS];
unsigned int  nbch;
unsigned long wrkmask;

pthread_mutex_t datamtx=PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t datawcd= PTHREAD_COND_INITIALIZER;

int inpmode=0;
int verbose=0;
int outtype=1;
int airflt=0;
int gain=-1000;
int ppm=0;

static void usage(void)
{
	fprintf(stderr, "Acarsdec 2.1 	Copyright (c) 2014 Thierry Leconte \n\n");
	fprintf(stderr, "Usage: acarsdec  [-v] [-o lv] -a alsapcmdevice  | -f sndfile | -r rtldevicenumber  f1 [f2] [f3] [f4] \n\n");
	fprintf(stderr, " -v :\t\t\tverbose\n");
	fprintf(stderr, " -o lv :\t\toutput format : 0 one line by msg., 1 full (default) \n");
	fprintf(stderr, " -A :\t\t\tdon't display uplink messages (ie : only aircraft messages)\n");
	fprintf(stderr, " -g gain :\t\tset rtl preamp gain in tenth of db. By default use AGC (ie -g 90 for +9db)\n");
	fprintf(stderr, " -p ppm :\t\tset rtl ppm frequency correction\n");
	fprintf(stderr, " -f sndfile :\t\tdecode from sound file (ie: a .wav file)\n");
	fprintf(stderr, " -a alsapcmdevice :\tdecode from soundcard input alsapcmdevice (ie: hw:0,0)\n");
	fprintf(stderr, " -r rtldevicenumber f1 [f2] [f3] [f4] :\t\tdecode from rtl dongle number rtldevicenumber receiving at VHF frequencies f1 and optionaly f2 to f4 in Mhz (ie : -r 0 131.525 131.725 131.825 )\n\n");
	fprintf(stderr, "For any input source , up to 4 channels  could be simultanously decoded\n");
	exit(1);
}

static void * demod_thread(void * arg)
{ 
   channel_t *ch=(channel_t*)arg;
   unsigned long msk;


  msk=1<<(ch->chn);

  do {
	pthread_mutex_lock(&datamtx);
	while((wrkmask&msk)==0) pthread_cond_wait(&datawcd, &datamtx);
	pthread_mutex_unlock(&datamtx);

	if(inpmode==3) demodAM(ch);
	demodMsk(ch);

	pthread_mutex_lock(&datamtx);
	wrkmask&=~msk;
	pthread_mutex_unlock(&datamtx);
	pthread_cond_broadcast(&datawcd);

   } while(1);
}

static void sighandler(int signum)
{
	fprintf(stderr, "Signal caught, exiting!\n");
	exit(1);
}		

int main(int argc, char **argv)
{
	int c;
	int ch;
	int res,n;
	unsigned long msk;
	struct sigaction sigact;

	while ((c = getopt(argc, argv, "vafro:g:Ap:")) != EOF) {


		switch (c) {
		case 'v':
			verbose=1;
			break;
		case 'o':
			outtype=atoi(argv[optind]);
			break;
		case 'a':
#ifdef WITH_ALSA
			res=initAlsa(argv,optind);
			inpmode=1;
#else
			fprintf(stderr,"Sorry this  binary is not compiled with ALSA support\n");
#endif
			break;
		case 'f':
#ifdef WITH_SNDFILE
			res=initSoundfile(argv,optind);
			inpmode=2;
#else
			fprintf(stderr,"Sorry this  binary is not compiled with soundfile support\n");
#endif
			break;
		case 'r':
#ifdef WITH_RTL
			res=initRtl(argv,optind);
			inpmode=3;
#else
			fprintf(stderr,"Sorry this  binary is not compiled with RTL support\n");
#endif
			break;

		case 'A':
			airflt=1;
			break;
		case 'g':
			gain=atoi(optarg);
			break;
		case 'p':
			ppm=atoi(optarg);
			break;
		default:
			usage();
		}
	}

	if(inpmode==0) {
		fprintf(stderr,"Need at least one of -a|-f|-r options\n");
		usage();
	}

	if(res) {
		fprintf(stderr,"Unable to init input\n");
		exit(res);
	}
	
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);

	wrkmask=0;
	msk=0;
	for(n=0;n<nbch;n++) {
		  		
		  if(channel[n].Infs) { 
			pthread_t th;


			channel[n].chn=n;

			res=initMsk(&(channel[n]));
			if(res) break;
			res=initAcars(&(channel[n]));
			if(res) break;

			msk|=1<<n;
			pthread_create(&th,NULL,demod_thread,&(channel[n]));
		}
	}

	if(res) {
		fprintf(stderr,"Unable to init internal decoders\n");
		exit(res);
	}

	if(verbose) fprintf(stderr,"Decoding %d channels\n",nbch);

	/* main decoding infinite loop */
	do {

		switch(inpmode) {
#ifdef WITH_ALSA
		case 1:
			res=getAlsaSample();
			break;
#endif
#ifdef WITH_SNDFILE
		case 2:
			res=getSoundfileSample();
			break;
#endif
#ifdef WITH_RTL
		case 3:
			res=getRtlSample();
			break;
#endif
		default: res=-1;
		}

		if(res) {
			if(verbose) fprintf(stderr,"exiting\n");
			exit(0);
		}

		pthread_mutex_lock(&datamtx);
		wrkmask=msk;
		pthread_mutex_unlock(&datamtx);
		pthread_cond_broadcast(&datawcd);

	} while (1);
}
