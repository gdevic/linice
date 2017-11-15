#define INT(_x) __asm__ __volatile__("int %0" :: "g" (_x))

main()
{
  INT(3);
}

