#include "mdnsd.h"

void ipush_bad(char *name, int type, void *arg)
{
    printf("ipush conflict detected %d:%s, arg %s\n",type,name,arg);
}

void ipush_reg(mdnsd d, char *ip, int port)
{
    unsigned char *packet;
    int len = 0;
    xht h;
    mdnsdr r;
    unsigned long int iip;

    r = mdnsd_shared(d,"_ichat._tcp.local.",QTYPE_PTR,120);
    mdnsd_set_host(d,r,"ipushtest._ichat._tcp.local.");
    r = mdnsd_unique(d,"ipushtest._ichat._tcp.local.",QTYPE_SRV,120,con,"SRV");
    mdnsd_set_srv(d,r,0,0,port,"ipushtest.local.");
    r = mdnsd_unique(d,"ipushtest.local.",QTYPE_A,120,con,"A");
    iip = inet_addr(ip);
    mdnsd_set_raw(d,r,(unsigned char *)&iip,4);
    r = mdnsd_unique(d,"ipushtest._ichat._tcp.local.",16,120,con,"TXT");
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

}

