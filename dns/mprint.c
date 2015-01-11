#include "1035.h"
#include "xht.h"
#include "ichat.h"
#include "sdtxt.h"

void _mprint_list(xht h, const char *key, void *val, void *arg)
{
    printf("%s='%s' ",key,(char*)val);
}

void _mprint(struct resource *r, char *name)
{
    xht h;
	if(!r || !name) return;
        printf("  %s %d %s (%d) <%d>: ",name,r->type,r->name,r->class,r->ttl);
        switch(r->type)
        {
            case 1:
                printf("ip %s\n",r->known.a.name);
                break;
            case 2:
                printf("server %s\n",r->known.ns.name);
                break;
            case 5:
                printf("cname to %s\n",r->known.cname.name);
                break;
            case 12:
                printf("ptr to %s\n",r->known.ptr.name);
                break;
            case 33:
                printf("srv %d/%d to %s:%d\n",r->known.srv.priority,r->known.srv.weight,r->known.srv.name,r->known.srv.port);
                break;
            case 16:
                if((h = txt2ichat(r->rdata, r->rdlength)) != 0 || (h = txt2sd(r->rdata,r->rdlength)) != 0)
                {
                    printf("txt ");
                    xht_walk(h,_mprint_list,0);
                    xht_free(h);
                    printf("\n");
                    break;
                }
            default:
                printf("raw +%d\n",r->rdlength);
                break;
        }
}

void mprint(struct message *m, int len, char *ip, int port)
{
    int i;
    static int last = 0;
    if(m->header.qr)
        printf("RESPN ");
    else
        printf("QUERY ");
    printf("+%d ",time(0) - last);
    last = time(0);
    if(len)
        printf("%d %s:%d ",len,ip,port);
    if(m->id)
        printf("ID:%d ",m->id);
    if(m->header.aa)
        printf("AA ");
    if(m->header.tc)
        printf("TC ");
    printf("\n");
//    printf("ID: %d QR[%d] Opcode[%d] AA[%d] TC[%d] RD[%d] RA[%d] Z[%d] RCODE[%d]\n",m->id, m->header.qr,m->header.opcode,m->header.aa,m->header.tc,m->header.rd,m->header.ra,m->header.z,m->header.rcode);
//    printf("questions[%d] answers[%d] servers[%d] additional[%d]\n",m->qdcount,m->ancount,m->nscount,m->arcount);
    for(i = 0; i < m->qdcount; i++)
    {
        printf("  Q %d %s (%d)\n",m->qd[i].type,m->qd[i].name,m->qd[i].class);
    }
    for(i = 0; i < m->ancount; i++) _mprint(&(m->an[i]),"A");
    for(i = 0; i < m->nscount; i++) _mprint(&(m->ns[i]),"S");
    for(i = 0; i < m->arcount; i++) _mprint(&(m->ar[i]),"E");
}
