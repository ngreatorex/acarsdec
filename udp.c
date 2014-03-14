/*
 *  Copyright (c) 2014 Neil Greatorex (https://github.com/ngreatorex)
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include "acars.h"
#include "udp.h"

#define UDP_BUF_SIZE 1024

static int socket_fd;
static int configured = 0;

void init_udp(char **argv, int optind)
{
	if (configured) {
		fprintf(stderr, "UDP already configured (-u specified twice?)\n");
		exit(1);
	}

	if(argv[optind]==NULL) {
                fprintf(stderr, "Need hostname and port after -u\n");
                exit(1);
        }

	if(argv[optind+1]==NULL) {
		fprintf(stderr, "Need port after -u\n");
		exit(1);
	}

	setup_networking(argv[optind], argv[optind+1]);
	configured = 1;
}

static void setup_networking(char *host, char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s;

	if (verbose) fprintf(stderr, "Sending UDP packets to '%s' port %s\n", host, port);
	
	/* Obtain address(es) matching host/port (ripped from man page for getaddrinfo!) */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	s = getaddrinfo(host, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		socket_fd = socket(rp->ai_family, rp->ai_socktype,
		             rp->ai_protocol);
		if (socket_fd == -1)
			continue;

		if (connect(socket_fd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;                  /* Success */

		close(socket_fd);
	}

	if (rp == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);           /* No longer needed */
}

void send_udp(acarsmsg_t *msg, int chn, time_t t)
{
	char buf[UDP_BUF_SIZE];
	int size;

	if (!configured) {
		fprintf(stderr, "UDP not configured - missing -u option?\n");
		exit(1);
	}

	size = snprintf(buf, UDP_BUF_SIZE-1, "AC%c %7s %c %2s %c %4s %6s %s",
		msg->mode,
		msg->addr,
		msg->ack == 21 ? '!' : msg->ack, /* Convert NAK to '!' */
		msg->label,
		msg->bid,
		msg->no,
		msg->fid,
		msg->txt);

	buf[UDP_BUF_SIZE-1] = 0;

	send(socket_fd, buf, size, 0);
	printf("#%d: UDP packet sent -> \"%s\"\n", chn, buf);
}
