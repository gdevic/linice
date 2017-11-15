static TASMADDR tasm_aaa[] = {    /*   0 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x37"}, };

static TASMADDR tasm_aad[] = {    /*   1 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xD5\24"}, //           void
/*  2 */   { {0, 0, 0 }, "\2\xD5\x0A"}, };

static TASMADDR tasm_aam[] = {    /*   2 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xD4\24"}, //           void
/*  2 */   { {0, 0, 0 }, "\2\xD4\x0A"}, };

static TASMADDR tasm_aas[] = {    /*   3 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x3F"}, };

static TASMADDR tasm_adc[] = {    /*   4 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x10\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x10\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x11\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x11\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x11\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x11\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x12\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x12\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x13\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x13\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x13\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x13\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x14\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x15\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x15\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\202\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\202\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\202\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\202\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\202\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\202\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\202\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\202\41"}, };

static TASMADDR tasm_add[] = {    /*   5 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\17\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\17\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x01\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x01\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x01\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x01\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x02\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x02\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x03\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x03\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x03\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x03\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x04\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x05\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x05\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\200\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\200\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\200\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\200\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\200\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\200\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\200\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\200\41"}, };

static TASMADDR tasm_and[] = {    /*   6 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x20\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x20\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x21\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x21\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x21\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x21\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x22\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x22\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x23\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x23\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x23\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x23\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x24\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x25\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x25\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\204\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\204\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\204\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\204\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\204\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\204\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\204\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\204\41"}, };

static TASMADDR tasm_arpl[] = {    /*   7 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\300\1\x63\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\300\1\x63\101"}, };

static TASMADDR tasm_bound[] = {    /*   8 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x62\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x62\110"}, };

static TASMADDR tasm_bsf[] = {    /*   9 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xBC\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\xBC\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xBC\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\xBC\110"}, };

static TASMADDR tasm_bsr[] = {    /*  10 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xBD\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\xBD\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xBD\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\xBD\110"}, };

static TASMADDR tasm_bswap[] = {    /*  11 */
//           reg32
/*  1 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\1\x0F\10\xC8"}, };

static TASMADDR tasm_bt[] = {    /*  12 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA3\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA3\101"}, //           mem,reg32
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA3\101"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA3\101"}, //           rm16,imm
/*  5 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\2\x0F\xBA\204\25"}, //           rm32,imm
/*  6 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\2\x0F\xBA\204\25"}, };

static TASMADDR tasm_btc[] = {    /*  13 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xBB\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xBB\101"}, //           mem,reg32
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xBB\101"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xBB\101"}, //           rm16,imm
/*  5 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\2\x0F\xBA\207\25"}, //           rm32,imm
/*  6 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\2\x0F\xBA\207\25"}, };

static TASMADDR tasm_btr[] = {    /*  14 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xB3\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xB3\101"}, //           mem,reg32
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xB3\101"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xB3\101"}, //           rm16,imm
/*  5 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\2\x0F\xBA\206\25"}, //           rm32,imm
/*  6 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\2\x0F\xBA\206\25"}, };

static TASMADDR tasm_bts[] = {    /*  15 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xAB\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xAB\101"}, //           mem,reg32
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xAB\101"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xAB\101"}, //           rm16,imm
/*  5 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\2\x0F\xBA\205\25"}, //           rm32,imm
/*  6 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\2\x0F\xBA\205\25"}, };

static TASMADDR tasm_call[] = {    /*  16 */
//           imm16
/*  1 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xE8\64"}, //           imm16|near
/*  2 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xE8\64"}, //           imm16|far
/*  3 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\x9A\34\37"}, //           imm32
/*  4 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xE8\64"}, //           imm32|near
/*  5 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xE8\64"}, //           imm32|far
/*  6 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\x9A\34\37"}, //           imm16:imm
/*  7 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\x9A\31\30"}, //           imm:imm16
/*  8 */   { {ASM_IMM, 0, 0 }, "\320\1\x9A\31\30"}, //           imm32:imm
/*  9 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\x9A\41\30"}, //           imm:imm32
/* 10 */   { {ASM_IMM, 0, 0 }, "\321\1\x9A\41\30"}, //           mem16|far
/* 11 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\203"}, //           mem32|far
/* 12 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\203"}, //           mem16|near
/* 13 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\202"}, //           mem32|near
/* 14 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\202"}, //           reg16
/* 15 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\300\1\xFF\202"}, //           reg32
/* 16 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\202"}, //           mem16
/* 17 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\202"}, //           mem32
/* 18 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\202"}, };

static TASMADDR tasm_cbw[] = {    /*  17 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x98"}, };

static TASMADDR tasm_cdq[] = {    /*  18 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x99"}, };

static TASMADDR tasm_clc[] = {    /*  19 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF8"}, };

static TASMADDR tasm_cld[] = {    /*  20 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xFC"}, };

static TASMADDR tasm_cli[] = {    /*  21 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xFA"}, };

static TASMADDR tasm_clts[] = {    /*  22 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x06"}, };

static TASMADDR tasm_cmc[] = {    /*  23 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF5"}, };

static TASMADDR tasm_cmp[] = {    /*  24 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x38\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x38\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x39\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x39\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x39\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x39\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x3A\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x3A\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x3B\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x3B\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x3B\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x3B\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x3C\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x3D\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x3D\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\207\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\207\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\207\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\207\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\207\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\207\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\207\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\207\41"}, };

static TASMADDR tasm_cmpsb[] = {    /*  25 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\1\xA6"}, };

static TASMADDR tasm_cmpsd[] = {    /*  26 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\321\1\xA7"}, };

static TASMADDR tasm_cmpsw[] = {    /*  27 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\320\1\xA7"}, };

static TASMADDR tasm_cmpxchg[] = {    /*  28 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xB0\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xB0\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xB1\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xB1\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xB1\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xB1\101"}, };

static TASMADDR tasm_cmpxchg486[] = {    /*  29 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xA6\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xA6\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA7\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA7\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA7\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA7\101"}, };

static TASMADDR tasm_cmpxchg8b[] = {    /*  30 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\xC7\201"}, };

static TASMADDR tasm_cpuid[] = {    /*  31 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\xA2"}, };

static TASMADDR tasm_cwd[] = {    /*  32 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x99"}, };

static TASMADDR tasm_cwde[] = {    /*  33 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x98"}, };

static TASMADDR tasm_daa[] = {    /*  34 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x27"}, };

static TASMADDR tasm_das[] = {    /*  35 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x2F"}, };

static TASMADDR tasm_dec[] = {    /*  36 */
//           reg16
/*  1 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\10\x48"}, //           reg32
/*  2 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\10\x48"}, //           rm8
/*  3 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xFE\201"}, //           rm16
/*  4 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\201"}, //           rm32
/*  5 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\201"}, };

static TASMADDR tasm_div[] = {    /*  37 */
//           rm8
/*  1 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\206"}, //           rm16
/*  2 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\206"}, //           rm32
/*  3 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\206"}, };

static TASMADDR tasm_emms[] = {    /*  38 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x77"}, };

static TASMADDR tasm_enter[] = {    /*  39 */
//           imm,imm
/*  1 */   { {ASM_IMM, ASM_IMM, 0 }, "\1\xC8\30\25"}, };

static TASMADDR tasm_f2xm1[] = {    /*  40 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF0"}, };

static TASMADDR tasm_fabs[] = {    /*  41 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE1"}, };

static TASMADDR tasm_fadd[] = {    /*  42 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\200"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\200"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xC0"}, //           fpureg
/*  4 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xC0"}, //           fpureg,fpu0
/*  5 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xC0"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xC0"}, };

static TASMADDR tasm_faddp[] = {    /*  43 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xC0"}, //           fpureg,fpu0
/*  2 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xC0"}, };

static TASMADDR tasm_fbld[] = {    /*  44 */
//           mem80
/*  1 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\1\xDF\204"}, //           mem
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDF\204"}, };

static TASMADDR tasm_fbstp[] = {    /*  45 */
//           mem80
/*  1 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\1\xDF\206"}, //           mem
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDF\206"}, };

static TASMADDR tasm_fchs[] = {    /*  46 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE0"}, };

static TASMADDR tasm_fclex[] = {    /*  47 */
//           void
/*  1 */   { {0, 0, 0 }, "\3\x9B\xDB\xE2"}, };

