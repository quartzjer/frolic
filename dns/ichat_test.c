#include "1035.h"
#include "ichat.h"

int main(int argc, char *argv[])
{
    struct message *m;
    unsigned char *packet;
    int len = 0;
    xht h;

/*
    m = message_wire();
    m->id = 257;
    m->head1.rd = 1;
    message_qd(m, "foo.com", 1);
    message_an(m, "bar.com", 2, 10);
    message_rdata(m, 4, "abc");
    m->_len += _host(m,&packet,"test");
    m->_len += _host(m,&packet,"foo.test");
    m->_len += _host(m,&packet,"new.label.host.");
    m->_len += _host(m,&packet,"label.host.");
    m->_len += _host(m,&packet,"label.host.test");
    m->_len += _host(m,&packet,"label.foo");
    packet = message_packet(m,&len);
*/

    h = xht_new(11);
    xht_set(h,"1st","Foo");
    xht_set(h,"last","Bar");
    xht_set(h,"status","avail");
    xht_set(h,"msg","Howdy Y'all");
xht_set(h,"long","********************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************************");

    packet = ichat2txt(h, &len);
    printf("len %d\n",len);
    hexdump(packet,len);
    xht_free(h);
    free(packet);
}

