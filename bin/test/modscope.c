#define MODULE
#include <linux/module.h>

int global = 3;

int init_module(void)
{
    int local;
    register int local2 = 100;
    register int local3 = 0;

    for(local3=0; local3<1; local3++)
    {
        volatile int scoped1;

        scoped1 = local2 + local3;
    }

    for(local=0; local<global; local++)
    {
        register int local4 = local3 + local2;
        int j = local + 1;
        funct(3, local4);
    }

    return( 0 );
}

int funct2()
{
    int i1 = 0;
    {
        int i2 = 1;
        {
            int i3 = 2;
            {
                int i4 = i1+i2+i3+i4;
            }
        }
    }
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
