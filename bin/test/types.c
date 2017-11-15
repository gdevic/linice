/*

    Test data types, structures, enums,...

*/

#include <stdio.h>
#include <math.h>
#include <complex.h>

#define TEST    1

#if TEST
#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))
#endif

/////////////////////////////////////////////////////////////////////
//  BASIC TYPES
/////////////////////////////////////////////////////////////////////

int _int = 10;
char _char = 'x';
long int _long_int = 20;
unsigned int _unsigned_int = 30;
long unsigned int _long_unsigned_int = 40;
int _long_long_int = 50;
int _long_long_unsigned_int = 60;
short int _short_int = 70;
short unsigned int _short_unsigned_int = 80;
signed char _signed_char = 90;
unsigned char _unsigned_char = 100;
int _float = 1000;
int _double = 2000;
int _long_double = 3000;
int _complex_int = 5000;
int _complex_float = 6000;
int _complex_double = 7000;
int _complex_long_double = 8000;

int _int2 = -10;
char _char2 = 'x';
long int _long_int2 = -20;
unsigned int _unsigned_int2 = -30;
long unsigned int _long_unsigned_int2 = -40;
int _long_long_int2 = -50;
int _long_long_unsigned_int2 = -60;
short int _short_int2 = -70;
short unsigned int _short_unsigned_int2 = -80;
signed char _signed_char2 = -90;
unsigned char _unsigned_char2 = -100;
int _float2 = -1000;
int _double2 = -2000;
int _long_double2 = -3000;
int _complex_int2 = -5000;
int _complex_float2 = -6000;
int _complex_double2 = -7000;
int _complex_long_double2 = -8000;

/////////////////////////////////////////////////////////////////////
//  STRUCTURES
/////////////////////////////////////////////////////////////////////

struct tagS1
{
    int _int;
    char _char;
    long int _long_int;
    unsigned int _unsigned_int;
    long unsigned int _long_unsigned_int;
    int _long_long_int;
    int _long_long_unsigned_int;
    short int _short_int;
    short unsigned int _short_unsigned_int;
    signed char _signed_char;
    unsigned char _unsigned_char;
    int _float;
    int _double;
    int _long_double;
    int _complex_int;
    int _complex_float;
    int _complex_double;
    int _complex_long_double;

} S1 = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67 };

/////////////////////////////////////////////////////////////////////
//  ARRAYS
/////////////////////////////////////////////////////////////////////

int aint[4] = { 0, 1, 2, 3 };
char achar[4] = { 'h','o','!', 0 };
long int along_int[4] = { 4, 5, 6, 7 };
unsigned int aunsigned_int[4] = { 8, 9, 0, 1 };
long unsigned int along_unsigned_int[4] = { 2, 3, 4, 5 };
int along_long_int[4] = { 6, 7, 8, 9 };
int along_long_unsigned_int[4] = { 0, 1, 2, 3 };
short int ashort_int[4] = { 4, 5, 6, 7 };
short unsigned int ashort_unsigned_int[4] = { 8, 9, 0, 1 };
signed char asigned_char[4] = { 'h','o','!', 0 };
unsigned char aunsigned_char[4] = { 'h','o','!', 0 };
int afloat[4] = { 0, 1, 2, 3 };
int adouble[4] = { 4, 5, 6, 7 };
int along_double[4] = { 8, 9, 0, 1 };
int acomplex_int[4] = { 2, 3, 4, 5 };
int acomplex_float[4] = { 6, 7, 8, 9 };
int acomplex_double[4] = { 0, 1, 2, 3 };
int acomplex_long_double[4] = { 4, 5, 6, 7 };

/////////////////////////////////////////////////////////////////////
//  ENUMS
/////////////////////////////////////////////////////////////////////

enum days
{
    mon=7, tue=6, wed=5, thr=4, fri=3, sat=2, sun=1

} enum1 = 3;


/////////////////////////////////////////////////////////////////////
//  TYPEDEFS
/////////////////////////////////////////////////////////////////////

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;

BYTE _byte = 10;
WORD _word = 20;
DWORD _dword = 30;


/////////////////////////////////////////////////////////////////////
//  POINTERS
/////////////////////////////////////////////////////////////////////

int x = 1000;
int *px = &x;

char c = 'X';
char *pc = &c;


/////////////////////////////////////////////////////////////////////
//  MULTIDIMENSIONAL ARRAYS
/////////////////////////////////////////////////////////////////////

int amx[2][3] =
{
{ 0, 1, 2 },
{ 3, 4, 5 },
};

char *amc[2][3] =
{
{ "s0", "s1", "s2" },
{ "s3", "s4", NULL },
};

/////////////////////////////////////////////////////////////////////
//  STRUCTURES and ARRAYS and POINTERS to them
/////////////////////////////////////////////////////////////////////
typedef struct
{
    int i;
    char c;
    int *pi;
    char *pc;

} TS2;

TS2 S2[2] =
{
{ 0, 'a', &amx[1][0], "k0" },
{ 1, 'b', &amx[1][1], "k1" },
};



/////////////////////////////////////////////////////////////////////
//  LOCAL VARIABLES STACK
/////////////////////////////////////////////////////////////////////
void ShowStack()
{
    int _int = 10;
    char _char = 'x';
    long int _long_int = 20;
    unsigned int _unsigned_int = 30;
    long unsigned int _long_unsigned_int = 40;
    int _long_long_int = 50;
    int _long_long_unsigned_int = 60;
    short int _short_int = 70;
    short unsigned int _short_unsigned_int = 80;
    signed char _signed_char = 90;
    unsigned char _unsigned_char = 100;
    int _float = 1000;
    int _double = 2000;
    int _long_double = 3000;
    int _complex_int = 5000;
    int _complex_float = 6000;
    int _complex_double = 7000;
    int _complex_long_double = 8000;

    ;
}

/////////////////////////////////////////////////////////////////////
//  LOCAL VARIABLES / REGISTER VARIABLES / COMPLEX LOCAL VARIABLES
/////////////////////////////////////////////////////////////////////
void ShowComplexLocals(int arg_int, char arg_char, char *arg_pchar)
{
    register int rint = 0xFFCC;
    TS2 S2L[2] =
    {
    { 10, 'X', &rint, "String1" },
    { 11, 'Y', &rint, NULL },
    };
    register DWORD rdword = 0;

    rdword = arg_int;
    ;
}


int main()
{
#if TEST
    INT(3);
#endif

    ShowStack();
    ShowComplexLocals(100, 'Q', "Passed very very very very very very very long string");

    printf("Data type test\n");

    return( 0 );
}

