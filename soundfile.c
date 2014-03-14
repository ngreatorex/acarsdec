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
#include <string.h>
#include <pthread.h>
#include <sndfile.h>
#include "acarsdec.h"

#define MAXNBFRAMES 1024
static SNDFILE *insnd;


int initSoundfile(char **argv,int optind)
{
	SF_INFO infsnd;
	int n;

	infsnd.format = 0;
	insnd = sf_open(argv[optind], SFM_READ, &infsnd);
	if (insnd == NULL) {
		fprintf(stderr, "could not open %s\n", argv[optind]);
		return (1);
	}
	nbch=infsnd.channels;
	if (nbch>MAXNBCHANNELS) {
		fprintf(stderr,"Too much input channels : %d\n",nbch);
		return (1);
	}
	if (infsnd.samplerate<9600) {
		fprintf(stderr,"Too low sample rate : %d\n",infsnd.samplerate);
		return (1);
	}
	for(n=0;n<nbch;n++) {
		channel[n].chn=n;
		channel[n].Infs=infsnd.samplerate;
		channel[n].InBuff=malloc(MAXNBFRAMES*sizeof(sample_t));
	}
	for(;n<MAXNBCHANNELS;n++) channel[n].Infs=0;
	
	return (0);
}


int getSoundfileSample(void)
{
	int nbi,n,i;
	sample_t sndbuff[MAXNBFRAMES*MAXNBCHANNELS];

	nbi = sf_read_float(insnd, sndbuff,MAXNBFRAMES*nbch);

	if (nbi == 0) {
		return -1;
	}

	pthread_mutex_lock(&datamtx);
	while(wrkmask) pthread_cond_wait(&datawcd, &datamtx);
	pthread_mutex_unlock(&datamtx);

	for(n=0;n<nbch;n++)
		if(channel[n].Infs) {
		  int len=nbi/nbch;
		  for(i=0;i<len;i++) channel[n].InBuff[i]=sndbuff[n+i*nbch];
		  channel[n].lenIn=len;
		}

	return 0;
}


