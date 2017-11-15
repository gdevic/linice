#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

int global = 3;
char gu;

const int ro[3] = { 0, 1, 2 };

int func1()
{
    funct(0, 0);
}

int init_module_2_6(void)
{
    int local;
    register int local2 = 100;
    register int local3 = 0;

    // Global uninitialized
    gu = 'x' + ro[0];

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

void cleanup_module_2_6(void)
{
}


module_init(init_module_2_6);
module_exit(cleanup_module_2_6);
