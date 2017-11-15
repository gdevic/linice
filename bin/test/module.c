/*	1	2	3	4	5	6	7
1	2	3	4	5	6	7	8	9	0
-	=	q
*/
	#define MODULE
	#include <linux/module.h>

typedef struct
{
    int display;
    char *name;
} GC;

typedef GC *PGC;

PGC Displays[2];
GC Display2[2];
GC OneDisplay;

typedef struct
  {
    unsigned long int __val[10];
    char x;
  } __sigset_t;


typedef unsigned char U008;

U008 BTFF0[] = { 0x1B, 0x1B, 0x1B, 0x00,
                 0x12, 0x12, 0x1B, 0x00 };
U008 BTFF1[] = { 0x9B, 0x9B, 0x80, 0x80,
                 0x9B, 0x9B, 0x80, 0x80 };
U008 BTFF2[] = { 0xC0, 0xC0, 0x92, 0xF6,
                 0xC0, 0xC0, 0x92, 0xF6 };


typedef unsigned long DWORD;

DWORD global_dword;

static int fnStatic(int arg);
int fnGlobal(int arg);
void fnNested(void);

char ***pppChar = 0;


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
