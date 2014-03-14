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
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include "acarsdec.h"

#define SYN 0x16
#define SOH 0x01
#define STX 0x02
#define ETX 0x83
#define ETB 0x97
#define DLE 0x7f

static const unsigned char  numbits[256]={
0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};
#include "syndrom.h"

const unsigned short crc_ccitt_table[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
	0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
	0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
	0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
	0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
	0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
	0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
	0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
	0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
	0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
	0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
	0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
	0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
	0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
	0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
	0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
	0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
	0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
	0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
	0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
	0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
	0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
	0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
	0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
	0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
	0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
	0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
	0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
	0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
	0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
	0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
	0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

typedef struct {
	unsigned char mode;
	unsigned char addr[8];
	unsigned char ack;
	unsigned char label[3];
	unsigned char bid;
	unsigned char no[5];
	unsigned char fid[7];
	unsigned char bs,be;
	unsigned char txt[240];
	int err,lvl;
} acarsmsg_t;

static pthread_mutex_t blkmtx=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t blkwcd= PTHREAD_COND_INITIALIZER;
static msgblk_t *blkq_s=NULL;
static msgblk_t *blkq_e=NULL;

#define update_crc(crc,c) crc= (crc>> 8)^crc_ccitt_table[(crc^(c))&0xff];

static void printtime(time_t t)
{
	struct tm *tmp;

	if(t==0) return;

	tmp = gmtime(&t);

	printf ("%02d/%02d/%04d %02d:%02d:%02d",
	     tmp->tm_mday, tmp->tm_mon + 1, tmp->tm_year + 1900,
	     tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

static void cleantxt(char *txt) 
{
char *ptr;

 for(ptr=txt;*ptr;ptr++) 
	if((*ptr<' ' || *ptr>126) && *ptr !='\n') *ptr=' ';

}

static void printmsg(acarsmsg_t *msg,int chn,time_t t)
{
	printf("\n[#%1d (%3d/%1d) ",chn+1,msg->lvl,msg->err);
	printtime(t);
	printf(" ----------------------------------------------------------\n");
	printf("Aircraft reg: %s ", msg->addr);
	if(msg->bs!=0x03) printf("Flight id: %s", msg->fid);
	printf("\n");
	printf("Mode: %c ", msg->mode);
	printf("Msg. label: %s\n", msg->label);
	printf("Block id: %x ", (int) msg->bid);
	printf("Ack: %x\n", msg->ack);
	printf("Msg. no: %s\n", msg->no);
	if(msg->bs==0x03)
		printf("No Text\n");
	else {
		cleantxt(msg->txt);
		printf("Message :\n%s\n", msg->txt);
	}
	if(msg->be==0x17) printf("Block End\n");
	fflush(stdout);
}

static void printoneline(acarsmsg_t *msg,int chn,time_t t)
{
	char txt[34];
	char *pstr;

	strncpy(txt,msg->txt,34);txt[33]=0;
	cleantxt(txt);
	strtok_r(txt,"\n",&pstr);

	printf("#%1d %7s %6s %2s %4s ", chn+1,msg->addr,msg->fid,msg->label,msg->no);
	printtime(t);
	printf(" %s\n", txt);
	fflush(stdout);
}


static void fixerr(msgblk_t *blk,int sft,unsigned char msk)
{
 if(sft<=2) {
	if(verbose) fprintf(stderr,"#%d error in CRC\n",blk->chn+1);
	return;
 }
 blk->txt[blk->len-1-(sft-2)]^=msk;
}

static void * blk_thread(void *arg)
{
 do {
	msgblk_t *blk;
	acarsmsg_t  msg;
	int i,k,pn;
	unsigned short crc;

	pthread_mutex_lock(&blkmtx);
	while(blkq_e==NULL) pthread_cond_wait(&blkwcd,&blkmtx);

	blk=blkq_e;
	blkq_e=blk->prev;
	if(blkq_e==NULL) blkq_s=NULL;
	pthread_mutex_unlock(&blkmtx);

	if(blk==NULL) return;

	if(blk->len<13) {
		if(verbose) fprintf(stderr,"#%d too short\n",blk->chn+1);
		free(blk);
		continue;
	}

	/* force STX/ETX */
	blk->txt[12]&=(ETX|STX);
	blk->txt[12]|=(ETX&STX);

	msg.err=0;
	/* parity check */
	pn=0;
	for(i=0;i<blk->len;i++) {
		if((numbits[blk->txt[i]]&1)==0) {
		pn++;
		}
	}
	if(pn>1) {
		if(verbose) fprintf(stderr,"#%d too much parity error : %d\n",blk->chn+1,pn);
		free(blk);
		continue;
	}

	/* crc check */
	crc=0;
	for(i=0;i<blk->len;i++) {
		update_crc(crc,blk->txt[i]);

	}
	update_crc(crc,blk->crc[0]);
	update_crc(crc,blk->crc[1]);

	/* try to fix errors */
	if(crc!=0) {
		int i,k,fx;
		if(verbose) fprintf(stderr,"#%d CRC error, try to recover 1 error\n",blk->chn+1);
		fx=0;
		for(i=0;i<242*8;i++) 
 			if(oneerr[i]==crc) {
				fixerr(blk,i/8,1<<(i%8));
				fx=1;
				msg.err=1;
				break;
			}

		if(fx==0 && pn==0 && blk->len<142) {
			int i,k,l;
			unsigned char  u,v;
			unsigned short n=0;
			if(verbose) fprintf(stderr,"#%d CRC error, try to recover 2 close errors\n",blk->chn+1);
			for(k=1,v=2;k<8;k++,v<<=1) {
		 	  for(l=0,u=1;l<k;l++,u<<=1) {
				if(twoerr[n]==crc) {
					fixerr(blk,0,u|v);
					fx=1;
					msg.err=2;
					break;
				}
				n++;
				for(i=1;i<142;i++) {
					if(twoerr[n]==crc) {
						fixerr(blk,i,u|v);
						fx=1;
						msg.err=2;
						break;
					}
					n++;
				}
				if(i<142) break;
			  }
			  if(l<k) break;
			}
		}

		if(fx==0) {
			if(verbose) fprintf(stderr,"#%d not able to fix it\n",blk->chn+1);
			free(blk);
			continue;
		} else {
			if(verbose) fprintf(stderr,"#%d fix it\n",blk->chn+1);
		}
	}

	/* redo parity checking and remove parity */
	pn=0;
	for(i=0;i<blk->len;i++) {
		if((numbits[blk->txt[i]]&1)==0) {
		pn++;
		}

		blk->txt[i]&=0x7f;
	}
	if(pn) {
		if(verbose) fprintf(stderr,"#%d parity error %d\n",blk->chn+1,pn);
		free(blk);
		continue;
	}

	/* fill msg struct */
	msg.txt[0]= '\0';
	msg.fid[0] = '\0';

	msg.lvl = blk->lvl;

	k = 0;
	msg.mode = blk->txt[k];
	k++;

	for (i = 0; i < 7; i++, k++) {
		msg.addr[i] = blk->txt[k];
	}
	msg.addr[7] = '\0';

	/* ACK/NAK */
	msg.ack = blk->txt[k];
	k++;

	msg.label[0] = blk->txt[k];
	k++;
	msg.label[1] = blk->txt[k];
	if(msg.label[1]==0x7f) msg.label[1]='d';
	k++;
	msg.label[2] = '\0';

	msg.bid = blk->txt[k];
	k++;
	/* STX/ETX */
	msg.bs=blk->txt[k];
	k++;

	msg.no[0] = '\0';
	msg.fid[0] = '\0';
	if(msg.mode <= 0x5d) {
	    /* donwlink */
	    if(k<blk->len) {

		for (i = 0; i < 4 && k<blk->len-1; i++, k++) {
			msg.no[i] = blk->txt[k];
		}
		msg.no[i] = '\0';

		msg.be=blk->txt[blk->len-1];
		for (i = 0; i < 6 && k<blk->len-1; i++, k++) {
			msg.fid[i] = blk->txt[k];
		}
		msg.fid[i] = '\0';

	    }
	} else {
	   /* uplink */
	  if(airflt) {
		free(blk);
		continue;
	  }
	}

	blk->txt[blk->len-1]='\0';
	strncpy(msg.txt, &(blk->txt[k]),240);msg.txt[239]=0;

	if(outtype==0)
		printoneline(&msg,blk->chn,blk->t);
	else
		printmsg(&msg,blk->chn,blk->t);

	free(blk);

 } while(1);
}

int initAcars(channel_t *ch)
{

 pthread_t th;

 ch->outbits=0;
 ch->nbits=8;
 ch->Acarsstate=WSYN;

 ch->blk=malloc(sizeof(msgblk_t));
 ch->blk->chn=ch->chn;

 pthread_create(&th,NULL,blk_thread,NULL);
return 0;
}



void decodeAcars(channel_t *ch)
{
	unsigned char r=ch->outbits;

	switch (ch->Acarsstate) {
		
		case WSYN:
			if (r==SYN) {
				ch->Acarsstate = SYN2;
				ch->nbits=8;
				return;
			}
			if (r==(unsigned char)~SYN) {
				ch->MskS^=2;
				ch->Acarsstate = SYN2;
				ch->nbits=8;
				return;
			}
			ch->nbits=1;
			return;

		case SYN2:
			if (r==SYN) {
				ch->Acarsstate = SOH1;
				ch->nbits=8;
				return ;
			}
			if (r==(unsigned char)~SYN) {
				ch->MskS^=2;
				ch->nbits=8;
				return;
			}
			ch->Acarsstate = WSYN;
			ch->nbits=1;
			return;

		case SOH1:
			if (r==SOH) {
				if(inpmode!=2) ch->blk->t = time(NULL);
				else ch->blk->t=0;
				ch->Acarsstate = TXT;
				ch->blk->len=0;
				ch->nbits=8;
				return ;
			}
			ch->Acarsstate = WSYN;
			ch->nbits=1;
			return;

		case TXT:
			ch->blk->txt[ch->blk->len] = r;
			ch->blk->len++;
			if (r == ETX || r == ETB) {
				ch->Acarsstate = CRC1;
				ch->nbits=8;
				return ;
			} 
			if (ch->blk->len>20 && r==DLE) {
				if(verbose) fprintf(stderr,"#%d miss txt end\n",ch->chn+1);
				ch->blk->len-=3;
				ch->blk->crc[0]=ch->blk->txt[ch->blk->len];
				ch->blk->crc[1]=ch->blk->txt[ch->blk->len+1];
				ch->Acarsstate = CRC2;
				goto putmsg_lbl;
			}
			if (ch->blk->len > 240) {
				if(verbose) fprintf(stderr,"#%d too long\n",ch->chn+1);
				ch->Acarsstate = WSYN;
				ch->nbits=1;
				return;
			} 
			ch->nbits=8;
			return ;

		case CRC1:
			ch->blk->crc[0]=r;
			ch->Acarsstate = CRC2;
			ch->nbits=8;
			return ;
		case CRC2:
			ch->blk->crc[1]=r;
			ch->blk->lvl=20*log10(ch->Mskdc)-81.0;
putmsg_lbl:
			pthread_mutex_lock(&blkmtx);
			ch->blk->prev=NULL;
			if(blkq_s) blkq_s->prev=ch->blk;
			blkq_s=ch->blk;
			if(blkq_e==NULL) blkq_e=blkq_s;
			pthread_cond_signal(&blkwcd);
			pthread_mutex_unlock(&blkmtx);

			ch->blk=malloc(sizeof(msgblk_t));
			ch->blk->chn=ch->chn;

			ch->Acarsstate = END;
			ch->nbits=8;
			return ;
		case END:
			ch->Acarsstate = WSYN;
			ch->nbits=8;
			return ;
	}
}
