/*

    Test data types, structures, enums,...

*/

#define TEST    1

#if TEST
#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))
#endif

// Typedef
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned char BYTE;

// Simple array
char a_char[10];
char a_string[10][10];

int a_int2[10][2];

// Enum
enum days
{
    mon = 1, tue, wed, thr, fri, sat, sun
} week;

enum days week1, week2;

// Structure
typedef struct TagRecord
{
    char *pName;
    DWORD dwValue;
    enum days day;

} TRecord, *PTRecord;

// Array of structures
TRecord Rec[10];
struct TagRecord Rec2[10];

// Union
union number
{
    DWORD dwValue;
    WORD wValue;
    BYTE bValue;
} num;

// Basic type :)
int i;

// Advanced pointers
char Chr;
char *pChar;
char **ppChar;

typedef TRecord *(*TFunc)(TRecord *pRec, char **ppChar);

TRecord *func(TRecord *pRec, char **ppChar)
{
    pRec->dwValue = 0;
    printf("%08X\n", (DWORD)pRec);
    return( pRec );
}

TFunc pFunction;

int main()
{
#if TEST
    INT(3);
#endif

    num.bValue = 10;
    pFunction = func;

    for(i=0; i<10; i++)
    {
        a_char[i] = ' '+i;

        Rec[i].pName = "Sample";
        Rec[i].dwValue = (DWORD) i;
        Rec[i].day = wed;

        Chr = *Rec[i].pName;
        pChar = &Chr;
        ppChar = &pChar;

        func(&Rec[i], ppChar);
        pFunction(&Rec[i], ppChar);

        Rec2[i].dwValue = i*100;

        a_int2[i][0] = i;
        a_int2[i][1] = 10-i;

        strcpy(a_string[i], "12345");

        num.wValue = i;
    }

    return( 0 );
}

