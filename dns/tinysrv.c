
// what a hack, I'm sure there's a better way, but I still hate dns
short int _ts_ljump(unsigned char *ptr)
{
    short int i = 0;
    unsigned char j = 0xc0;
    j ^= ptr[0];
    ((unsigned char*)&i)[0] = j;
    ((unsigned char*)&i)[1] = ptr[1];
    return i;
}

void _ts_lcopy(unsigned char *a, unsigned char *label, char *host)
{
    int len;

    if(*label == 0)
    {
        *host = 0;
        return;
    }

    // copy chars for this label
    for(len = 1;len <= *label; len++)
        *host++ = label[len];
    *host++ = '.';

    // next label
    label += *label + 1;

    // follow compression pointers
    if(*label & 0xc0) return _ts_lcopy(a,a+_ts_ljump(label),host);

    // recurse to next label
    _ts_lcopy(a,label, host);
}

// internal label matching utility
int _ts_lmatch(unsigned char *a, unsigned char *l1, unsigned char *l2)
{
    int len;

    // always ensure we get called w/o a pointer
    if(*l1 & 0xc0) return _ts_lmatch(a,a + _ts_ljump(l1),l2);
    if(*l2 & 0xc0) return _ts_lmatch(a,l1,a + _ts_ljump(l2));
    
    // same already?
    if(l1 == l2) return 1;
    
    // compare all label characters
    if(*l1 != *l2) return 0;
    for(len = 1; len <= *l1; len++) 
        if(l1[len] != l2[len]) return 0;

    // get new labels
    l1 += *l1 + 1;
    l2 += *l2 + 1;
    
    // at the end, all matched
    if(*l1 == 0 && *l2 == 0) return 1;
    
    // try next labels
    return _ts_lmatch(a,l1,l2);
}

// internal generic request creator
void _ts_q(char *host, unsigned char *q, int qtype)
{
    static short int counter = 1;
    int last, x, y;

    if(host == 0) return;

    // header
    q[0] = ((char*)&counter)[1];
    q[1] = ((char*)&counter)[0];
    q[2] = q[5] = 1;
    q[3] = q[4] = q[6] = q[7] = q[8] = q[9] = q[10] = q[11] = 0;

    // question
    last = 12;
    x = 13;
    y = 0;
    while(host[y])
    {
        if(host[y] == '.')
        {
            if(!host[y+1]) break;
            q[last] = x - (last + 1);
            last = x;
        }else{
            q[x] = host[y];
        }
        x++;
        y++;
    }
    q[last] = x - (last + 1);
    q[x++] = 0;
    q[x++] = 0;
    q[x++] = qtype;
    q[x++] = 0;
    q[x++] = 1;
}

// checks to make sure answer matches question
// caller ensures a len is > q len
int ts_check(unsigned char *q, unsigned char *a)
{
    int x, y;

    // header checks out
    if(!(a[0] == q[0] && a[1] == q[1] && (a[2] == 0x81 || a[2] == 0x85 || a[2] == 0x87) && a[3] == 0x80 && a[4] == 0 && a[5] == 1 && a[6] == 0 && a[7] > 0))
        return -1;

    // spin through question label, validating
    for(x = 12; a[x] != 0; x++)
        if(a[x] != q[x]) return -1;

    // check rest of question validity
    for(y = x+4; x <= y; x++)
        if(a[x] != q[x]) return -1;

    return 0;
}

// returns number of answers in a
int ts_count(unsigned char *a)
{
    return a[7];
}

