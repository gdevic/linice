// Tests:
//
//  Data types

#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))

typedef struct
{
    int i;
    
} t_struct;

main(int argn, char *argv[])
{
    // !!!!! WATCH THIS !!!!!
    INT(3);

    int t_int = 0;
    int t_int_a[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int *t_pint = &t_int;

    t_struct t_str;


    t_str.i = t_int;

}

