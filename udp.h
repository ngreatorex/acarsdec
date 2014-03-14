#include <stdio.h>

#include "acars.h"

extern int verbose;

void init_udp(char **argv, int optind);
static void setup_networking(char *host, char *port);
void send_udp(acarsmsg_t *msg, int chn, time_t t);

