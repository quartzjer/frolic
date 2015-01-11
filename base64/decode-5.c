/* jer:
 * Yet another decoder, unknown origin, and a bit more of a brute force approach than the others
 */ 

#define decode(c) if(c >= 'A' && c <= 'Z') c  = c - 'A'; \
             else if(c >= 'a' && c <= 'z') c  = c - 'a' + 26; \
             else if(c >= '0' && c <= '9') c  = c - '0' + 52; \
             else if(c == '+')             c  = 62; \
             else if(c == '/')             c  = 63; \
             else                          c  = 0; \

int b64decode(const char *src)
{
  int x, y = 0;
  int len;
  char triple[3];
  char quad[4];
  char *dst;

  len = strlen(src);
  for(x = 0; x < len; x += 4)
    {
      memset(quad, 0, 4);
      memcpy(quad, &src[x], 4 - (len - x) % 4);

      decode(quad[0]);
      decode(quad[1]);
      decode(quad[2]);
      decode(quad[3]);

      triple[0] = (quad[0] << 2) | quad[1] >> 4;
      triple[1] = ((quad[1] << 4) & 0xF0) | quad[2] >> 2;
      triple[2] = ((quad[2] << 6) & 0xC0) | quad[3];

      memcpy(&dst[y], triple, 3);
      y += 3;
    }
  return y;
}

