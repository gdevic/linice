
void LineInput(char *sLine);

main()
{
    char sLine[256];
    LineInput(sLine);
}

static char sLine[256];

void DisplayRegs()
{
    if( deb.wr == FALSE )
        return;

    sprintf(sLine, "EAX=%08X   EBX=%08X   ECX=%08X   EDX=%08X   ESI=%08X\n",
            deb.pRegs->eax, deb.pRegs->ebx, deb.pRegs->ecx, deb.pRegs->edx, deb.pRegs->esi );
    PrintOut(sLine);
    sprintf(sLine, "EDI=%08X   EBP=%08X   ESP=%08X   EIP=%08X   %c %c %c %c %c %c %c %c\n",
            deb.pRegs->edi, deb.pRegs->ebp, deb.pRegs->esp, deb.pRegs->eip,
            (deb.pRegs->eflags & EFLAGS_OF)? 'O' : 'o',
            (deb.pRegs->eflags & EFLAGS_DF)? 'D' : 'd',
            (deb.pRegs->eflags & EFLAGS_IF)? 'I' : 'i',
            (deb.pRegs->eflags & EFLAGS_SF)? 'S' : 's',
            (deb.pRegs->eflags & EFLAGS_ZF)? 'Z' : 'z',
            (deb.pRegs->eflags & EFLAGS_AF)? 'A' : 'a',
            (deb.pRegs->eflags & EFLAGS_PF)? 'P' : 'p',
            (deb.pRegs->eflags & EFLAGS_CF)? 'C' : 'c' );
    PrintOut(sLine);
    sprintf(sLine, "CS=%04X   DS=%04X   SS=%04X   ES=%04X   FS=%04X   GS=%04X   \n",
            deb.pRegs->cs, deb.pRegs->ds, deb.pRegs->ss, deb.pRegs->es, deb.pRegs->fs, deb.pRegs->gs );
    PrintOut(sLine);
}

void DisplayLocals()
{
    if( deb.wl == FALSE )
        return;
}

void DisplayWatch()
{
    if( deb.ww == FALSE )
        return;
}

void DisplayData()
{
    if( deb.dd == FALSE )
        return;

    PrintOutStrike("");     // Single horizontal line divider

}

void DisplayCode()
{
    if( deb.wc == FALSE )
        return;

    PrintOutStrike("");     // Single horizontal line divider
}

void DisplayStack()
{
    if( deb.ws == FALSE )
        return;

    PrintOutStrike("");     // Single horizontal line divider
}


typedef struct
{
    char *pCmd;
    char *pHelp;
    char *pSyntax;
} TCommand;

TCommand cmd[] = {
{ "A", "", "A [Address]" },
{ "ADDR", "Display/change address Contexts", "ADDR [context-handle | task | *]" },
{ "ALTKEY", "Set key sequence to invoke window", "ALTKEY [ALT letter | CTRL letter]" },
{ "ALTSCR", "Change to alternate display", "ALTSCR [MONO | VGA | OFF]" },
{ "ANSWER", "Auto-answer and redirect console to modem", "ANSWER [ON [com-port] [baud-rate] [i=init] | OFF]" },
{ "BC", "Clear breakpoint", "BC list | *" },
{ "BD", "Disable breakpoint", "BD list | *" },
{ "BE", "Enable breakpoint", "BE list | *" },
{ "BH", "Breakpoint history", "Breakpoint history" },
{ "BL", "List current breakpoints", "List current breakpoints" },
{ "BMSG", "Breakpoint on Windows message", "BMSG hWnd [L] [begin-msg [end-msg]] [IF expression]" },
{ "BPE", "Edit breakpoint", "BPE breakpoint number" },
{ "BPINT", "Breakpoint on interrupt", "BPINT interrupt-number [IF expression] [DO bp-action]" },
{ "BPIO", "Breakpoint on I/O port access", "BPIO [-h] port [R|W|RW] [debug register] [IF expression] [DO bp-action]" },
{ "BPM", "Breakpoint on memory access", "BPM[size] address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]" },
{ "BPMB", "Breakpoint on memory access", "BPMB address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]" },
{ "BPMD", "Breakpoint on memory access", "BPMD address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]" },
{ "BPMW", "Breakpoint on memory access", "BPMW address [R|W|RW|X] [debug register] [IF expression] [DO bp-action]" },
{ "BPR", "Breakpoint on memory range", "BPR address address [R|W|RW|T|TW] [IF expression] [DO bp-action]" },
{ "BPRW", "Breakpoint on memory range", "BPRW module name | code selector [R|W|RW|T|TW] [IF expression] [DO bp-action]" },
{ "BPT", "Use breakpoint as a template", "BPT breakpoint number" },
{ "BPX", "Breakpoint on execution", "BPX address [IF expression] [DO bp-action]" },
{ "C", "", "C address1 L length address2" },
{ "CLS", "Clear window", "Clear window" },
{ "CLASS", "Display window class information", "CLASS [-x] [task name]" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
{ "", "", "" },
}

void LineInput(char *sLine)
{
    char c = '\0';

    DisplayRegs();
    DisplayLocals();
    DisplayWatch();
    DisplayData();
    DisplayCode();
    DisplayStack();

    PrintOutHelpLine("     Enter a command (H for help)");

    while( c!='\n' )
    {
        c = GetKey();


    }
}