// a is required to have passed tr_check() first
// returns ip, if multiple in a, uses which (0 is first, 1 is second, etc)
// -1: error or none, 0: host is set to A record that needs to be resolved yet
unsigned long int ts_ip(int which, unsigned char *a, int len, char *host, int *port)
{
    unsigned char *rr, *srv_target[10], *buf = a;
    unsigned short int i;
    unsigned long int l;
    int srv = 0, srvs = 0, aflag = 0;
    short int srv_port[10], srv_pri[10], srv_weight[10];

// some shorthand space savers
#define BUF_LEN(x) if((buf+x)-a > len) return 0;
#define BUF_SHORT(x) BUF_LEN(2); ((char*)&x)[0] = *buf++; ((char*)&x)[1] = *buf++;
#define BUF_LONG(x) BUF_LEN(4); ((char*)&x)[0] = *buf++; ((char*)&x)[1] = *buf++; ((char*)&x)[2] = *buf++; ((char*)&x)[3] = *buf++;
#define BUF_LABEL for(; *buf != 0 && !(*buf & 0xc0 && buf++); buf += *buf + 1) if((buf + *buf + 1) > (a + len)) return 0; buf++;

    // start with the question label, find the end, then skip on to the first rr
    buf += 12;
    BUF_LABEL;
    BUF_SHORT(i);
    if(i == 1) aflag = 1;
    BUF_SHORT(i);

    // process one srv rr per loop
    while(buf < (a + len))
    {
        // store the name and spin through the label
        rr = buf;
        BUF_LABEL;
        
        // make sure this is srv, or kick out
        BUF_SHORT(i);
        if(i != 33) break;
        BUF_SHORT(i);
        if(i != 1) break;
        BUF_LONG(l); // ignore ttl, no caching here
        
        // make sure srv data is right size
        BUF_SHORT(i);
        BUF_LEN(i);
        
        // get all our srv data
        BUF_SHORT(i);
        srv_pri[srvs] = i;
        BUF_SHORT(i);
        srv_weight[srvs] = i;
        BUF_SHORT(i);
        srv_port[srvs] = i;
        srv_target[srvs] = buf;
        BUF_LABEL;
        srvs++;
        if(srvs == 10) break;
    }

    if(!aflag && (srvs == 0 || which >= srvs)) return 0;
    
    // reset buf to start of rr when kicked out
    buf = rr;
    
    // process the srv records, select match if any
    while(srvs > 1)
    {
        int cur;
        // find best
        for(cur = 0; cur < srvs; cur++)
        {
            if(srv_pri[srv] > srv_pri[cur] || (srv_pri[srv] == srv_pri[cur] && srv_weight[srv] > srv_weight[cur])) continue;
            srv = cur;
        }
        // bail if we want the first match now
        if(which == 0) break;

        // else zero it and find another
        srv_pri[srv] = srv_weight[srv] = -1;
        which--;
    }

    // set right port for return
    if(!aflag)
        *port = srv_port[srv];

    // search remaining buf for target A (or CNAME?) record matching target, or any IP if A answer
    while(buf < (a + len))
    {
        int match =  0;
        
        // process label first, make sure it's ok, then let match compare them
        rr = buf;
        BUF_LABEL;
        if(aflag)
            match = (which-- == 0)?1:0;
        else
            match = _ts_lmatch(a, srv_target[srv], rr);
        
        BUF_SHORT(i);
        // handle returning CNAMEs
        if(i == 5 && match == 1)
        {
            BUF_SHORT(i);
            BUF_LONG(l);
            BUF_SHORT(i);
            BUF_LEN(i);
            _ts_lcopy(a,buf,host);
            return 0;
        }

        // make sure this is A
        if(i != 1) match = 0;
        BUF_SHORT(i);
        if(i != 1) match = 0;
        BUF_LONG(l); // ignore ttl, no caching here
        
        // make sure rr data is right size
        BUF_SHORT(i);
        BUF_LEN(i);

        // found our ip, all done, return in network order
        if(match)
        {
            BUF_LEN(4);
            ((char*)&l)[0] = *buf++;
            ((char*)&l)[1] = *buf++;
            ((char*)&l)[2] = *buf++;
            ((char*)&l)[3] = *buf++;
            return l;
        }
        
        // skip this rr's data and move on to next
        buf += i;
    }

    // no matching ip, copy in the target host label
    _ts_lcopy(a,srv_target[srv],host);
    return 0;
}


// ts_srv/a create a q of size 18 + strlen(host)

// stores SRV request for host in question
void ts_srv(char *host, unsigned char *q)
{
    _ts_q(host, q, 33);
}

// stores A request for host in question
char *ts_a(char *host, unsigned char *q)
{
    _ts_q(host, q, 1);
}
