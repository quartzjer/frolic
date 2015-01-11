#ifndef ichat_h
#define ichat_h
#include "xht.h"

// returns ichat data from a _ichat._tcp TXT record rdata as a simple hashtable of values
xht txt2ichat(unsigned char *txt, int len);

// returns a raw block that can be sent with a TXT record for _ichat._tcp, sets length
// handles most of the ugly stuff, just pass attributes for the packet
unsigned char *ichat2txt(xht h, int *len);

#endif
