#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");


int i, j=0;

int init_module_2_6(void)
{

    i=0;
    while( i<3 )
    {
        i++;
    }

    i=0;
    do
    {
        i++;
    }
    while( i<3 );

    for(i=0; i<3; i++)
    {
        j++;
    }

    switch( j )
    {
        case 0:     i=0;  break;
        case 2:     i=4;  break;
        default:
            j++;
            break;
    }

    goto label;

    j++;
label:

    return( 0 );
}

void cleanup_module_2_6(void)
{
}


module_init(init_module_2_6);
module_exit(cleanup_module_2_6);
