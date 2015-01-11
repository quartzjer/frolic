#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <strings.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "1035.h"

void dnsdaemon(int s)
{
    int asize, ssize = sizeof(struct sockaddr_in);
    unsigned char a[MAX_PACKET_LEN];
    struct message m;
    struct sockaddr_in sender;

    bzero(a,MAX_PACKET_LEN);
    while(s > 1 && (asize = recvfrom(s,a,MAX_PACKET_LEN,0,(struct sockaddr*)&sender,&ssize)) > 1)
    {
        bzero(&m,sizeof(struct message));
        message_parse(&m,a);
	mprint(&m,asize,inet_ntoa(sender.sin_addr),ntohs(sender.sin_port));
//	hexdump(a,asize);
        bzero(a,MAX_PACKET_LEN);
        ssize = sizeof(struct sockaddr_in);
    }
    printf("dead socket (%s) or bad response\n",strerror(errno));
}


int main(int argc, char *argv[])
{
    int s, flag = 1, ittl = 255;
    struct sockaddr_in in;
    struct ip_mreq mc;
    char ttl = 255;

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(5353);
    in.sin_addr.s_addr = 0;

    s = socket(AF_INET,SOCK_DGRAM,0);
#ifdef SO_REUSEPORT
    setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char*)&flag, sizeof(flag));
#endif
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
    bind(s,(struct sockaddr*)&in,sizeof(in));

//    setsockopt(s, IPPROTO_IP, IP_PKTINFO, &flag, sizeof(flag))
    mc.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mc.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc)); 
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ittl, sizeof(ittl));

//    printf("seeding...\n");
//    ipush(s);
    printf("watching...\n");
    dnsdaemon(s);
    return 0;
}

