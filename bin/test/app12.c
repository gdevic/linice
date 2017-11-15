// Tests:
//
//      * Arguments to a function
//      * local variable
//      * global variable
//
//      * static function
//      * global function

int global;

#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))

static int fnStatic(int arg);
int fnGlobal(int arg);
void fnNested(void);

extern int fnGlobal2(int arg);

main(int argn, char *argv[])
{
    int local;

    // !!!!! WATCH THIS !!!!!
    INT(3);
    local = 1;

    printf("argn=%d\n", argn);
    for(local=0; local<argn; local++)
    {
        printf(" %d) %s\n", local, argv[local]);
    }

    global = 1;
    fnStatic(global);

    fnGlobal2(global);

    fnNested();

    global = fnScope(global);
}

static int fnStatic(int arg)
{
    printf("Static function: %d\n", arg);
    global++;
    fnGlobal(global);
    return( 0 );
}

int fnGlobal(int arg)
{
    printf("Global function: %d\n", arg);
    global++;
    return( arg+1 );
}

void fnNested(void)
{
    int fnInside(int arg)
    {
        arg--;
        return( arg );
    }

    int local = 0;

    while( global-- )
    {
        printf("Nested: %d\n", local = fnInside(local));
    }
}

int fnScope(int arg)
{
    int i = 0;

    while( i++ < 3 )
    {
        int b = 0;
        while( b++ < 2 )
        {
            int c;

            arg += c++;
        }
    }

    return( arg );
}
