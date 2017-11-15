
extern int global;

static int fnStatic2(int arg);
int fnGlobal2(int arg);

typedef signed long SWORD;

SWORD global_sword;

static SWORD static_variable_app22;
static SWORD static_variable_app23 = 23;

static int fnStatic2(int arg)
{
    printf("Static function 2: %d\n", arg);
    global++;
    fnGlobal(global);
    static_variable_app22 = 0;
    return( 0 );
}

int fnGlobal2(int arg)
{
    printf("Global function 2: %d\n", arg);
    global++;
    fnStatic2(global);
    return( arg+1 );
}

