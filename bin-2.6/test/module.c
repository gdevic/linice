
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");


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

int init_module_2_6(void)
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

void cleanup_module_2_6(void)
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


module_init(init_module_2_6);
module_exit(cleanup_module_2_6);
