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
#include <net/if.h>
#include <sys/ioctl.h>

#define SIZEOF_IFREQ(ifr) \
        ((ifr).ifr_addr.sa_len > sizeof(struct sockaddr) ? \
         (sizeof(struct ifreq) - sizeof(struct sockaddr) + \
          (ifr).ifr_addr.sa_len) : sizeof(struct ifreq))

/* notes
might have to know which interface packets came in on, to know which to use in responding? in msghdr? (ick!)
does inaddr_any send out on all?
need to externally define HAVE_SA_LEN, impossibly unportable
iface config -> iface req(?) -> ifconfig-like data, one per address on every interface
IF_UP|IF_RUNNING doesn't tell operative status, it just seems to go away (the one w/ the addr)
any way to tell iface events? if sock is bound to it, get read errors? (seems not besides polling)
stupid way to get ifconf data, have to loop growing if buf not big enough
*/

/*
known answers:
	when sending out a question and you know (some) answers, include them,
	if > 1 packet, set TC and send rest w/o question in more packets
	process incoming questions w/ known answers in multi-packet if TC set too
	and don't query if someone else asked the same thing
	and don't answer if someone else did the same answer (? when would that happen?)

responses
	don't send any questions back
	delay randomly time 20-120ms,
	or not delay if you know your the only one
	if source isn't 5353, reply directly to source AND broadcast to link

startup
	send out all unique as questions to ensure uniqueness, +250ms x 2
	include our answers in Authority
	if while probing another probe is seen, byte compare their rdata w/ ours, highest wins
	after OK, gratuitous response, in answer section all of our rr's, multiple packets if needed
	if unique, high bit in rrclass is 1 ("cache flush")
	up to ten, time doubling
	if above seen, go to conflict resolution

*/

int main(int argc, char *argv[])
{
    int s, flag = 1;
    struct sockaddr_in in;
    struct ip_mreq mc;
    struct ifconf ifc;
    struct ifreq *ifr;
    char ttl = 255;
    char buf[8192];
    int i;
    char *ptr;

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(5353);
    in.sin_addr.s_addr = 0;

    s = socket(AF_INET,SOCK_DGRAM,0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
    bind(s,(struct sockaddr*)&in,sizeof(in));

    // our interfaces
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = buf;
    if(ioctl(s, SIOCGIFCONF, &ifc) != 0)
    {
        printf("SIOCGIFCONF died: %s\n",strerror(errno));
        return 1;
    }
    // if ifc_len == sizeof(buf), loop w/ larger buf!

    for(i=0;i <  ifc.ifc_len;)
    {
    
/* #ifdef HAVE_SA_LEN
               i += (sizeof ifp -> ifr_name) +
                  ((ifp->ifr_addr.sa_len < sizeof(ifp->ifr_addr)) ?
                   sizeof(ifp->ifr_addr) : ifp->ifr_addr.sa_len);
 #else
                        i += sizeof *ifp;
#endif
*/
        ifr = buf+i;
        i +=  (ifr->ifr_addr.sa_len > sizeof(struct sockaddr)) ? (sizeof(struct ifreq) - sizeof(struct sockaddr) + ifr->ifr_addr.sa_len) : sizeof(struct ifreq);
        ioctl (s, SIOCGIFFLAGS, ifr);
        printf("%d/%d got iface: %s state[%d/%d] and skipping %d of %d ifc %X buf %X\n",ifr->ifr_addr.sa_len,sizeof(struct sockaddr),ifr->ifr_name,ifr->ifr_flags&IFF_UP,ifr->ifr_flags&IFF_RUNNING,i,ifc.ifc_len,ifr,buf);
//        fflush(stdout);

    }
        printf("%d of %d ifc %X buf %X\n",i,ifc.ifc_len,ifr,buf+ifc.ifc_len);
    return 0;

    // multicast
    mc.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mc.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc)); 
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)); // ttl could be int?

    printf("starting...\n");
    dnsdaemon(s);
    return 0;
}