static TASMADDR tasm_fcmovb[] = {    /*  48 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDA\10\xC0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDA\11\xC0"}, };

static TASMADDR tasm_fcmovbe[] = {    /*  49 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDA\10\xD0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDA\11\xD0"}, };

static TASMADDR tasm_fcmove[] = {    /*  50 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDA\10\xC8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDA\11\xC8"}, };

static TASMADDR tasm_fcmovnb[] = {    /*  51 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xC0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xC0"}, };

static TASMADDR tasm_fcmovnbe[] = {    /*  52 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xD0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xD0"}, };

static TASMADDR tasm_fcmovne[] = {    /*  53 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xC8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xC8"}, };

static TASMADDR tasm_fcmovnu[] = {    /*  54 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xD8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xD8"}, };

static TASMADDR tasm_fcmovu[] = {    /*  55 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDA\10\xD8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDA\11\xD8"}, };

static TASMADDR tasm_fcom[] = {    /*  56 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\202"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\202"}, //           fpureg
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xD0"}, //           fpu0,fpureg
/*  4 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xD0"}, };

static TASMADDR tasm_fcomi[] = {    /*  57 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xF0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xF0"}, };

static TASMADDR tasm_fcomip[] = {    /*  58 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDF\10\xF0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDF\11\xF0"}, };

static TASMADDR tasm_fcomp[] = {    /*  59 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\203"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\203"}, //           fpureg
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xD8"}, //           fpu0,fpureg
/*  4 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xD8"}, };

static TASMADDR tasm_fcompp[] = {    /*  60 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDE\xD9"}, };

static TASMADDR tasm_fcos[] = {    /*  61 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFF"}, };

static TASMADDR tasm_fdecstp[] = {    /*  62 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF6"}, };

static TASMADDR tasm_fdisi[] = {    /*  63 */
//           void
/*  1 */   { {0, 0, 0 }, "\3\x9B\xDB\xE1"}, };

static TASMADDR tasm_fdiv[] = {    /*  64 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\206"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\206"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xF8"}, //           fpureg,fpu0
/*  4 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xF8"}, //           fpureg
/*  5 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xF0"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xF0"}, };

static TASMADDR tasm_fdivp[] = {    /*  65 */
//           fpureg,fpu0
/*  1 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xF8"}, //           fpureg
/*  2 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xF8"}, };

static TASMADDR tasm_fdivr[] = {    /*  66 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\207"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\207"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xF0"}, //           fpureg,fpu0
/*  4 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xF0"}, //           fpureg
/*  5 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xF8"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xF8"}, };

static TASMADDR tasm_fdivrp[] = {    /*  67 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xF0"}, //           fpureg,fpu0
/*  2 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xF0"}, };

static TASMADDR tasm_femms[] = {    /*  68 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x0E"}, };

static TASMADDR tasm_feni[] = {    /*  69 */
//           void
/*  1 */   { {0, 0, 0 }, "\3\x9B\xDB\xE0"}, };

static TASMADDR tasm_ffree[] = {    /*  70 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDD\10\xC0"}, };

static TASMADDR tasm_fiadd[] = {    /*  71 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\200"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\200"}, };

static TASMADDR tasm_ficom[] = {    /*  72 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\202"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\202"}, };

static TASMADDR tasm_ficomp[] = {    /*  73 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\203"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\203"}, };

static TASMADDR tasm_fidiv[] = {    /*  74 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\206"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\206"}, };

static TASMADDR tasm_fidivr[] = {    /*  75 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\207"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\207"}, };

static TASMADDR tasm_fild[] = {    /*  76 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDB\200"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDF\200"}, //           mem64
/*  3 */   { {ASM_MEM, 0, 0 }, "\300\1\xDF\205"}, };

static TASMADDR tasm_fimul[] = {    /*  77 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\201"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\201"}, };

static TASMADDR tasm_fincstp[] = {    /*  78 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF7"}, };

static TASMADDR tasm_finit[] = {    /*  79 */
//           void
/*  1 */   { {0, 0, 0 }, "\3\x9B\xDB\xE3"}, };

static TASMADDR tasm_fist[] = {    /*  80 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDB\202"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDF\202"}, };

static TASMADDR tasm_fistp[] = {    /*  81 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDB\203"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDF\203"}, //           mem64
/*  3 */   { {ASM_MEM, 0, 0 }, "\300\1\xDF\207"}, };

static TASMADDR tasm_fisub[] = {    /*  82 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\204"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\204"}, };

static TASMADDR tasm_fisubr[] = {    /*  83 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xDA\205"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\xDE\205"}, };

static TASMADDR tasm_fld[] = {    /*  84 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD9\200"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\200"}, //           mem80
/*  3 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\1\xDB\205"}, //           fpureg
/*  4 */   { {ASM_FPUREG, 0, 0 }, "\1\xD9\10\xC0"}, };

static TASMADDR tasm_fld1[] = {    /*  85 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE8"}, };

static TASMADDR tasm_fldcw[] = {    /*  86 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xD9\205"}, };

static TASMADDR tasm_fldenv[] = {    /*  87 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xD9\204"}, };

static TASMADDR tasm_fldl2e[] = {    /*  88 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xEA"}, };

static TASMADDR tasm_fldl2t[] = {    /*  89 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE9"}, };

static TASMADDR tasm_fldlg2[] = {    /*  90 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xEC"}, };

static TASMADDR tasm_fldln2[] = {    /*  91 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xED"}, };

static TASMADDR tasm_fldpi[] = {    /*  92 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xEB"}, };

static TASMADDR tasm_fldz[] = {    /*  93 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xEE"}, };

static TASMADDR tasm_fmul[] = {    /*  94 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\201"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\201"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xC8"}, //           fpureg,fpu0
/*  4 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xC8"}, //           fpureg
/*  5 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xC8"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xC8"}, };

static TASMADDR tasm_fmulp[] = {    /*  95 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xC8"}, //           fpureg,fpu0
/*  2 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xC8"}, };

static TASMADDR tasm_fnclex[] = {    /*  96 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDB\xE2"}, };

static TASMADDR tasm_fndisi[] = {    /*  97 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDB\xE1"}, };

static TASMADDR tasm_fneni[] = {    /*  98 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDB\xE0"}, };

static TASMADDR tasm_fninit[] = {    /*  99 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDB\xE3"}, };

static TASMADDR tasm_fnop[] = {    /* 100 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xD0"}, };

static TASMADDR tasm_fnsave[] = {    /* 101 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\206"}, };

static TASMADDR tasm_fnstcw[] = {    /* 102 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xD9\207"}, };

static TASMADDR tasm_fnstenv[] = {    /* 103 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xD9\206"}, };

static TASMADDR tasm_fnstsw[] = {    /* 104 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\207"}, //           reg_ax
/*  2 */   { {ASM_AX|ASM_WORD, 0, 0 }, "\2\xDF\xE0"}, };

static TASMADDR tasm_fpatan[] = {    /* 105 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF3"}, };

static TASMADDR tasm_fprem[] = {    /* 106 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF8"}, };

static TASMADDR tasm_fprem1[] = {    /* 107 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF5"}, };

static TASMADDR tasm_fptan[] = {    /* 108 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF2"}, };

static TASMADDR tasm_frndint[] = {    /* 109 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFC"}, };

static TASMADDR tasm_frstor[] = {    /* 110 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\204"}, };

static TASMADDR tasm_fsave[] = {    /* 111 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x9B\xDD\206"}, };

static TASMADDR tasm_fscale[] = {    /* 112 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFD"}, };

static TASMADDR tasm_fsetpm[] = {    /* 113 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDB\xE4"}, };

static TASMADDR tasm_fsin[] = {    /* 114 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFE"}, };

static TASMADDR tasm_fsincos[] = {    /* 115 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFB"}, };

static TASMADDR tasm_fsqrt[] = {    /* 116 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xFA"}, };

static TASMADDR tasm_fst[] = {    /* 117 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD9\202"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\202"}, //           fpureg
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDD\10\xD0"}, };

static TASMADDR tasm_fstcw[] = {    /* 118 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x9B\xD9\207"}, };

static TASMADDR tasm_fstenv[] = {    /* 119 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x9B\xD9\206"}, };

static TASMADDR tasm_fstp[] = {    /* 120 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD9\203"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDD\203"}, //           mem80
/*  3 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\1\xDB\207"}, //           fpureg
/*  4 */   { {ASM_FPUREG, 0, 0 }, "\1\xDD\10\xD8"}, };

static TASMADDR tasm_fstsw[] = {    /* 121 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x9B\xDD\207"}, //           reg_ax
/*  2 */   { {ASM_AX|ASM_WORD, 0, 0 }, "\3\x9B\xDF\xE0"}, };

