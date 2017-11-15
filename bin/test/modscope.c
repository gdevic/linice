#define MODULE
#include <linux/module.h>

int global = 3;

int init_module(void)
{
    int local;

    for(local=0; local<global; local++)
    {
        int j = local + 1;
        funct(3, j);
    }
    
    return( 0 );
}

int funct(int param1, int param2)
{
    int loc1, ret = 0;

    loc1 = param1;

    while( loc1-- )
    {
        int loc2;

        loc2 = loc1 + param2;
        ret += loc2;
    }

    return( ret );
}

void cleanup_module(void)
{
}
