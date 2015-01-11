/* jer:
 * This one is gleaned from Base64.xs in the perl MIME::Base64 sources,
 * the perl wrapper code in the module handles the line endings properly, not the decoder here
 */

#define MAX_LINE  76 /* size of encoded lines */

static char basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define XX      255	/* illegal base64 char */
#define EQ      254	/* padding */
#define INVALID XX

static unsigned char index_64[256] = {
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,62, XX,XX,XX,63,
    52,53,54,55, 56,57,58,59, 60,61,XX,XX, XX,EQ,XX,XX,
    XX, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,XX, XX,XX,XX,XX,
    XX,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,XX, XX,XX,XX,XX,

    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
    XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX, XX,XX,XX,XX,
};


int b64decode(char *r)
{
	int len = strlen(r);
	unsigned char *str,*end;
	char *start = r;
	unsigned char c[4];
end=r+len; str = r;
	while (str < end) {
	    int i = 0;
            do {
		unsigned char uc = index_64[*str++];
		if (uc != INVALID)
		    c[i++] = uc;

		if (str == end) {
		    if (i < 4) {
			if (i < 2) goto thats_it;
			if (i == 2) c[2] = EQ;
			c[3] = EQ;
		    }
		    break;
		}
            } while (i < 4);
	    
	    if (c[0] == EQ || c[1] == EQ) {
		break;
            }
	    /* printf("c0=%d,c1=%d,c2=%d,c3=%d\n", c[0],c[1],c[2],c[3]);/**/

	    *r++ = (c[0] << 2) | ((c[1] & 0x30) >> 4);

	    if (c[2] == EQ)
		break;
	    *r++ = ((c[1] & 0x0F) << 4) | ((c[2] & 0x3C) >> 2);

	    if (c[3] == EQ)
		break;
	    *r++ = ((c[2] & 0x03) << 6) | c[3];
	}

      thats_it:
	*r = '\0';
        return (r - start);
}
