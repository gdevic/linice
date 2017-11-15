#define MODULE
#include <linux/module.h>

int i, j=0;

int init_module(void)
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

void cleanup_module(void)
{
}
