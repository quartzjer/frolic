#include "ichat.h"

#include <string.h>
#include "b64.h"

void _ichat2txt_count(xht h, const char *key, void *val, void *arg)
{
    int *count = (int*)arg;
    
    count[1] += strlen(key) + 2;
    count[1] += strlen(val) + 2;
    count[0]++;
}

void _ichat2txt_write(xht h, const char *key, void *val, void *arg)
{
    unsigned char **txtp = (unsigned char **)arg;
    int len;

    // copy in lengths, then strings
    len = strlen(key);
    short2net(len,txtp);
    memcpy(*txtp,key,len);
    *txtp += len;
    len = strlen(val);
    short2net(len,txtp);
    memcpy(*txtp,val,len);
    *txtp += len;
}

unsigned char *ichat2txt(xht h, int *len)
{
    static unsigned long int sequence = 1;
    unsigned char *txt, *raw, *buf;
    unsigned char size, head[] = "<?xml version='1.0' encoding='UTF-8'?><!DOCTYPE plist PUBLIC '-//Apple Computer//DTD PLIST 1.0//EN' 'http://www.apple.com/DTDs/PropertyList-1.0.dtd'><plist version='1.0'><data>", tail[] = "</data></plist>";
    int rawlen = 20, txtlen, count[2] = {0,0};

    xht_walk(h,_ichat2txt_count,(void*)count);
    rawlen += count[1];
    buf = raw = (unsigned char *)malloc(rawlen);

    // write header/block stuff
    memcpy(buf,"subn",4); // old header name
    buf += 4;
    long2net(1,&buf); // version
    long2net(sequence++,&buf); // sequence always increasing
    long2net(1,&buf); // type field?
    long2net(count[0],&buf); // number of attributes
    xht_walk(h,_ichat2txt_write,&buf);
    
    txtlen = b64_encode_len(rawlen);
    txtlen += strlen(head) + strlen(tail);
    txtlen += (txtlen / 255) + 1; // blocks
    *len = txtlen;  // return the resulting total length
    txt = (unsigned char *)malloc(txtlen);
    b64_encode(raw, rawlen, txt);
    free(raw);
    memmove(txt+strlen(head),txt,b64_encode_len(rawlen));
    memcpy(txt,head,strlen(head));
    memcpy(txt+strlen(head)+b64_encode_len(rawlen),tail,strlen(tail));

    // chunk it out (block length prefixing)
    for(buf = txt; txtlen > 0; txtlen -= 255)
    {
        size = (txtlen > 255) ? 255 : txtlen;
        memmove(buf+1,buf,txtlen);
        *buf = size;
        buf += size + 1;
    }
    return txt;
}

xht txt2ichat(unsigned char *txt, int len)
{
    int b64len, lumplen = 0, namelen, i;
    char *st, *et, *name, *b64;
    char *lump, key[4];
    long int version, sequence, type, count;
    xht h;

    if(txt == 0 || len == 0) return 0;

    // make our own copy so we can fix it (block length prefixing, ick)
    lump = (char*)malloc(len);
    bzero(lump,len); // lump is always larger, will be 0 terminated
    for(;*txt <= len && len > 0; len -= *txt, lumplen += *txt, txt += *txt + 1)
        memcpy(lump+lumplen,txt+1,*txt);

    // the xml ichat sends is static and depreciated, just rip out the base64
    if((st = strstr(lump,"<data>")) == 0 || (et = strstr(st,"</data>")) == 0)
    {
        free(lump);
        return 0;
    }
    
    b64 = st + 6;
    *et = 0;
    if((b64len = b64_decode(b64)) < 20)
    {
        free(lump);
        return 0;
    }

    // get the values, even though we don't really care, depreciated protocol
    st = b64; // 4 bounds check l8r
    memcpy(key,b64,4); b64 += 4;
    version = net2long(&b64);
    sequence = net2long(&b64);
    type = net2long(&b64);
    count = net2long(&b64);
//    printf("ichat '%.*s' ver %d seq %d type %d with %d attributes:",4,key,version,sequence,type,count);

    // get what we really want, the attributes
    h = xht_new(23);
    for(i = 0; i < count; i++)
    {
        // get name
        namelen = net2short(&b64);
        if((b64 + namelen) - st > b64len) break;
        name = b64;
        b64 += namelen;

        // get value and store
        len = net2short(&b64);
        if((b64 + len) - st > b64len) break;
        xht_store(h, name, namelen, b64, len);
        b64 += len;
    }
    free(lump);
    return h;
}
