#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include "1035.h"
#include "mdnsd.h"
#include "ichat.h"

// if our name is valid, share, hack for now
int ok_share(mdnsda a, void *arg)
{
    mdnsd d = (mdnsd)arg;
    mdnsdr r;
    static once = 0;
    if(once++) return;
    printf("got our answer %d:%s\n",a->type,a->name);
    r = mdnsd_shared(d,"_ichat._tcp.local.",QTYPE_PTR,3600);
    mdnsd_set_host(d,r,"ipushtest._ichat._tcp.local.");    
}

// process an answer
int ans(mdnsda a, void *arg)
{
    printf("got answer %d:%s, arg %s\n",a->type,a->name,arg);
}

// conflict!
void con(char *name, int type, void *arg)
{
    printf("conflict detected %d:%s, arg %s\n",type,name,arg);
}

// create multicast 224.0.0.251:5353 socket
int msock()
{
    int s, flag = 1, ittl = 255;
    struct sockaddr_in in;
    struct ip_mreq mc;
    char ttl = 255;

    bzero(&in, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_port = htons(5353);
    in.sin_addr.s_addr = 0;

    if((s = socket(AF_INET,SOCK_DGRAM,0)) < 0) return 0;
#ifdef SO_REUSEPORT
    setsockopt(s, SOL_SOCKET, SO_REUSEPORT, (char*)&flag, sizeof(flag));
#endif
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
    if(bind(s,(struct sockaddr*)&in,sizeof(in))) { close(s); return 0; }

    mc.imr_multiaddr.s_addr = inet_addr("224.0.0.251");
    mc.imr_interface.s_addr = htonl(INADDR_ANY);
    setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mc, sizeof(mc)); 
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, &ittl, sizeof(ittl));

    flag =  fcntl(s, F_GETFL, 0);
    flag |= O_NONBLOCK;
    fcntl(s, F_SETFL, flag);

    return s;
}

int main(int argc, char *argv[])
{
    mdnsd d;
    mdnsdr r;
    struct message m;
    unsigned long int ip;
    unsigned short int port;
    struct timeval *tv;
    int bsize, ssize = sizeof(struct sockaddr_in);
    unsigned char buf[MAX_PACKET_LEN];
    struct sockaddr_in from, to;
    fd_set fds;
    int s, zero = 0;
    unsigned char *packet;
    int len = 0;
    xht h;

    d = mdnsd_new(1,1000);
    if((s = msock()) == 0) { printf("can't create socket: %s\n",strerror(errno)); return 1; }

//    mdnsd_query(d,"_http._tcp.local.",QTYPE_A,ans,"HTTP");
/*    r = mdnsd_shared(d,"_http._tcp.local.",QTYPE_PTR,120);
    mdnsd_set_host(d,r,"jer's test._http._tcp.local.");
    r = mdnsd_unique(d,"jer's test._http._tcp.local.",QTYPE_SRV,120,con,"SRV");
    mdnsd_set_srv(d,r,0,0,80,"home.local.");
    r = mdnsd_unique(d,"home.local.",QTYPE_A,120,con,"A");
    ip = inet_addr("199.120.70.133");
    mdnsd_set_raw(d,r,(unsigned char *)&ip,4);
    r = mdnsd_unique(d,"jer's test._http._tcp.local.",16,120,con,"TXT");
    mdnsd_set_raw(d,r,(unsigned char *)&zero,1);
*/
    mdnsd_query(d,"ipushtest._ichat._tcp.local.",QTYPE_SRV,ok_share,(void*)d);
    r = mdnsd_unique(d,"ipushtest._ichat._tcp.local.",QTYPE_SRV,3600,con,"SRV");
    mdnsd_set_srv(d,r,0,0,5299,"ipushtest.local.");
    r = mdnsd_unique(d,"ipushtest.local.",QTYPE_A,3600,con,"A");
    ip = inet_addr("207.177.38.222");
    mdnsd_set_raw(d,r,(unsigned char *)&ip,4);
    r = mdnsd_unique(d,"ipushtest._ichat._tcp.local.",16,3600,con,"TXT");
    h = xht_new(11);
    xht_set(h,"1st","Foo");
    xht_set(h,"last","Bar");
    xht_set(h,"status","avail");
    xht_set(h,"msg","Howdy Y'all");
    xht_set(h,"email","test@test.com");
    xht_set(h,"port.p2pj","5298");
    packet = ichat2txt(h, &len);
    xht_free(h);
    mdnsd_set_raw(d,r,packet,len);
    free(packet);

    while(1)
    {
        tv = mdnsd_sleep(d);
        printf("sleep %d.%d\n",tv->tv_sec,tv->tv_usec);
        FD_ZERO(&fds);
        FD_SET(s,&fds);
        select(s+1,&fds,0,0,tv);

        if(FD_ISSET(s,&fds))
        {
            while((bsize = recvfrom(s,buf,MAX_PACKET_LEN,0,(struct sockaddr*)&from,&ssize)) > 0)
            {
                bzero(&m,sizeof(struct message));
                message_parse(&m,buf);
                mprint(&m,bsize,inet_ntoa(from.sin_addr),ntohs(from.sin_port));
                mdnsd_in(d,&m,(unsigned long int)from.sin_addr.s_addr,from.sin_port);
            }
            if(bsize < 0 && errno != EAGAIN) { printf("can't read from socket %d: %s\n",errno,strerror(errno)); return 1; }
        }
        while(mdnsd_out(d,&m,&ip,&port))
        {
            bzero(&to, sizeof(to));
            to.sin_family = AF_INET;
            to.sin_port = port;
            to.sin_addr.s_addr = ip;
            printf("SEND(%d) -> %s:%d\n",message_packet_len(&m),inet_ntoa(to.sin_addr),ntohs(to.sin_port));
            if(sendto(s,message_packet(&m),message_packet_len(&m),0,(struct sockaddr *)&to,sizeof(struct sockaddr_in)) != message_packet_len(&m))  { printf("can't write to socket: %s\n",strerror(errno)); return 1; }
        }
    }

    mdnsd_shutdown(d);
    mdnsd_free(d);
    return 0;
}

