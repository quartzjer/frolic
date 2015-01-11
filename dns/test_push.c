
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "1035.h"
#include "ichat.h"

int main(int argc, char *argv[])
{
    int s, flag = 1, rdlen, qsize;
    struct sockaddr_in to;
    struct message *m;
    unsigned char *q, *rdata, *rd;
    xht h;

    bzero(&to, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(5353);
    to.sin_addr.s_addr = inet_addr("224.0.0.251");

    s = socket(AF_INET,SOCK_DGRAM,0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));

    printf("pushing...\n");
    m = message_wire();
    m->id = 2;
    m->head1.rd = 1;

    message_an(m, "_ichat._tcp.local.", 12, 120);
    rdata = rd = malloc(256);
    _host(m,&rd,"test._ichat._tcp.local.");
    rdlen = rd - rdata;
    message_rdata(m,rdlen,rdata);
    free(rdata);

    message_an(m, "test._ichat._tcp.local.", 16, 120);
    h = xht_new(11);
    xht_set(h,"1st","Foo");
    xht_set(h,"last","Bar");
    xht_set(h,"status","avail");
    rdata = ichat2txt(h, &rdlen);
    xht_free(h);
    message_rdata(m,rdlen,rdata);

    q = message_packet(m,&qsize);
    hexdump(q,qsize);
    if(s < 1 || sendto(s,q,qsize,0,(struct sockaddr*)&to,sizeof(struct sockaddr_in)) != qsize)
    {
        free(m);
        printf("dead socket (%s) or bad response\n",strerror(errno));
        return;
    }
    free(m);
    return 0;
}
