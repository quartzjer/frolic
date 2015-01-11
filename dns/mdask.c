
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

void deliver(int s, int type, char *host, struct sockaddr *to)
{
    int qsize, asize, i;
    unsigned char *q, a[MAX_PACKET_LEN];
    struct message m;

    printf("lookup of %s type %d:\n",host,type);
/*    q = malloc(512);
    ask(host,q,type);
    qsize = 18+strlen(host);
*/
    bzero(&m,sizeof(struct message));
    m.id = 2;
    m.header.rd = 1;
    message_qd(&m, host, type, 1);
    q = message_packet(&m,&qsize);
    hexdump(q,qsize);
    if(s < 1 || sendto(s,q,qsize,0,to,sizeof(struct sockaddr_in)) != qsize)
    {
        free(m);
        printf("dead socket (%s) or bad response\n",strerror(errno));
        return;
    }
while(1){
    bzero(a,4096);
    if((asize = recvfrom(s,a,4096,0,0,0)) < qsize)
    {
        printf("dead socket (%s) or bad response\n",strerror(errno));
        return;
    }
    printf("answer of size %d:\n",asize);
//    hexdump(a,asize);
    bzero(&m,sizeof(struct message));
    message_parse(&m,a);
    mprint(&m);
}
}


int main(int argc, char *argv[])
{
    int s, port, type, flag = 1;
    struct sockaddr_in to;

    if(argc != 5 || (port = atoi(argv[4])) <= 0 || (type = atoi(argv[2])) <= 0)
    {
        printf("usage: (host) (type) (dns ip) (dns port)\n\tmdtool _foo._tcp.host.com 33 12.34.56.78 53\n"); 	return;
    }

    bzero(&to, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = inet_addr(argv[3]);

    s = socket(AF_INET,SOCK_DGRAM,0);
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));

    printf("connecting to %s:%d\n",argv[3],port);
    deliver(s,type,argv[1],(struct sockaddr *)&to);
    return 0;
}

/* bit fields
struct
{
    unsigned short icon : 8;
    unsigned short color : 4;
    unsigned short underline : 1;
    unsigned short blink : 1;
} screen[25][80];
*/
