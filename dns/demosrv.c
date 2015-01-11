#include "lib.h";

void dump(unsigned char *b, int len)
{
    int i, j = 0;
    printf("dumping packet of len %d\n",len);
    while(j < len)
    {
        printf("%d:\t", j);
        for(i = 7; i>=0; i--)
        {
            if((1<<i)&b[j]) printf("1");
            else printf("0");
        }
        printf("\t%d\t%02x\t%c\n", b[j], b[j], b[j]);
        j++;
    }
}

void lookup(int s, int aflag, char *host)
{
    int qsize, asize, count, port = 0;
    struct in_addr ip;
    unsigned long int ret;
    unsigned char q[512], a[512];
    char ahost[256];

    if(aflag)
        ts_a(host, q);
    else
        ts_srv(host, q);
    qsize = 18+strlen(host);
    
    printf("%d lookup of %s\n",aflag,host);

    if(s < 1 || send(s,q,qsize,0) != qsize || (asize = recv(s,a,512,0)) < qsize)
    {
        printf("dead socket (%s) or bad response\n",strerror(errno));
        return;
    }

//    if(aflag) dump(a,asize);
    printf("response check [%d] of [%d] answers\n",ts_check(q,a),ts_count(a));
    if(ts_check(q,a)) return;
    for(count = 0; count < ts_count(a); count++)
    {
        bzero(ahost,256);
        ret = ts_ip(count,a,asize,ahost, &port);
        if(ret == 0 && *ahost == 0) continue; // done
        ip = *((struct in_addr *)&ret);
        printf("[%d] processed [%s]/[%s]:%d\n",count,inet_ntoa(ip),ahost,port);
        if((!aflag && count > 1) && *ahost != 0)
            lookup(s,1,ahost);
    }
}

int main(int argc, char *argv[])
{
    int s, qsize, asize, count, port = 0;
    struct in_addr ip;
    unsigned long int ret;
    unsigned char q[512], a[512];
    char ahost[256];

    if(argc != 3) {printf("usage: (SRV hostname) (ip of dns server)\n\tdemosrv _foo._tcp.host.com 12.34.56.78\n"); return;}

    // socket to dns server
    s = make_netsocket(53,argv[2],NETSOCKET_UDP);
    lookup(s,0,argv[1]);
    return 0;
}
