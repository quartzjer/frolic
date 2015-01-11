#ifndef b64_h
#define b64_h

// length of string resulting from encoding
#define b64_encode_len(x) (4 * (((x) + 2) / 3))

// encode str of len into out (must be at least b64_encode_len(len) big)
void b64_encode(unsigned char *str, int len, unsigned char *out);

// decode str into itself, return decoded len
int b64_decode(unsigned char *str);

#endif

