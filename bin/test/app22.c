
extern int global;

static int fnStatic2(int arg);
int fnGlobal2(int arg);


static int fnStatic2(int arg)
{
    printf("Static function 2: %d\n", arg);
    global++;
    fnGlobal(global);
    return( 0 );
}

int fnGlobal2(int arg)
{
    printf("Global function 2: %d\n", arg);
    global++;
    fnStatic2(global);
    return( arg+1 );
}