static TASMADDR tasm_fsub[] = {    /* 122 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\204"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\204"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xE8"}, //           fpureg,fpu0
/*  4 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xE8"}, //           fpureg
/*  5 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xE0"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xE0"}, };

static TASMADDR tasm_fsubp[] = {    /* 123 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xE8"}, //           fpureg,fpu0
/*  2 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xE8"}, };

static TASMADDR tasm_fsubr[] = {    /* 124 */
//           mem32
/*  1 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\300\1\xD8\205"}, //           mem64
/*  2 */   { {ASM_MEM, 0, 0 }, "\300\1\xDC\205"}, //           fpureg|to
/*  3 */   { {ASM_FPUREG, 0, 0 }, "\1\xDC\10\xE0"}, //           fpureg,fpu0
/*  4 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDC\10\xE0"}, //           fpureg
/*  5 */   { {ASM_FPUREG, 0, 0 }, "\1\xD8\10\xE8"}, //           fpu0,fpureg
/*  6 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD8\11\xE8"}, };

static TASMADDR tasm_fsubrp[] = {    /* 125 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDE\10\xE0"}, //           fpureg,fpu0
/*  2 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xDE\10\xE0"}, };

static TASMADDR tasm_ftst[] = {    /* 126 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE4"}, };

static TASMADDR tasm_fucom[] = {    /* 127 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDD\10\xE0"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDD\11\xE0"}, };

static TASMADDR tasm_fucomi[] = {    /* 128 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDB\10\xE8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDB\11\xE8"}, };

static TASMADDR tasm_fucomip[] = {    /* 129 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDF\10\xE8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDF\11\xE8"}, };

static TASMADDR tasm_fucomp[] = {    /* 130 */
//           fpureg
/*  1 */   { {ASM_FPUREG, 0, 0 }, "\1\xDD\10\xE8"}, //           fpu0,fpureg
/*  2 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xDD\11\xE8"}, };

static TASMADDR tasm_fucompp[] = {    /* 131 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xDA\xE9"}, };

static TASMADDR tasm_fxam[] = {    /* 132 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xE5"}, };

static TASMADDR tasm_fxch[] = {    /* 133 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xC9"}, //           fpureg
/*  2 */   { {ASM_FPUREG, 0, 0 }, "\1\xD9\10\xC8"}, //           fpureg,fpu0
/*  3 */   { {ASM_FPUREG, ASM_FPU0, 0 }, "\1\xD9\10\xC8"}, //           fpu0,fpureg
/*  4 */   { {ASM_FPU0, ASM_FPUREG, 0 }, "\1\xD9\11\xC8"}, };

static TASMADDR tasm_fxtract[] = {    /* 134 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF4"}, };

static TASMADDR tasm_fyl2x[] = {    /* 135 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF1"}, };

static TASMADDR tasm_fyl2xp1[] = {    /* 136 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\xD9\xF9"}, };

static TASMADDR tasm_hlt[] = {    /* 137 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF4"}, };

static TASMADDR tasm_ibts[] = {    /* 138 */
//           mem,reg16
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA7\101"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xA7\101"}, //           mem,reg32
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA7\101"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xA7\101"}, };

