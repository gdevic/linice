#define MODULE
#include <linux/module.h>

int global = 10;
static int static_var = 1;
int global2;

int init_module(void)
{
    int i;

    printk("<1>init_module\n");

    static_fn();

    for(i=0; i<10; i++)
    {
        printk("%d", increment(global));
    }
    
    return( 0 );
}

void cleanup_module(void)
{
    printk("<1>cleanup_module\n");
}

int increment(int i)
{
    return( ++i );
}

static int static_fn(void)
{
    global = 0;
}
