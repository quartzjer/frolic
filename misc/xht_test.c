#include "xht.h"

void list(xht h, const char *key, void *val, void *arg)
{
    printf("%s=%s\n",key,(char*)val);
}

int main(int argc, char *argv[])
{
    xht x;

    x = xht_new(11);
    xht_set(x,"key","value");
    xht_set(x,"foo","bar");
    xht_set(x,"abc","defgh");
    printf("key is %s ",xht_get(x,"key"));
    xht_set(x,"key","different");
    printf("and is now %s ",xht_get(x,"key"));
    xht_set(x,"key",0);
    printf("and finally is %s\n",xht_get(x,"key"));
    xht_walk(x,list,0);
}