static TASMADDR tasm_icebp[] = {    /* 139 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF1"}, };

static TASMADDR tasm_idiv[] = {    /* 140 */
//           rm8
/*  1 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\207"}, //           rm16
/*  2 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\207"}, //           rm32
/*  3 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\207"}, };

static TASMADDR tasm_imul[] = {    /* 141 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xAF\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\xAF\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xAF\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\xAF\110"}, //           reg16,mem,imm8
/*  5 */   { {ASM_REG|ASM_WORD, ASM_MEM, ASM_IMM|ASM_BYTE }, "\320\301\1\x6B\110\16"}, //           reg16,reg16,imm8
/*  6 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_IMM|ASM_BYTE }, "\320\301\1\x6B\110\16"}, //           reg16,mem,imm
/*  7 */   { {ASM_REG|ASM_WORD, ASM_MEM, ASM_IMM }, "\320\301\1\x69\110\32"}, //           reg16,reg16,imm
/*  8 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_IMM }, "\320\301\1\x69\110\32"}, //           reg32,mem,imm8
/*  9 */   { {ASM_REG|ASM_DWORD, ASM_MEM, ASM_IMM|ASM_BYTE }, "\321\301\1\x6B\110\16"}, //           reg32,reg32,imm8
/* 10 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_IMM|ASM_BYTE }, "\321\301\1\x6B\110\16"}, //           reg32,mem,imm
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, ASM_IMM }, "\321\301\1\x69\110\42"}, //           reg32,reg32,imm
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_IMM }, "\321\301\1\x69\110\42"}, //           reg16,imm8
/* 13 */   { {ASM_REG|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\1\x6B\100\15"}, //           reg16,imm
/* 14 */   { {ASM_REG|ASM_WORD, ASM_IMM, 0 }, "\320\1\x69\100\31"}, //           reg32,imm8
/* 15 */   { {ASM_REG|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\1\x6B\100\15"}, //           reg32,imm
/* 16 */   { {ASM_REG|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x69\100\41"}, //           rm8
/* 17 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\205"}, //           rm16
/* 18 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\205"}, //           rm32
/* 19 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\205"}, };

static TASMADDR tasm_in[] = {    /* 142 */
//           reg_al,imm
/*  1 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\xE4\25"}, //           reg_ax,imm
/*  2 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\xE5\25"}, //           reg_eax,imm
/*  3 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\xE5\25"}, //           reg_al,reg_dx
/*  4 */   { {ASM_AL|ASM_BYTE, ASM_DX|ASM_WORD, 0 }, "\1\xEC"}, //           reg_ax,reg_dx
/*  5 */   { {ASM_AX|ASM_WORD, ASM_DX|ASM_WORD, 0 }, "\320\1\xED"}, //           reg_eax,reg_dx
/*  6 */   { {ASM_EAX|ASM_DWORD, ASM_DX|ASM_WORD, 0 }, "\321\1\xED"}, };

static TASMADDR tasm_inc[] = {    /* 143 */
//           reg16
/*  1 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\10\x40"}, //           reg32
/*  2 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\10\x40"}, //           rm8
/*  3 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xFE\200"}, //           rm16
/*  4 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\200"}, //           rm32
/*  5 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\200"}, };

static TASMADDR tasm_insb[] = {    /* 144 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x6C"}, };

static TASMADDR tasm_insd[] = {    /* 145 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x6D"}, };

static TASMADDR tasm_insw[] = {    /* 146 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x6D"}, };

static TASMADDR tasm_int[] = {    /* 147 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xCD\24"}, };

static TASMADDR tasm_int01[] = {    /* 148 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF1"}, };

static TASMADDR tasm_int1[] = {    /* 149 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF1"}, };

static TASMADDR tasm_int03[] = {    /* 150 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xCC"}, };

static TASMADDR tasm_int3[] = {    /* 151 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xCC"}, };

static TASMADDR tasm_into[] = {    /* 152 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xCE"}, };

static TASMADDR tasm_invd[] = {    /* 153 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x08"}, };

static TASMADDR tasm_invlpg[] = {    /* 154 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\207"}, };

static TASMADDR tasm_iret[] = {    /* 155 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xCF"}, };

static TASMADDR tasm_iretd[] = {    /* 156 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\xCF"}, };

static TASMADDR tasm_iretw[] = {    /* 157 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\xCF"}, };

static TASMADDR tasm_jcxz[] = {    /* 158 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\320\1\xE3\50"}, };

static TASMADDR tasm_jecxz[] = {    /* 159 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\321\1\xE3\50"}, };

static TASMADDR tasm_jmp[] = {    /* 160 */
//           imm|short
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xEB\50"}, //           imm16
/*  2 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xE9\64"}, //           imm16|near
/*  3 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xE9\64"}, //           imm16|far
/*  4 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xEA\34\37"}, //           imm32
/*  5 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xE9\64"}, //           imm32|near
/*  6 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xE9\64"}, //           imm32|far
/*  7 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xEA\34\37"}, //           imm16:imm
/*  8 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\xEA\31\30"}, //           imm:imm16
/*  9 */   { {ASM_IMM, 0, 0 }, "\320\1\xEA\31\30"}, //           imm32:imm
/* 10 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\xEA\41\30"}, //           imm:imm32
/* 11 */   { {ASM_IMM, 0, 0 }, "\321\1\xEA\41\30"}, //           mem16|far
/* 12 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\205"}, //           mem32|far
/* 13 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\205"}, //           mem16|near
/* 14 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\204"}, //           mem32|near
/* 15 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\204"}, //           reg16
/* 16 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\300\1\xFF\204"}, //           reg32
/* 17 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\204"}, //           mem16
/* 18 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\204"}, //           mem32
/* 19 */   { {ASM_MEM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\204"}, };

static TASMADDR tasm_jo[] = {    /* 161 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jno[] = {    /* 162 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jb[] = {    /* 163 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnb[] = {    /* 164 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jz[] = {    /* 165 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnz[] = {    /* 166 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jbe[] = {    /* 167 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnbe[] = {    /* 168 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_js[] = {    /* 169 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jns[] = {    /* 170 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jp[] = {    /* 171 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnp[] = {    /* 172 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jl[] = {    /* 173 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnl[] = {    /* 174 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_je[] = {    /* 175 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_jnle[] = {    /* 176 */
//           redirect,1
/*  1 */   { {ASM_REDIRECT, 1, 0 }, "Jcc"}, };

static TASMADDR tasm_lahf[] = {    /* 177 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x9F"}, };

static TASMADDR tasm_lar[] = {    /* 178 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\x02\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\x02\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\x02\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\x02\110"}, };

static TASMADDR tasm_lds[] = {    /* 179 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\xC5\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\xC5\110"}, };

static TASMADDR tasm_lea[] = {    /* 180 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x8D\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x8D\110"}, };

static TASMADDR tasm_leave[] = {    /* 181 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xC9"}, };

static TASMADDR tasm_les[] = {    /* 182 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\xC4\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\xC4\110"}, };

static TASMADDR tasm_lfs[] = {    /* 183 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xB4\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xB4\110"}, };

static TASMADDR tasm_lgdt[] = {    /* 184 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\202"}, };

static TASMADDR tasm_lgs[] = {    /* 185 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xB5\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xB5\110"}, };

static TASMADDR tasm_lidt[] = {    /* 186 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\203"}, };

static TASMADDR tasm_lldt[] = {    /* 187 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\202"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\202"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\202"}, };

static TASMADDR tasm_lmsw[] = {    /* 188 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\206"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\2\x0F\x01\206"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\2\x0F\x01\206"}, };

static TASMADDR tasm_loadall[] = {    /* 189 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x07"}, };

static TASMADDR tasm_loadall286[] = {    /* 190 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x05"}, };

static TASMADDR tasm_lock[] = {    /* 191 */
//           prefix
/*  1 */   { {ASM_PREFIX, 0, 0 }, "\xF0"}, };

static TASMADDR tasm_lodsb[] = {    /* 192 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xAC"}, };

static TASMADDR tasm_lodsd[] = {    /* 193 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\xAD"}, };

static TASMADDR tasm_lodsw[] = {    /* 194 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\xAD"}, };

static TASMADDR tasm_loop[] = {    /* 195 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xE2\50"}, //           imm,reg_cx
/*  2 */   { {ASM_IMM, ASM_CX|ASM_WORD, 0 }, "\310\1\xE2\50"}, //           imm,reg_ecx
/*  3 */   { {ASM_IMM, ASM_ECX|ASM_DWORD, 0 }, "\311\1\xE2\50"}, };

static TASMADDR tasm_loope[] = {    /* 196 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xE1\50"}, //           imm,reg_cx
/*  2 */   { {ASM_IMM, ASM_CX|ASM_WORD, 0 }, "\310\1\xE1\50"}, //           imm,reg_ecx
/*  3 */   { {ASM_IMM, ASM_ECX|ASM_DWORD, 0 }, "\311\1\xE1\50"}, };

static TASMADDR tasm_loopne[] = {    /* 197 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xE0\50"}, //           imm,reg_cx
/*  2 */   { {ASM_IMM, ASM_CX|ASM_WORD, 0 }, "\310\1\xE0\50"}, //           imm,reg_ecx
/*  3 */   { {ASM_IMM, ASM_ECX|ASM_DWORD, 0 }, "\311\1\xE0\50"}, };

static TASMADDR tasm_loopnz[] = {    /* 198 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xE0\50"}, //           imm,reg_cx
/*  2 */   { {ASM_IMM, ASM_CX|ASM_WORD, 0 }, "\310\1\xE0\50"}, //           imm,reg_ecx
/*  3 */   { {ASM_IMM, ASM_ECX|ASM_DWORD, 0 }, "\311\1\xE0\50"}, };

static TASMADDR tasm_loopz[] = {    /* 199 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xE1\50"}, //           imm,reg_cx
/*  2 */   { {ASM_IMM, ASM_CX|ASM_WORD, 0 }, "\310\1\xE1\50"}, //           imm,reg_ecx
/*  3 */   { {ASM_IMM, ASM_ECX|ASM_DWORD, 0 }, "\311\1\xE1\50"}, };

static TASMADDR tasm_lsl[] = {    /* 200 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\x03\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\x03\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\x03\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\x03\110"}, };

static TASMADDR tasm_lss[] = {    /* 201 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xB2\110"}, //           reg32,mem
/*  2 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xB2\110"}, };

static TASMADDR tasm_ltr[] = {    /* 202 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\203"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\203"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\203"}, };

static TASMADDR tasm_mov[] = {    /* 203 */
//           mem,reg_dess
/*  1 */   { {ASM_MEM, ASM_DESS|ASM_SR|ASM_WORD, 0 }, "\320\300\1\x8C\101"}, //           mem,reg_fsgs
/*  2 */   { {ASM_MEM, ASM_FSGS|ASM_SR|ASM_WORD, 0 }, "\320\300\1\x8C\101"}, //           reg16,reg_dess
/*  3 */   { {ASM_REG|ASM_WORD, ASM_DESS|ASM_SR|ASM_WORD, 0 }, "\320\300\1\x8C\101"}, //           reg16,reg_fsgs
/*  4 */   { {ASM_REG|ASM_WORD, ASM_FSGS|ASM_SR|ASM_WORD, 0 }, "\320\300\1\x8C\101"}, //           rm32,reg_dess
/*  5 */   { {ASM_RM|ASM_DWORD, ASM_DESS|ASM_SR|ASM_WORD, 0 }, "\321\300\1\x8C\101"}, //           rm32,reg_fsgs
/*  6 */   { {ASM_RM|ASM_DWORD, ASM_FSGS|ASM_SR|ASM_WORD, 0 }, "\321\300\1\x8C\101"}, //           reg_dess,mem
/*  7 */   { {ASM_DESS|ASM_SR|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x8E\110"}, //           reg_fsgs,mem
/*  8 */   { {ASM_FSGS|ASM_SR|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x8E\110"}, //           reg_dess,reg16
/*  9 */   { {ASM_DESS|ASM_SR|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x8E\110"}, //           reg_fsgs,reg16
/* 10 */   { {ASM_FSGS|ASM_SR|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x8E\110"}, //           reg_dess,rm32
/* 11 */   { {ASM_DESS|ASM_SR|ASM_WORD, ASM_RM|ASM_DWORD, 0 }, "\321\301\1\x8E\110"}, //           reg_fsgs,rm32
/* 12 */   { {ASM_FSGS|ASM_SR|ASM_WORD, ASM_RM|ASM_DWORD, 0 }, "\321\301\1\x8E\110"}, //           reg_al,mem_offs
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_MEM, 0 }, "\301\1\xA0\35"}, //           reg_ax,mem_offs
/* 14 */   { {ASM_AX|ASM_WORD, ASM_MEM, 0 }, "\301\320\1\xA1\35"}, //           reg_eax,mem_offs
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_MEM, 0 }, "\301\321\1\xA1\35"}, //           mem_offs,reg_al
/* 16 */   { {ASM_MEM, ASM_AL|ASM_BYTE, 0 }, "\300\1\xA2\34"}, //           mem_offs,reg_ax
/* 17 */   { {ASM_MEM, ASM_AX|ASM_WORD, 0 }, "\300\320\1\xA3\34"}, //           mem_offs,reg_eax
/* 18 */   { {ASM_MEM, ASM_EAX|ASM_DWORD, 0 }, "\300\321\1\xA3\34"}, //           reg32,reg_creg
/* 19 */   { {ASM_REG|ASM_DWORD, ASM_CR|ASM_DWORD, 0 }, "\2\x0F\x20\101"}, //           reg32,reg_dreg
/* 20 */   { {ASM_REG|ASM_DWORD, ASM_DR|ASM_DWORD, 0 }, "\2\x0F\x21\101"}, //           reg32,reg_treg
/* 21 */   { {ASM_REG|ASM_DWORD, ASM_TR|ASM_DWORD, 0 }, "\2\x0F\x24\101"}, //           reg_creg,reg32
/* 22 */   { {ASM_CR|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\2\x0F\x22\110"}, //           reg_dreg,reg32
/* 23 */   { {ASM_DR|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\2\x0F\x23\110"}, //           reg_treg,reg32
/* 24 */   { {ASM_TR|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\2\x0F\x26\110"}, //           mem,reg8
/* 25 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x88\101"}, //           reg8,reg8
/* 26 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x88\101"}, //           mem,reg16
/* 27 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x89\101"}, //           reg16,reg16
/* 28 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x89\101"}, //           mem,reg32
/* 29 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x89\101"}, //           reg32,reg32
/* 30 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x89\101"}, //           reg8,mem
/* 31 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x8A\110"}, //           reg8,reg8
/* 32 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x8A\110"}, //           reg16,mem
/* 33 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x8B\110"}, //           reg16,reg16
/* 34 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x8B\110"}, //           reg32,mem
/* 35 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x8B\110"}, //           reg32,reg32
/* 36 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x8B\110"}, //           reg8,imm
/* 37 */   { {ASM_REG|ASM_BYTE, ASM_IMM, 0 }, "\10\xB0\21"}, //           reg16,imm
/* 38 */   { {ASM_REG|ASM_WORD, ASM_IMM, 0 }, "\320\10\xB8\31"}, //           reg32,imm
/* 39 */   { {ASM_REG|ASM_DWORD, ASM_IMM, 0 }, "\321\10\xB8\41"}, //           rm8,imm
/* 40 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC6\200\21"}, //           rm16,imm
/* 41 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC7\200\31"}, //           rm32,imm
/* 42 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC7\200\41"}, //           mem,imm8
/* 43 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\xC6\200\21"}, //           mem,imm16
/* 44 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\xC7\200\31"}, //           mem,imm32
/* 45 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\xC7\200\41"}, };

static TASMADDR tasm_movd[] = {    /* 204 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x6E\110"}, //           mmxreg,reg32
/*  2 */   { {ASM_REGMMX, ASM_REG|ASM_DWORD, 0 }, "\2\x0F\x6E\110"}, //           mem,mmxreg
/*  3 */   { {ASM_MEM, ASM_REGMMX, 0 }, "\300\2\x0F\x7E\101"}, //           reg32,mmxreg
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REGMMX, 0 }, "\2\x0F\x7E\101"}, };

static TASMADDR tasm_movq[] = {    /* 205 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x6F\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x6F\110"}, //           mem,mmxreg
/*  3 */   { {ASM_MEM, ASM_REGMMX, 0 }, "\300\2\x0F\x7F\101"}, //           mmxreg,mmxreg
/*  4 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x7F\101"}, };

static TASMADDR tasm_movsb[] = {    /* 206 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xA4"}, };

static TASMADDR tasm_movsd[] = {    /* 207 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\xA5"}, };

static TASMADDR tasm_movsw[] = {    /* 208 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\xA5"}, };

static TASMADDR tasm_movsx[] = {    /* 209 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xBE\110"}, //           reg16,reg8
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_BYTE, 0 }, "\320\301\2\x0F\xBE\110"}, //           reg32,rm8
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_RM|ASM_BYTE, 0 }, "\321\301\2\x0F\xBE\110"}, //           reg32,rm16
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_RM|ASM_WORD, 0 }, "\321\301\2\x0F\xBF\110"}, };

static TASMADDR tasm_movzx[] = {    /* 210 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xB6\110"}, //           reg16,reg8
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_BYTE, 0 }, "\320\301\2\x0F\xB6\110"}, //           reg32,rm8
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_RM|ASM_BYTE, 0 }, "\321\301\2\x0F\xB6\110"}, //           reg32,rm16
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_RM|ASM_WORD, 0 }, "\321\301\2\x0F\xB7\110"}, };

static TASMADDR tasm_mul[] = {    /* 211 */
//           rm8
/*  1 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\204"}, //           rm16
/*  2 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\204"}, //           rm32
/*  3 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\204"}, };

static TASMADDR tasm_neg[] = {    /* 212 */
//           rm8
/*  1 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\203"}, //           rm16
/*  2 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\203"}, //           rm32
/*  3 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\203"}, };

static TASMADDR tasm_nop[] = {    /* 213 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x90"}, };

static TASMADDR tasm_not[] = {    /* 214 */
//           rm8
/*  1 */   { {ASM_RM|ASM_BYTE, 0, 0 }, "\300\1\xF6\202"}, //           rm16
/*  2 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xF7\202"}, //           rm32
/*  3 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xF7\202"}, };

static TASMADDR tasm_or[] = {    /* 215 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x08\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x08\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x09\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x09\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x09\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x09\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x0A\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x0A\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x0B\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x0B\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x0B\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x0B\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x0C\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x0D\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x0D\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\201\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\201\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\201\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\201\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\201\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\201\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\201\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\201\41"}, };

static TASMADDR tasm_out[] = {    /* 216 */
//           imm,reg_al
/*  1 */   { {ASM_IMM, ASM_AL|ASM_BYTE, 0 }, "\1\xE6\24"}, //           imm,reg_ax
/*  2 */   { {ASM_IMM, ASM_AX|ASM_WORD, 0 }, "\320\1\xE7\24"}, //           imm,reg_eax
/*  3 */   { {ASM_IMM, ASM_EAX|ASM_DWORD, 0 }, "\321\1\xE7\24"}, //           reg_dx,reg_al
/*  4 */   { {ASM_DX|ASM_WORD, ASM_AL|ASM_BYTE, 0 }, "\1\xEE"}, //           reg_dx,reg_ax
/*  5 */   { {ASM_DX|ASM_WORD, ASM_AX|ASM_WORD, 0 }, "\320\1\xEF"}, //           reg_dx,reg_eax
/*  6 */   { {ASM_DX|ASM_WORD, ASM_EAX|ASM_DWORD, 0 }, "\321\1\xEF"}, };

static TASMADDR tasm_outsb[] = {    /* 217 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x6E"}, };

static TASMADDR tasm_outsd[] = {    /* 218 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x6F"}, };

static TASMADDR tasm_outsw[] = {    /* 219 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x6F"}, };

static TASMADDR tasm_packssdw[] = {    /* 220 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x6B\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x6B\110"}, };

static TASMADDR tasm_packsswb[] = {    /* 221 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x63\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x63\110"}, };

static TASMADDR tasm_packuswb[] = {    /* 222 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x67\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x67\110"}, };

static TASMADDR tasm_paddb[] = {    /* 223 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xFC\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xFC\110"}, };

static TASMADDR tasm_paddd[] = {    /* 224 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xFE\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xFE\110"}, };

static TASMADDR tasm_paddsb[] = {    /* 225 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xEC\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xEC\110"}, };

static TASMADDR tasm_paddsiw[] = {    /* 226 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x51\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x51\110"}, };

static TASMADDR tasm_paddsw[] = {    /* 227 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xED\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xED\110"}, };

static TASMADDR tasm_paddusb[] = {    /* 228 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xDC\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xDC\110"}, };

static TASMADDR tasm_paddusw[] = {    /* 229 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xDD\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xDD\110"}, };

static TASMADDR tasm_paddw[] = {    /* 230 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xFD\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xFD\110"}, };

static TASMADDR tasm_pand[] = {    /* 231 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xDB\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xDB\110"}, };

static TASMADDR tasm_pandn[] = {    /* 232 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xDF\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xDF\110"}, };

static TASMADDR tasm_paveb[] = {    /* 233 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x50\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x50\110"}, };

static TASMADDR tasm_pavgusb[] = {    /* 234 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xBF"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xBF"}, };

static TASMADDR tasm_pcmpeqb[] = {    /* 235 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x74\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x74\110"}, };

static TASMADDR tasm_pcmpeqd[] = {    /* 236 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x76\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x76\110"}, };

static TASMADDR tasm_pcmpeqw[] = {    /* 237 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x75\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x75\110"}, };

static TASMADDR tasm_pcmpgtb[] = {    /* 238 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x64\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x64\110"}, };

static TASMADDR tasm_pcmpgtd[] = {    /* 239 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x66\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x66\110"}, };

static TASMADDR tasm_pcmpgtw[] = {    /* 240 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x65\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x65\110"}, };

static TASMADDR tasm_pdistib[] = {    /* 241 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x54\110"}, };

static TASMADDR tasm_pf2id[] = {    /* 242 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x1D"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x1D"}, };

static TASMADDR tasm_pfacc[] = {    /* 243 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xAE"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xAE"}, };

static TASMADDR tasm_pfadd[] = {    /* 244 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x9E"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x9E"}, };

static TASMADDR tasm_pfcmpeq[] = {    /* 245 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xB0"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xB0"}, };

static TASMADDR tasm_pfcmpge[] = {    /* 246 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x90"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x90"}, };

static TASMADDR tasm_pfcmpgt[] = {    /* 247 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xA0"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xA0"}, };

static TASMADDR tasm_pfmax[] = {    /* 248 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xA4"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xA4"}, };

static TASMADDR tasm_pfmin[] = {    /* 249 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x94"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x94"}, };

static TASMADDR tasm_pfmul[] = {    /* 250 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xB4"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xB4"}, };

static TASMADDR tasm_pfrcp[] = {    /* 251 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x96"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x96"}, };

static TASMADDR tasm_pfrcpit1[] = {    /* 252 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xA6"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xA6"}, };

static TASMADDR tasm_pfrcpit2[] = {    /* 253 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xB6"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xB6"}, };

static TASMADDR tasm_pfrsqit1[] = {    /* 254 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xA7"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xA7"}, };

static TASMADDR tasm_pfrsqrt[] = {    /* 255 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x97"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x97"}, };

static TASMADDR tasm_pfsub[] = {    /* 256 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x9A"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x9A"}, };

static TASMADDR tasm_pfsubr[] = {    /* 257 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\xAA"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\xAA"}, };

static TASMADDR tasm_pi2fd[] = {    /* 258 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\01\x0D"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\01\x0D"}, };

static TASMADDR tasm_pmachriw[] = {    /* 259 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x5E\110"}, };

static TASMADDR tasm_pmaddwd[] = {    /* 260 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF5\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF5\110"}, };

static TASMADDR tasm_pmagw[] = {    /* 261 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x52\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x52\110"}, };

static TASMADDR tasm_pmulhriw[] = {    /* 262 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x5D\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x5D\110"}, };

static TASMADDR tasm_pmulhrwa[] = {    /* 263 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x0F\110\1\xB7"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x0F\110\1\xB7"}, };

static TASMADDR tasm_pmulhrwc[] = {    /* 264 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x59\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x59\110"}, };

static TASMADDR tasm_pmulhw[] = {    /* 265 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xE5\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xE5\110"}, };

static TASMADDR tasm_pmullw[] = {    /* 266 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD5\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD5\110"}, };

static TASMADDR tasm_pmvgezb[] = {    /* 267 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x5C\110"}, };

static TASMADDR tasm_pmvlzb[] = {    /* 268 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x5B\110"}, };

static TASMADDR tasm_pmvnzb[] = {    /* 269 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x5A\110"}, };

static TASMADDR tasm_pmvzb[] = {    /* 270 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x58\110"}, };

static TASMADDR tasm_pop[] = {    /* 271 */
//           reg_fsgs
/*  1 */   { {ASM_FSGS|ASM_SR|ASM_WORD, 0, 0 }, "\1\x0F\4\x81"}, //           reg_dess
/*  2 */   { {ASM_DESS|ASM_SR|ASM_WORD, 0, 0 }, "\4\x07"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\10\x58"}, //           reg32
/*  4 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\10\x58"}, //           rm16
/*  5 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\x8F\200"}, //           rm32
/*  6 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\x8F\200"}, };

static TASMADDR tasm_popa[] = {    /* 272 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x61"}, };

static TASMADDR tasm_popad[] = {    /* 273 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x61"}, };

static TASMADDR tasm_popaw[] = {    /* 274 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x61"}, };

static TASMADDR tasm_popf[] = {    /* 275 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x9D"}, };

static TASMADDR tasm_popfd[] = {    /* 276 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x9D"}, };

static TASMADDR tasm_popfw[] = {    /* 277 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x9D"}, };

static TASMADDR tasm_por[] = {    /* 278 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xEB\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xEB\110"}, };

static TASMADDR tasm_prefetch[] = {    /* 279 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\2\x0F\x0D\200"}, };

static TASMADDR tasm_prefetchw[] = {    /* 280 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\2\x0F\x0D\201"}, };

static TASMADDR tasm_pslld[] = {    /* 281 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF2\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF2\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x72\206\25"}, };

static TASMADDR tasm_psllq[] = {    /* 282 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF3\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF3\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x73\206\25"}, };

static TASMADDR tasm_psllw[] = {    /* 283 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF1\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF1\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x71\206\25"}, };

static TASMADDR tasm_psrad[] = {    /* 284 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xE2\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xE2\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x72\204\25"}, };

static TASMADDR tasm_psraw[] = {    /* 285 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xE1\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xE1\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x71\204\25"}, };

static TASMADDR tasm_psrld[] = {    /* 286 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD2\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD2\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x72\202\25"}, };

static TASMADDR tasm_psrlq[] = {    /* 287 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD3\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD3\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x73\202\25"}, };

static TASMADDR tasm_psrlw[] = {    /* 288 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD1\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD1\110"}, //           mmxreg,imm
/*  3 */   { {ASM_REGMMX, ASM_IMM, 0 }, "\2\x0F\x71\202\25"}, };

static TASMADDR tasm_psubb[] = {    /* 289 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF8\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF8\110"}, };

static TASMADDR tasm_psubd[] = {    /* 290 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xFA\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xFA\110"}, };

static TASMADDR tasm_psubsb[] = {    /* 291 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xE8\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xE8\110"}, };

static TASMADDR tasm_psubsiw[] = {    /* 292 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x55\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x55\110"}, };

static TASMADDR tasm_psubsw[] = {    /* 293 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xE9\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xE9\110"}, };

static TASMADDR tasm_psubusb[] = {    /* 294 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD8\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD8\110"}, };

static TASMADDR tasm_psubusw[] = {    /* 295 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xD9\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xD9\110"}, };

static TASMADDR tasm_psubw[] = {    /* 296 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xF9\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xF9\110"}, };

static TASMADDR tasm_punpckhbw[] = {    /* 297 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x68\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x68\110"}, };

static TASMADDR tasm_punpckhdq[] = {    /* 298 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x6A\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x6A\110"}, };

static TASMADDR tasm_punpckhwd[] = {    /* 299 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x69\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x69\110"}, };

static TASMADDR tasm_punpcklbw[] = {    /* 300 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x60\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x60\110"}, };

static TASMADDR tasm_punpckldq[] = {    /* 301 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x62\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x62\110"}, };

static TASMADDR tasm_punpcklwd[] = {    /* 302 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\x61\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\x61\110"}, };

static TASMADDR tasm_push[] = {    /* 303 */
//           reg_fsgs
/*  1 */   { {ASM_FSGS|ASM_SR|ASM_WORD, 0, 0 }, "\1\x0F\4\x80"}, //           reg_sreg
/*  2 */   { {ASM_SR|ASM_WORD, 0, 0 }, "\4\x06"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\320\10\x50"}, //           reg32
/*  4 */   { {ASM_REG|ASM_DWORD, 0, 0 }, "\321\10\x50"}, //           rm16
/*  5 */   { {ASM_RM|ASM_WORD, 0, 0 }, "\320\300\1\xFF\206"}, //           rm32
/*  6 */   { {ASM_RM|ASM_DWORD, 0, 0 }, "\321\300\1\xFF\206"}, //           imm8
/*  7 */   { {ASM_IMM|ASM_BYTE, 0, 0 }, "\1\x6A\14"}, //           imm16
/*  8 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\x68\30"}, //           imm32
/*  9 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\x68\40"}, };

static TASMADDR tasm_pusha[] = {    /* 304 */
//           void
/*  1 */   { {0, 0, 0 }, "\322\1\x60"}, };

static TASMADDR tasm_pushad[] = {    /* 305 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x60"}, };

static TASMADDR tasm_pushaw[] = {    /* 306 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x60"}, };

static TASMADDR tasm_pushf[] = {    /* 307 */
//           void
/*  1 */   { {0, 0, 0 }, "\322\1\x9C"}, };

static TASMADDR tasm_pushfd[] = {    /* 308 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\x9C"}, };

static TASMADDR tasm_pushfw[] = {    /* 309 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\x9C"}, };

static TASMADDR tasm_pxor[] = {    /* 310 */
//           mmxreg,mem
/*  1 */   { {ASM_REGMMX, ASM_MEM, 0 }, "\301\2\x0F\xEF\110"}, //           mmxreg,mmxreg
/*  2 */   { {ASM_REGMMX, ASM_REGMMX, 0 }, "\2\x0F\xEF\110"}, };

static TASMADDR tasm_rcl[] = {    /* 311 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\202"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\202"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\202\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\202"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\202"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\202\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\202"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\202"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\202\25"}, };

static TASMADDR tasm_rcr[] = {    /* 312 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\203"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\203"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\203\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\203"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\203"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\203\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\203"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\203"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\203\25"}, };

static TASMADDR tasm_rdshr[] = {    /* 313 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x36"}, };

static TASMADDR tasm_rdmsr[] = {    /* 314 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x32"}, };

static TASMADDR tasm_rdpmc[] = {    /* 315 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x33"}, };

static TASMADDR tasm_rdtsc[] = {    /* 316 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x31"}, };

static TASMADDR tasm_resb[] = {    /* 317 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\340"}, };

static TASMADDR tasm_ret[] = {    /* 318 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xC2\30"}, //           void
/*  2 */   { {0, 0, 0 }, "\1\xC3"}, };

static TASMADDR tasm_retf[] = {    /* 319 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xCA\30"}, //           void
/*  2 */   { {0, 0, 0 }, "\1\xCB"}, };

static TASMADDR tasm_retn[] = {    /* 320 */
//           imm
/*  1 */   { {ASM_IMM, 0, 0 }, "\1\xC2\30"}, //           void
/*  2 */   { {0, 0, 0 }, "\1\xC3"}, };

static TASMADDR tasm_rol[] = {    /* 321 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\200"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\200"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\200\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\200"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\200"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\200\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\200"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\200"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\200\25"}, };

static TASMADDR tasm_ror[] = {    /* 322 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\201"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\201"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\201\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\201"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\201"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\201\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\201"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\201"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\201\25"}, };

static TASMADDR tasm_rsdc[] = {    /* 323 */
//           reg_sreg,mem80
/*  1 */   { {ASM_SR|ASM_WORD, ASM_MEM|ASM_BYTE, 0 }, "\301\2\x0F\x79\101"}, };

static TASMADDR tasm_rsldt[] = {    /* 324 */
//           mem80
/*  1 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\2\x0F\x7B\200"}, };

static TASMADDR tasm_rsm[] = {    /* 325 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\xAA"}, };

static TASMADDR tasm_sahf[] = {    /* 326 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x9E"}, };

static TASMADDR tasm_sal[] = {    /* 327 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\204"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\204"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\204\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\204"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\204"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\204\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\204"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\204"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\204\25"}, };

static TASMADDR tasm_salc[] = {    /* 328 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xD6"}, };

static TASMADDR tasm_sar[] = {    /* 329 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\207"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\207"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\207\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\207"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\207"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\207\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\207"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\207"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\207\25"}, };

static TASMADDR tasm_sbb[] = {    /* 330 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x18\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x18\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x19\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x19\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x19\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x19\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x1A\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x1A\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x1B\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x1B\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x1B\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x1B\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x1C\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x1D\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x1D\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\203\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\203\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\203\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\203\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\203\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\203\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\203\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\203\41"}, };

static TASMADDR tasm_scasb[] = {    /* 331 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\1\xAE"}, };

static TASMADDR tasm_scasd[] = {    /* 332 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\321\1\xAF"}, };

static TASMADDR tasm_scasw[] = {    /* 333 */
//           void
/*  1 */   { {0, 0, 0 }, "\332\320\1\xAF"}, };

static TASMADDR tasm_sgdt[] = {    /* 334 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\200"}, };

static TASMADDR tasm_shl[] = {    /* 335 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\204"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\204"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\204\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\204"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\204"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\204\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\204"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\204"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\204\25"}, };

static TASMADDR tasm_shld[] = {    /* 336 */
//           mem,reg16,imm
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, ASM_IMM }, "\300\320\2\x0F\xA4\101\26"}, //           reg16,reg16,imm
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_IMM }, "\300\320\2\x0F\xA4\101\26"}, //           mem,reg32,imm
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, ASM_IMM }, "\300\321\2\x0F\xA4\101\26"}, //           reg32,reg32,imm
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_IMM }, "\300\321\2\x0F\xA4\101\26"}, //           mem,reg16,reg_cl
/*  5 */   { {ASM_MEM, ASM_REG|ASM_WORD, ASM_CL|ASM_BYTE }, "\300\320\2\x0F\xA5\101"}, //           reg16,reg16,reg_cl
/*  6 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_CL|ASM_BYTE }, "\300\320\2\x0F\xA5\101"}, //           mem,reg32,reg_cl
/*  7 */   { {ASM_MEM, ASM_REG|ASM_DWORD, ASM_CL|ASM_BYTE }, "\300\321\2\x0F\xA5\101"}, //           reg32,reg32,reg_cl
/*  8 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_CL|ASM_BYTE }, "\300\321\2\x0F\xA5\101"}, };

static TASMADDR tasm_shr[] = {    /* 337 */
//           rm8,unity
/*  1 */   { {ASM_RM|ASM_BYTE, ASM_1, 0 }, "\300\1\xD0\205"}, //           rm8,reg_cl
/*  2 */   { {ASM_RM|ASM_BYTE, ASM_CL|ASM_BYTE, 0 }, "\300\1\xD2\205"}, //           rm8,imm
/*  3 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xC0\205\25"}, //           rm16,unity
/*  4 */   { {ASM_RM|ASM_WORD, ASM_1, 0 }, "\320\300\1\xD1\205"}, //           rm16,reg_cl
/*  5 */   { {ASM_RM|ASM_WORD, ASM_CL|ASM_BYTE, 0 }, "\320\300\1\xD3\205"}, //           rm16,imm
/*  6 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xC1\205\25"}, //           rm32,unity
/*  7 */   { {ASM_RM|ASM_DWORD, ASM_1, 0 }, "\321\300\1\xD1\205"}, //           rm32,reg_cl
/*  8 */   { {ASM_RM|ASM_DWORD, ASM_CL|ASM_BYTE, 0 }, "\321\300\1\xD3\205"}, //           rm32,imm
/*  9 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xC1\205\25"}, };

static TASMADDR tasm_shrd[] = {    /* 338 */
//           mem,reg16,imm
/*  1 */   { {ASM_MEM, ASM_REG|ASM_WORD, ASM_IMM }, "\300\320\2\x0F\xAC\101\26"}, //           reg16,reg16,imm
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_IMM }, "\300\320\2\x0F\xAC\101\26"}, //           mem,reg32,imm
/*  3 */   { {ASM_MEM, ASM_REG|ASM_DWORD, ASM_IMM }, "\300\321\2\x0F\xAC\101\26"}, //           reg32,reg32,imm
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_IMM }, "\300\321\2\x0F\xAC\101\26"}, //           mem,reg16,reg_cl
/*  5 */   { {ASM_MEM, ASM_REG|ASM_WORD, ASM_CL|ASM_BYTE }, "\300\320\2\x0F\xAD\101"}, //           reg16,reg16,reg_cl
/*  6 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, ASM_CL|ASM_BYTE }, "\300\320\2\x0F\xAD\101"}, //           mem,reg32,reg_cl
/*  7 */   { {ASM_MEM, ASM_REG|ASM_DWORD, ASM_CL|ASM_BYTE }, "\300\321\2\x0F\xAD\101"}, //           reg32,reg32,reg_cl
/*  8 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, ASM_CL|ASM_BYTE }, "\300\321\2\x0F\xAD\101"}, };

static TASMADDR tasm_sidt[] = {    /* 339 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\201"}, };

static TASMADDR tasm_sldt[] = {    /* 340 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\200"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\200"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\200"}, };

static TASMADDR tasm_smi[] = {    /* 341 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF1"}, };

static TASMADDR tasm_smint[] = {    /* 342 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x38"}, };

static TASMADDR tasm_smsw[] = {    /* 343 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\2\x0F\x01\204"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\2\x0F\x01\204"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\2\x0F\x01\204"}, };

static TASMADDR tasm_stc[] = {    /* 344 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xF9"}, };

static TASMADDR tasm_std[] = {    /* 345 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xFD"}, };

static TASMADDR tasm_sti[] = {    /* 346 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xFB"}, };

static TASMADDR tasm_stosb[] = {    /* 347 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xAA"}, };

static TASMADDR tasm_stosd[] = {    /* 348 */
//           void
/*  1 */   { {0, 0, 0 }, "\321\1\xAB"}, };

static TASMADDR tasm_stosw[] = {    /* 349 */
//           void
/*  1 */   { {0, 0, 0 }, "\320\1\xAB"}, };

static TASMADDR tasm_str[] = {    /* 350 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\201"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\201"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\201"}, };

static TASMADDR tasm_sub[] = {    /* 351 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x28\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x28\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x29\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x29\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x29\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x29\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x2A\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x2A\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x2B\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x2B\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x2B\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x2B\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x2C\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x2D\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x2D\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\205\21"}, //           rm16,imm8
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\205\15"}, //           rm32,imm8
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\205\15"}, //           rm16,imm
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\205\31"}, //           rm32,imm
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\205\41"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\205\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\205\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\205\41"}, };

static TASMADDR tasm_svdc[] = {    /* 352 */
//           mem80,reg_sreg
/*  1 */   { {ASM_MEM|ASM_BYTE, ASM_SR|ASM_WORD, 0 }, "\300\2\x0F\x78\101"}, };

static TASMADDR tasm_svldt[] = {    /* 353 */
//           mem80
/*  1 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\2\x0F\x7A\200"}, };

static TASMADDR tasm_svts[] = {    /* 354 */
//           mem80
/*  1 */   { {ASM_MEM|ASM_BYTE, 0, 0 }, "\300\2\x0F\x7C\200"}, };

static TASMADDR tasm_syscall[] = {    /* 355 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x05"}, };

static TASMADDR tasm_sysenter[] = {    /* 356 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x34"}, };

static TASMADDR tasm_sysexit[] = {    /* 357 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x36"}, };

static TASMADDR tasm_sysret[] = {    /* 358 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x07"}, };

static TASMADDR tasm_test[] = {    /* 359 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x84\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x84\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x85\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x85\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x85\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x85\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x84\110"}, //           reg16,mem
/*  8 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x85\110"}, //           reg32,mem
/*  9 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x85\110"}, //           reg_al,imm
/* 10 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\xA8\21"}, //           reg_ax,imm
/* 11 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\xA9\31"}, //           reg_eax,imm
/* 12 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\xA9\41"}, //           rm8,imm
/* 13 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\xF6\200\21"}, //           rm16,imm
/* 14 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\xF7\200\31"}, //           rm32,imm
/* 15 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\xF7\200\41"}, //           mem,imm8
/* 16 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\xF6\200\21"}, //           mem,imm16
/* 17 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\xF7\200\31"}, //           mem,imm32
/* 18 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\xF7\200\41"}, };

static TASMADDR tasm_ud1[] = {    /* 360 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\xB9"}, };

static TASMADDR tasm_ud2[] = {    /* 361 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x0B"}, };

static TASMADDR tasm_umov[] = {    /* 362 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\x10\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\x10\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\x11\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\x11\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\x11\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\x11\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\2\x0F\x12\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\2\x0F\x12\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\x13\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\x13\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\x13\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\x13\110"}, };

static TASMADDR tasm_verr[] = {    /* 363 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\204"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\204"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\204"}, };

static TASMADDR tasm_verw[] = {    /* 364 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\17\205"}, //           mem16
/*  2 */   { {ASM_MEM|ASM_WORD, 0, 0 }, "\300\1\x0F\17\205"}, //           reg16
/*  3 */   { {ASM_REG|ASM_WORD, 0, 0 }, "\300\1\x0F\17\205"}, };

static TASMADDR tasm_wait[] = {    /* 365 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\x9B"}, };

static TASMADDR tasm_wbinvd[] = {    /* 366 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x09"}, };

static TASMADDR tasm_wrshr[] = {    /* 367 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x37"}, };

static TASMADDR tasm_wrmsr[] = {    /* 368 */
//           void
/*  1 */   { {0, 0, 0 }, "\2\x0F\x30"}, };

static TASMADDR tasm_xadd[] = {    /* 369 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xC0\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\2\x0F\xC0\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xC1\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\2\x0F\xC1\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xC1\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\2\x0F\xC1\101"}, };

static TASMADDR tasm_xbts[] = {    /* 370 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\2\x0F\xA6\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\2\x0F\xA6\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\2\x0F\xA6\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\2\x0F\xA6\110"}, };

static TASMADDR tasm_xchg[] = {    /* 371 */
//           reg_ax,reg16
/*  1 */   { {ASM_AX|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\11\x90"}, //           reg_eax,reg32
/*  2 */   { {ASM_EAX|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\11\x90"}, //           reg16,reg_ax
/*  3 */   { {ASM_REG|ASM_WORD, ASM_AX|ASM_WORD, 0 }, "\320\10\x90"}, //           reg32,reg_eax
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_EAX|ASM_DWORD, 0 }, "\321\10\x90"}, //           reg8,mem
/*  5 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x86\110"}, //           reg8,reg8
/*  6 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x86\110"}, //           reg16,mem
/*  7 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x87\110"}, //           reg16,reg16
/*  8 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x87\110"}, //           reg32,mem
/*  9 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x87\110"}, //           reg32,reg32
/* 10 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x87\110"}, //           mem,reg8
/* 11 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x86\101"}, //           reg8,reg8
/* 12 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x86\101"}, //           mem,reg16
/* 13 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x87\101"}, //           reg16,reg16
/* 14 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x87\101"}, //           mem,reg32
/* 15 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x87\101"}, //           reg32,reg32
/* 16 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x87\101"}, };

static TASMADDR tasm_xlatb[] = {    /* 372 */
//           void
/*  1 */   { {0, 0, 0 }, "\1\xD7"}, };

static TASMADDR tasm_xor[] = {    /* 373 */
//           mem,reg8
/*  1 */   { {ASM_MEM, ASM_REG|ASM_BYTE, 0 }, "\300\1\x30\101"}, //           reg8,reg8
/*  2 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\300\1\x30\101"}, //           mem,reg16
/*  3 */   { {ASM_MEM, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x31\101"}, //           reg16,reg16
/*  4 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\300\1\x31\101"}, //           mem,reg32
/*  5 */   { {ASM_MEM, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x31\101"}, //           reg32,reg32
/*  6 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\300\1\x31\101"}, //           reg8,mem
/*  7 */   { {ASM_REG|ASM_BYTE, ASM_MEM, 0 }, "\301\1\x32\110"}, //           reg8,reg8
/*  8 */   { {ASM_REG|ASM_BYTE, ASM_REG|ASM_BYTE, 0 }, "\301\1\x32\110"}, //           reg16,mem
/*  9 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x33\110"}, //           reg16,reg16
/* 10 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x33\110"}, //           reg32,mem
/* 11 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x33\110"}, //           reg32,reg32
/* 12 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x33\110"}, //           reg_al,imm
/* 13 */   { {ASM_AL|ASM_BYTE, ASM_IMM, 0 }, "\1\x34\21"}, //           reg_ax,imm
/* 14 */   { {ASM_AX|ASM_WORD, ASM_IMM, 0 }, "\320\1\x35\31"}, //           reg_eax,imm
/* 15 */   { {ASM_EAX|ASM_DWORD, ASM_IMM, 0 }, "\321\1\x35\41"}, //           rm8,imm
/* 16 */   { {ASM_RM|ASM_BYTE, ASM_IMM, 0 }, "\300\1\x80\206\21"}, //           rm16,imm
/* 17 */   { {ASM_RM|ASM_WORD, ASM_IMM, 0 }, "\320\300\1\x81\206\31"}, //           rm32,imm
/* 18 */   { {ASM_RM|ASM_DWORD, ASM_IMM, 0 }, "\321\300\1\x81\206\41"}, //           rm16,imm8
/* 19 */   { {ASM_RM|ASM_WORD, ASM_IMM|ASM_BYTE, 0 }, "\320\300\1\x83\206\15"}, //           rm32,imm8
/* 20 */   { {ASM_RM|ASM_DWORD, ASM_IMM|ASM_BYTE, 0 }, "\321\300\1\x83\206\15"}, //           mem,imm8
/* 21 */   { {ASM_MEM, ASM_IMM|ASM_BYTE, 0 }, "\300\1\x80\206\21"}, //           mem,imm16
/* 22 */   { {ASM_MEM, ASM_IMM|ASM_WORD, 0 }, "\320\300\1\x81\206\31"}, //           mem,imm32
/* 23 */   { {ASM_MEM, ASM_IMM|ASM_DWORD, 0 }, "\321\300\1\x81\206\41"}, };

static TASMADDR tasm_cmovcc[] = {    /* 374 */
//           reg16,mem
/*  1 */   { {ASM_REG|ASM_WORD, ASM_MEM, 0 }, "\320\301\1\x0F\330\x40\110"}, //           reg16,reg16
/*  2 */   { {ASM_REG|ASM_WORD, ASM_REG|ASM_WORD, 0 }, "\320\301\1\x0F\330\x40\110"}, //           reg32,mem
/*  3 */   { {ASM_REG|ASM_DWORD, ASM_MEM, 0 }, "\321\301\1\x0F\330\x40\110"}, //           reg32,reg32
/*  4 */   { {ASM_REG|ASM_DWORD, ASM_REG|ASM_DWORD, 0 }, "\321\301\1\x0F\330\x40\110"}, };

static TASMADDR tasm_jcc[] = {    /* 375 */
//           imm16|near
/*  1 */   { {ASM_IMM|ASM_WORD, 0, 0 }, "\320\1\x0F\330\x80\64"}, //           imm32|near
/*  2 */   { {ASM_IMM|ASM_DWORD, 0, 0 }, "\321\1\x0F\330\x80\64"}, //           imm
/*  3 */   { {ASM_IMM, 0, 0 }, "\330\x70\50"}, //           imm|short
/*  4 */   { {ASM_IMM, 0, 0 }, "\330\x70\50"}, };

static TASMADDR tasm_setcc[] = {    /* 376 */
//           mem
/*  1 */   { {ASM_MEM, 0, 0 }, "\300\1\x0F\330\x90\200"}, //           reg8
/*  2 */   { {ASM_REG|ASM_BYTE, 0, 0 }, "\300\1\x0F\330\x90\200"}, };

