#line 1 "dalf_test.c"           //work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
//cannot set breakpoints if this directive is used:
//info: http://www.microchip.com/forums/m105540-print.aspx
//uncomment only when breakpoints are no longer needed
/******************************************************************************
*******************************************************************************
**
**
**                  test.c - Debug and test module of the ROVIM T2D and
**                          Dalf-1F modules.
**
**      This module implements the debug information reporting and testing
**      funcionality of the original Dalf-1F firmware and its ROVIM T2D
**      extension.
**      It covers status reports, warning and error information and
**      unitary and integrated test procedures.
**
**      This code was designed originally for the Dalf-1F motor control
**      board, the brain of the T2D module.
**      Original Dalf-1F firmware revision was 1.73.
**      See Dalf-1F owner's manual and the ROVIM T2D documentation for more 
**      details.
**
**          The ROVIM Project
*******************************************************************************
******************************************************************************/

#include "dalf.h"
#include "dalf_test.h"
#include "rovim_t2d.h"
#include <stdio.h>
#include <string.h>

#pragma romdata TestGPIOsDescription
rom const IOPinDescription OutNonInvNonPull[]={
    { "A",  { J5,    1,      OUT,    OFF,   OFF }},
    { "B",  { J5,    2,      OUT,    OFF,   OFF }},
    { "C",  { J5,    3,      OUT,    OFF,   OFF }},
    { "D",  { J5,    4,      OUT,    OFF,   OFF }},
    { "E",  { J5,    5,      OUT,    OFF,   OFF }},
    { "F",  { J5,    6,      OUT,    OFF,   OFF }},
    { "G",  { J5,    7,      OUT,    OFF,   OFF }},
    { "H",  { J5,    8,      OUT,    OFF,   OFF }},
    { "I",  { J5,    9,      OUT,    OFF,   OFF }},
    { "J",  { J5,    10,     OUT,    OFF,   OFF }},
    { "K",  { J5,    11,     OUT,    OFF,   OFF }},
    { "L",  { J5,    12,     OUT,    OFF,   OFF }},
    { "M",  { J5,    13,     OUT,    OFF,   OFF }},
    { "N",  { J5,    14,     OUT,    OFF,   OFF }},
    { "O",  { J5,    15,     OUT,    OFF,   OFF }},
    { "P",  { J5,    16,     OUT,    OFF,   OFF }},
    { "Q",  { J6,    1,      OUT,    OFF,   OFF }},
    { "R",  { J6,    2,      OUT,    OFF,   OFF }},
    { "S",  { J6,    3,      OUT,    OFF,   OFF }},
    { "T",  { J6,    4,      OUT,    OFF,   OFF }},
    { "U",  { J6,    5,      OUT,    OFF,   OFF }},
    { "V",  { J6,    6,      OUT,    OFF,   OFF }},
    { "W",  { J6,    7,      OUT,    OFF,   OFF }},
    { "X",  { J6,    8,      OUT,    OFF,   OFF }},
    { "Y",  { J6,    9,      OUT,    OFF,   OFF }},
    { "Z",  { J6,    10,     OUT,    OFF,   OFF }},
    { "AA", { J6,    11,     OUT,    OFF,   OFF }},
    { "AB", { J6,    12,     OUT,    OFF,   OFF }},
    { "AC", { J6,    13,     OUT,    OFF,   OFF }},
    { "AD", { J6,    14,     OUT,    OFF,   OFF }},
    { "AE", { J6,    15,     OUT,    OFF,   OFF }},
    { "AF", { J6,    16,     OUT,    OFF,   OFF }}
};

rom const IOPinDescription OutNonInvPull[]={
    { "A",  { J5,    1,      OUT,    ON,     OFF }},
    { "B",  { J5,    2,      OUT,    ON,     OFF }},
    { "C",  { J5,    3,      OUT,    ON,     OFF }},
    { "D",  { J5,    4,      OUT,    ON,     OFF }},
    { "E",  { J5,    5,      OUT,    ON,     OFF }},
    { "F",  { J5,    6,      OUT,    ON,     OFF }},
    { "G",  { J5,    7,      OUT,    ON,     OFF }},
    { "H",  { J5,    8,      OUT,    ON,     OFF }},
    { "I",  { J5,    9,      OUT,    ON,     OFF }},
    { "J",  { J5,    10,     OUT,    ON,     OFF }},
    { "K",  { J5,    11,     OUT,    ON,     OFF }},
    { "L",  { J5,    12,     OUT,    ON,     OFF }},
    { "M",  { J5,    13,     OUT,    ON,     OFF }},
    { "N",  { J5,    14,     OUT,    ON,     OFF }},
    { "O",  { J5,    15,     OUT,    ON,     OFF }},
    { "P",  { J5,    16,     OUT,    ON,     OFF }},
    { "Q",  { J6,    1,      OUT,    ON,     OFF }},
    { "R",  { J6,    2,      OUT,    ON,     OFF }},
    { "S",  { J6,    3,      OUT,    ON,     OFF }},
    { "T",  { J6,    4,      OUT,    ON,     OFF }},
    { "U",  { J6,    5,      OUT,    ON,     OFF }},
    { "V",  { J6,    6,      OUT,    ON,     OFF }},
    { "W",  { J6,    7,      OUT,    ON,     OFF }},
    { "X",  { J6,    8,      OUT,    ON,     OFF }},
    { "Y",  { J6,    9,      OUT,    ON,     OFF }},
    { "Z",  { J6,    10,     OUT,    ON,     OFF }},
    { "AA", { J6,    11,     OUT,    ON,     OFF }},
    { "AB", { J6,    12,     OUT,    ON,     OFF }},
    { "AC", { J6,    13,     OUT,    ON,     OFF }},
    { "AD", { J6,    14,     OUT,    ON,     OFF }},
    { "AE", { J6,    15,     OUT,    ON,     OFF }},
    { "AF", { J6,    16,     OUT,    ON,     OFF }}
};

rom const IOPinDescription OutInvNonPull[]={
    { "A",  { J5,    1,      OUT,    OFF,   ON }},
    { "B",  { J5,    2,      OUT,    OFF,   ON }},
    { "C",  { J5,    3,      OUT,    OFF,   ON }},
    { "D",  { J5,    4,      OUT,    OFF,   ON }},
    { "E",  { J5,    5,      OUT,    OFF,   ON }},
    { "F",  { J5,    6,      OUT,    OFF,   ON }},
    { "G",  { J5,    7,      OUT,    OFF,   ON }},
    { "H",  { J5,    8,      OUT,    OFF,   ON }},
    { "I",  { J5,    9,      OUT,    OFF,   ON }},
    { "J",  { J5,    10,     OUT,    OFF,   ON }},
    { "K",  { J5,    11,     OUT,    OFF,   ON }},
    { "L",  { J5,    12,     OUT,    OFF,   ON }},
    { "M",  { J5,    13,     OUT,    OFF,   ON }},
    { "N",  { J5,    14,     OUT,    OFF,   ON }},
    { "O",  { J5,    15,     OUT,    OFF,   ON }},
    { "P",  { J5,    16,     OUT,    OFF,   ON }},
    { "Q",  { J6,    1,      OUT,    OFF,   ON }},
    { "R",  { J6,    2,      OUT,    OFF,   ON }},
    { "S",  { J6,    3,      OUT,    OFF,   ON }},
    { "T",  { J6,    4,      OUT,    OFF,   ON }},
    { "U",  { J6,    5,      OUT,    OFF,   ON }},
    { "V",  { J6,    6,      OUT,    OFF,   ON }},
    { "W",  { J6,    7,      OUT,    OFF,   ON }},
    { "X",  { J6,    8,      OUT,    OFF,   ON }},
    { "Y",  { J6,    9,      OUT,    OFF,   ON }},
    { "Z",  { J6,    10,     OUT,    OFF,   ON }},
    { "AA", { J6,    11,     OUT,    OFF,   ON }},
    { "AB", { J6,    12,     OUT,    OFF,   ON }},
    { "AC", { J6,    13,     OUT,    OFF,   ON }},
    { "AD", { J6,    14,     OUT,    OFF,   ON }},
    { "AE", { J6,    15,     OUT,    OFF,   ON }},
    { "AF", { J6,    16,     OUT,    OFF,   ON }}
};

rom const IOPinDescription OutInvPull[]={
    { "A",  { J5,    1,      OUT,    ON,     ON }},
    { "B",  { J5,    2,      OUT,    ON,     ON }},
    { "C",  { J5,    3,      OUT,    ON,     ON }},
    { "D",  { J5,    4,      OUT,    ON,     ON }},
    { "E",  { J5,    5,      OUT,    ON,     ON }},
    { "F",  { J5,    6,      OUT,    ON,     ON }},
    { "G",  { J5,    7,      OUT,    ON,     ON }},
    { "H",  { J5,    8,      OUT,    ON,     ON }},
    { "I",  { J5,    9,      OUT,    ON,     ON }},
    { "J",  { J5,    10,     OUT,    ON,     ON }},
    { "K",  { J5,    11,     OUT,    ON,     ON }},
    { "L",  { J5,    12,     OUT,    ON,     ON }},
    { "M",  { J5,    13,     OUT,    ON,     ON }},
    { "N",  { J5,    14,     OUT,    ON,     ON }},
    { "O",  { J5,    15,     OUT,    ON,     ON }},
    { "P",  { J5,    16,     OUT,    ON,     ON }},
    { "Q",  { J6,    1,      OUT,    ON,     ON }},
    { "R",  { J6,    2,      OUT,    ON,     ON }},
    { "S",  { J6,    3,      OUT,    ON,     ON }},
    { "T",  { J6,    4,      OUT,    ON,     ON }},
    { "U",  { J6,    5,      OUT,    ON,     ON }},
    { "V",  { J6,    6,      OUT,    ON,     ON }},
    { "W",  { J6,    7,      OUT,    ON,     ON }},
    { "X",  { J6,    8,      OUT,    ON,     ON }},
    { "Y",  { J6,    9,      OUT,    ON,     ON }},
    { "Z",  { J6,    10,     OUT,    ON,     ON }},
    { "AA", { J6,    11,     OUT,    ON,     ON }},
    { "AB", { J6,    12,     OUT,    ON,     ON }},
    { "AC", { J6,    13,     OUT,    ON,     ON }},
    { "AD", { J6,    14,     OUT,    ON,     ON }},
    { "AE", { J6,    15,     OUT,    ON,     ON }},
    { "AF", { J6,    16,     OUT,    ON,     ON }}
};

rom const IOPinDescription InNonInvNonPull[]={
    { "A",  { J5,    1,      IN,       OFF,   OFF }},
    { "B",  { J5,    2,      IN,       OFF,   OFF }},
    { "C",  { J5,    3,      IN,       OFF,   OFF }},
    { "D",  { J5,    4,      IN,       OFF,   OFF }},
    { "E",  { J5,    5,      IN,       OFF,   OFF }},
    { "F",  { J5,    6,      IN,       OFF,   OFF }},
    { "G",  { J5,    7,      IN,       OFF,   OFF }},
    { "H",  { J5,    8,      IN,       OFF,   OFF }},
    { "I",  { J5,    9,      IN,       OFF,   OFF }},
    { "J",  { J5,    10,     IN,       OFF,   OFF }},
    { "K",  { J5,    11,     IN,       OFF,   OFF }},
    { "L",  { J5,    12,     IN,       OFF,   OFF }},
    { "M",  { J5,    13,     IN,       OFF,   OFF }},
    { "N",  { J5,    14,     IN,       OFF,   OFF }},
    { "O",  { J5,    15,     IN,       OFF,   OFF }},
    { "P",  { J5,    16,     IN,       OFF,   OFF }},
    { "Q",  { J6,    1,      IN,       OFF,   OFF }},
    { "R",  { J6,    2,      IN,       OFF,   OFF }},
    { "S",  { J6,    3,      IN,       OFF,   OFF }},
    { "T",  { J6,    4,      IN,       OFF,   OFF }},
    { "U",  { J6,    5,      IN,       OFF,   OFF }},
    { "V",  { J6,    6,      IN,       OFF,   OFF }},
    { "W",  { J6,    7,      IN,       OFF,   OFF }},
    { "X",  { J6,    8,      IN,       OFF,   OFF }},
    { "Y",  { J6,    9,      IN,       OFF,   OFF }},
    { "Z",  { J6,    10,     IN,       OFF,   OFF }},
    { "AA", { J6,    11,     IN,       OFF,   OFF }},
    { "AB", { J6,    12,     IN,       OFF,   OFF }},
    { "AC", { J6,    13,     IN,       OFF,   OFF }},
    { "AD", { J6,    14,     IN,       OFF,   OFF }},
    { "AE", { J6,    15,     IN,       OFF,   OFF }},
    { "AF", { J6,    16,     IN,       OFF,   OFF }}
};

rom const IOPinDescription InNonInvPull[]={
    { "A",  { J5,    1,      IN,       ON,     OFF }},
    { "B",  { J5,    2,      IN,       ON,     OFF }},
    { "C",  { J5,    3,      IN,       ON,     OFF }},
    { "D",  { J5,    4,      IN,       ON,     OFF }},
    { "E",  { J5,    5,      IN,       ON,     OFF }},
    { "F",  { J5,    6,      IN,       ON,     OFF }},
    { "G",  { J5,    7,      IN,       ON,     OFF }},
    { "H",  { J5,    8,      IN,       ON,     OFF }},
    { "I",  { J5,    9,      IN,       ON,     OFF }},
    { "J",  { J5,    10,     IN,       ON,     OFF }},
    { "K",  { J5,    11,     IN,       ON,     OFF }},
    { "L",  { J5,    12,     IN,       ON,     OFF }},
    { "M",  { J5,    13,     IN,       ON,     OFF }},
    { "N",  { J5,    14,     IN,       ON,     OFF }},
    { "O",  { J5,    15,     IN,       ON,     OFF }},
    { "P",  { J5,    16,     IN,       ON,     OFF }},
    { "Q",  { J6,    1,      IN,       ON,     OFF }},
    { "R",  { J6,    2,      IN,       ON,     OFF }},
    { "S",  { J6,    3,      IN,       ON,     OFF }},
    { "T",  { J6,    4,      IN,       ON,     OFF }},
    { "U",  { J6,    5,      IN,       ON,     OFF }},
    { "V",  { J6,    6,      IN,       ON,     OFF }},
    { "W",  { J6,    7,      IN,       ON,     OFF }},
    { "X",  { J6,    8,      IN,       ON,     OFF }},
    { "Y",  { J6,    9,      IN,       ON,     OFF }},
    { "Z",  { J6,    10,     IN,       ON,     OFF }},
    { "AA", { J6,    11,     IN,       ON,     OFF }},
    { "AB", { J6,    12,     IN,       ON,     OFF }},
    { "AC", { J6,    13,     IN,       ON,     OFF }},
    { "AD", { J6,    14,     IN,       ON,     OFF }},
    { "AE", { J6,    15,     IN,       ON,     OFF }},
    { "AF", { J6,    16,     IN,       ON,     OFF }}
};

rom const IOPinDescription InInvNonPull[]={
    { "A",  { J5,    1,      IN,       OFF,    ON }},
    { "B",  { J5,    2,      IN,       OFF,    ON }},
    { "C",  { J5,    3,      IN,       OFF,    ON }},
    { "D",  { J5,    4,      IN,       OFF,    ON }},
    { "E",  { J5,    5,      IN,       OFF,    ON }},
    { "F",  { J5,    6,      IN,       OFF,    ON }},
    { "G",  { J5,    7,      IN,       OFF,    ON }},
    { "H",  { J5,    8,      IN,       OFF,    ON }},
    { "I",  { J5,    9,      IN,       OFF,    ON }},
    { "J",  { J5,    10,     IN,       OFF,    ON }},
    { "K",  { J5,    11,     IN,       OFF,    ON }},
    { "L",  { J5,    12,     IN,       OFF,    ON }},
    { "M",  { J5,    13,     IN,       OFF,    ON }},
    { "N",  { J5,    14,     IN,       OFF,    ON }},
    { "O",  { J5,    15,     IN,       OFF,    ON }},
    { "P",  { J5,    16,     IN,       OFF,    ON }},
    { "Q",  { J6,    1,      IN,       OFF,    ON }},
    { "R",  { J6,    2,      IN,       OFF,    ON }},
    { "S",  { J6,    3,      IN,       OFF,    ON }},
    { "T",  { J6,    4,      IN,       OFF,    ON }},
    { "U",  { J6,    5,      IN,       OFF,    ON }},
    { "V",  { J6,    6,      IN,       OFF,    ON }},
    { "W",  { J6,    7,      IN,       OFF,    ON }},
    { "X",  { J6,    8,      IN,       OFF,    ON }},
    { "Y",  { J6,    9,      IN,       OFF,    ON }},
    { "Z",  { J6,    10,     IN,       OFF,    ON }},
    { "AA", { J6,    11,     IN,       OFF,    ON }},
    { "AB", { J6,    12,     IN,       OFF,    ON }},
    { "AC", { J6,    13,     IN,       OFF,    ON }},
    { "AD", { J6,    14,     IN,       OFF,    ON }},
    { "AE", { J6,    15,     IN,       OFF,    ON }},
    { "AF", { J6,    16,     IN,       OFF,    ON }}
};

rom const IOPinDescription InInvPull[]={
    { "A",  { J5,    1,      IN,       ON,    ON }},
    { "B",  { J5,    2,      IN,       ON,    ON }},
    { "C",  { J5,    3,      IN,       ON,    ON }},
    { "D",  { J5,    4,      IN,       ON,    ON }},
    { "E",  { J5,    5,      IN,       ON,    ON }},
    { "F",  { J5,    6,      IN,       ON,    ON }},
    { "G",  { J5,    7,      IN,       ON,    ON }},
    { "H",  { J5,    8,      IN,       ON,    ON }},
    { "I",  { J5,    9,      IN,       ON,    ON }},
    { "J",  { J5,    10,     IN,       ON,    ON }},
    { "K",  { J5,    11,     IN,       ON,    ON }},
    { "L",  { J5,    12,     IN,       ON,    ON }},
    { "M",  { J5,    13,     IN,       ON,    ON }},
    { "N",  { J5,    14,     IN,       ON,    ON }},
    { "O",  { J5,    15,     IN,       ON,    ON }},
    { "P",  { J5,    16,     IN,       ON,    ON }},
    { "Q",  { J6,    1,      IN,       ON,    ON }},
    { "R",  { J6,    2,      IN,       ON,    ON }},
    { "S",  { J6,    3,      IN,       ON,    ON }},
    { "T",  { J6,    4,      IN,       ON,    ON }},
    { "U",  { J6,    5,      IN,       ON,    ON }},
    { "V",  { J6,    6,      IN,       ON,    ON }},
    { "W",  { J6,    7,      IN,       ON,    ON }},
    { "X",  { J6,    8,      IN,       ON,    ON }},
    { "Y",  { J6,    9,      IN,       ON,    ON }},
    { "Z",  { J6,    10,     IN,       ON,    ON }},
    { "AA", { J6,    11,     IN,       ON,    ON }},
    { "AB", { J6,    12,     IN,       ON,    ON }},
    { "AC", { J6,    13,     IN,       ON,    ON }},
    { "AD", { J6,    14,     IN,       ON,    ON }},
    { "AE", { J6,    15,     IN,       ON,    ON }},
    { "AF", { J6,    16,     IN,       ON,    ON }}
};
#pragma romdata
const BYTE nTestgpios=32;

//Testing features. This module is only compiled if the features are enabled.
static GPIOTestSM currentTest = idle;
static DWORD BootupDelay=0;

void TEST_TestInit(void)
{
    TIME start={0}, stop={0};
    GetTime(&stop);
    start.secs=0;
    start.ticks=0;
    BootupDelay=CalculateDelayMs(&start,&stop);

    return;
}

//This function is used to test ad-hoc the various features I'm developing
//and to better understand how the system works.
void TEST_InDevelopmentTesting(void)
{
    TIME now;
    TIME then;
    TIME later;
    DWORD ms=0;
    BYTE x=1;
    BYTE IO1Bckp[22]={0};
    BYTE IO2Bckp[22]={0};
    BYTE GPIOBankA=0;
    BYTE GPIOBankB=0;
    BYTE OLATBankA=0;
    BYTE OLATBankB=0;
    BYTE InitVerbosity = GetVerbosity();

    if(SCFG != TEcfg)
    {
        return; //we need serial port to use the printf() in these tests
    }
    printf("\nStarting some testing of the dalf features.\r\n");
    
    printf("Boot-up time: %ld ms.\r\n",BootupDelay);

    printf("Testing the command line now.\r\n");
    
    printf("Sending 'C' as a command. Should see the A/D readings.\r\n");
    CMD = 'C';
    ARGN = 0;
    TeCmdDispatchExt();
    printf("Command line testing done.\r\n");

    printf("Testing the time now.\r\n");
    //Now lets wait x.3 secs before reprinting the time
    //Unfortunately this is a busy wait and cannot be used in real applications

    printf("Doing a busy wait of %d.3 secs.\r\n",x);
    GetTime(&now);
    ms=TIME_TO_MSEC(now);
    printf("secs before = %lu; ticks before = %u; msecs before= %ld.\r\n",now.secs, now.ticks, ms);
    SetDelay(&then,x,msec_300);
    while(!Timeout(&then));
    ms=TIME_TO_MSEC(then);
    printf("secs after= %lu; ticks after= %u; msecs after= %ld.\r\n",then.secs, then.ticks, ms);
    printf("delay (ms)= %ld.\r\n", CalculateDelayMs(&now, &then));
    printf("Timing testing done.\r\n");

    //test the macros for file and line number - for now the __FILE__ macro isn't processing the backslashes correctly
    printf("Testing __FILE__ and __LINE__ macros. The __FILE__ macro may not be eating backslashes. If so, uncomment 1st line of file.\r\n");
    printf("File: "__FILE__"; line: %d.\r\n",__LINE__);
    printf("Macros testing done.\r\n");

    // verbosity test
    //Remember that fatal error includes always timestamp and caller information
    printf("Starting verbosity macros test. Remember that fatal error always includes timestamp and call info.\r\n");
    SetVerbosity(VERBOSITY_DISABLED); //Prints fatal error
    printf("Prints fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_USE_TIMESTAMP); //Prints fatal error
    printf("Prints fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_USE_CALL_INFO); //Prints fatal error
    printf("Prints fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_LEVEL_ERROR); //Prints error & fatal error
    printf("Prints msg, error & fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING); //Prints warning & fatal error
    printf("Prints msg, warning & fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS); //Prints fatal error & status
    printf("Prints msg, fatal error & status:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_DEBUG); //Prints fatal error & debug
    printf("Prints msg, fatal error & debug:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_LEVEL_DEBUG | VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_TIMESTAMP | VERBOSITY_USE_CALL_INFO); //Prints status, debug, warning, error & fatal error, all with timestamp & call info
    printf("Prints msg, status, debug, warning, error & fatal error, all with timestamp & call info:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_CALL_INFO); //Prints warning w/call info, fatal error & error w/call info
    printf("Prints msg, warning w/call info, fatal error & error w/call info:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_USE_TIMESTAMP | VERBOSITY_LEVEL_DEBUG); //Prints status & debug w/timestamp & fatal error
    printf("Prints msg, status & debug w/timestamp & fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING); //Prints warning & fatal error
    printf("Prints msg, warning & fatal error:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X.\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X.\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X.\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X.\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X.\r\n",GetVerbosity());
    MSG("Just a simple msg, without qualifiers. Verbosity=%02X.\r\n",GetVerbosity());
    
    printf("Now testing the delay in a message.\r\n");
    GetTime(&then);
    FATAL_ERROR_MSG("Just measuring a FATAL_ERROR_MSG processing time.\r\n");
    GetTime(&later);
    printf("Worst case scenario verbosity message processing time (does not include serial port\
flushing)=%ld ms.\r\n",CalculateDelayMs(&then,&later));
    
    printf("Ends verbosity tests.\r\n");

    printf("Testing expandable I/O now.\r\n");
    
    printf("Now testing I/O pins to be used for input.\r\n");
    printf("Configuring all registers as input.\r\n");
    WriteIOExp1(0x00,0xFF); //I/O Exp1, Bank A - set as Input
    WriteIOExp1(0x01,0xFF); //I/O Exp1, Bank B - set as Input
    WriteIOExp2(0x00,0xFF); //I/O Exp2, Bank A - set as Input
    WriteIOExp2(0x01,0xFF); //I/O Exp2, Bank B - set as Input
    
    printf("Configuring expander 1's registers with inverted polarity, and expander 2's with \
non-inverted.\r\n");
    WriteIOExp1(0x02,0xFF); //I/O Exp1, Bank A - set as inverted
    WriteIOExp1(0x03,0xFF); //I/O Exp1, Bank B - set as inverted
    WriteIOExp2(0x02,0x00); //I/O Exp2, Bank A - set as Non-inverted
    WriteIOExp2(0x03,0x00); //I/O Exp2, Bank B - set as Non-inverted
    
    printf("Disabling bank B's register pull-ups, and enabling Bank A's.\r\n");
    WriteIOExp1(0x0C,0xFF); //I/O Exp1, Bank A - enable pull-ups
    WriteIOExp1(0x0D,0x00); //I/O Exp1, Bank B - disable pull-ups
    WriteIOExp2(0x0C,0xFF); //I/O Exp2, Bank A - enable pull-ups
    WriteIOExp2(0x0D,0x00); //I/O Exp2, Bank B - disable pull-ups
    printf("All pins ready to be used as input. Expected readings per register/bank/expander \
using this configuration:\r\n");
    printf("Exp 1 - Bank A: should read '1' only if grounded. '0' otherwise.\r\n");
    printf("Exp 1 - Bank B: should read '0' only if fed with 5V. '1' otherwise.\r\n");
    printf("Exp 2 - Bank A: should read '0' only if grounded. '1' otherwise.\r\n");
    printf("Exp 2 - Bank B: should read '1' only if fed with 5V. '0' otherwise.\r\n");
    
    GPIOBankA=ReadIOExp1(0x12);
    GPIOBankB=ReadIOExp1(0x13);
    OLATBankA=ReadIOExp1(0x14);
    OLATBankB=ReadIOExp1(0x15);
    printf("Expander 1 readings: GPIOA=%08b; GPIOB=%08b; OLATA=%08b; OLATB=%08b.\r\n",GPIOBankA, GPIOBankB,
    OLATBankA, OLATBankB);
    GPIOBankA=ReadIOExp2(0x12);
    GPIOBankB=ReadIOExp2(0x13);
    OLATBankA=ReadIOExp2(0x14);
    OLATBankB=ReadIOExp2(0x15);
    printf("Expander 2 readings: GPIOA=%08b; GPIOB=%08b; OLATA=%08b; OLATB=%08b.\r\n",GPIOBankA, GPIOBankB,
    OLATBankA, OLATBankB);
    //Cannot let the user measure signal fed to the pins, because we must restore the original state
    //of the I/O expanders after testing. If you want to do that, you must comment that action below
    
    printf("\r\nAll testing done. Reverting all configurations and resuming normal operation\r\n");
    ROVIM_T2D_Init();
    ROVIM_T2D_Start();
    
    return;
}

//this function start the GPIO sub-driver test procedure.
/*The GPIO sub-driver test procedure intends to validate all aspects of the sub-driver operation.
Is runs as follows:
-Public functions are tested for recovery from invalid input;
-A state machine runs all possible GPIO configurations;
-A state machine tests all possible operations on each GPIO configuration. User input is requested
when necessary.
-Test ends when all configurations have been tested.
*/
void TEST_StartGPIODriverTest(void)
{
    BYTE value=0;
    IOPinConfig config={0}, compareConfig={0};
    BYTE results=0;
    
    //let's have a valid GPIO to use whenever needed. We choose one input, as will be needed later.
    memcpypgm2ram(&config,(const rom void *) &GPIOsDescription[5].config,sizeof(config));

    MSG("\r\nStarting GPIO driver test.\r\nFirst battery tests public driver functions for \
correct input validation.\r\n");
    //invalid input test
 
    //1-NULL pointer
    STATUS_MSG("Now testing invalid pointer (NULL) arguments detection.\r\n");
    /*In this test all functions should return error without executing any further*/
    results|=SetGPIOConfig(GPIOsDescription[5].name, NULL);
    results|=GetGPIOConfig(GPIOsDescription[5].name, NULL);
    results|=SetGPIOConfig(NULL, &config);
    results|=GetGPIOConfig(NULL, &config);
    results|=CompareGPIOConfig(NULL, &config);
    results|=CompareGPIOConfig(&config, NULL);
    results|=SetGPIO(NULL);
    results|=GetGPIO(NULL,&value);
    results|=GetGPIO(GPIOsDescription[5].name,NULL);
    results|=ToggleGPIO(NULL);
    results|=ResetGPIO(NULL);
    if(results)
    {
        STATUS_MSG("Invalid pointer detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Invalid pointer detection test successful.\r\n");
    }
    
    //3-Non-existent GPIO name
    STATUS_MSG("Now testing non-existent name detection.\r\n");
    /*In this test all functions should detect that the GPIO name isn't in the database, and should
    return an error, , without interacting with the I/O expanders*/
    /*Let's run this test around a valid GPIO configuration, except for the name, which is not 
    registered*/
    results=0;
    results|=SetGPIOConfig("1st",&config);
    results|=GetGPIOConfig("1st",&config);
    results|=SetGPIO("1st");
    results|=GetGPIO("1st",&value);
    results|=ToggleGPIO("1st");
    results|=ResetGPIO("1st");
    if(results)
    {
        STATUS_MSG("Non-existent name detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Non-existent name detection test successful.\r\n");
    }
    
    //3-array out of bounds
    STATUS_MSG("Now testing array out of bounds detection.\r\n");
    /*In this test all functions should detect that the GPIO name is either too long or too short,
    and should return an error , without interacting with the I/O expanders*/
    /*Let's run this test around a valid GPIO configuration, except for the name, which is not 
    registered*/
    results=0;
    results|=SetGPIOConfig("name tooooooooooooooooooooooooo long",&config);
    results|=GetGPIOConfig("name tooooooooooooooooooooooooo long",&config);
    results|=SetGPIO("name tooooooooooooooooooooooooo long");
    results|=GetGPIO("name tooooooooooooooooooooooooo long",&value);
    results|=ToggleGPIO("name tooooooooooooooooooooooooo long");
    results|=ResetGPIO("name tooooooooooooooooooooooooo long");
    results|=SetGPIOConfig("",&config);
    results|=GetGPIOConfig("",&config);
    results|=SetGPIO("");
    results|=GetGPIO("",&value);
    results|=ToggleGPIO("");
    results|=ResetGPIO("");
    if(results)
    {
        STATUS_MSG("Array out of bounds detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Array out of bounds detection test successful.\r\n");
    }
    
    //4-Invalid Operations on GPIO
    STATUS_MSG("Now testing invalid operations on input pin detection.\r\n");
    /*In this test the SetGPIO, ResetGPIO and ToggleGPIO operations should fail without interacting\
    with the I/O expanders, because the GPIO is configured as input*/
    SetGPIOConfig(GPIOsDescription[5].name,&config); //Configure the GPIo correctly as an input
    results=0;
    results|=SetGPIO(GPIOsDescription[5].name);
    results|=ToggleGPIO(GPIOsDescription[5].name);
    results|=ResetGPIO(GPIOsDescription[5].name);
    if(results)
    {
        STATUS_MSG("Invalid operations on input pin detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Invalid operations on input pin detection test successful.\r\n");
    }
    
    //5-invalid config data
    STATUS_MSG("Now testing invalid config data detection.\r\n");
    /*In this test all functions should detect the invalid configuration for the GPIO and return an\
    error, without interacting with the I/O expanders. Let's run this test around a valid GPIO name*/
    memset(&config,0x0,sizeof(config)); //invalid pin number
    results=0;
    results|=SetGPIOConfig(GPIOsDescription[5].name,&config);
    memset(&config,0x1,sizeof(config)); //configuration data valid, but not matching GPIO's pin and expander
    results|=SetGPIOConfig(GPIOsDescription[5].name,&config);
    memset(&config,0x5,sizeof(config)); //invalid exp, dir, pullup and inverted fields
    results|=SetGPIOConfig(GPIOsDescription[5].name,&config);
    memset(&config,0x11,sizeof(config)); //all fields invalid
    results|=SetGPIOConfig(GPIOsDescription[5].name,&config);
    if(results)
    {
        STATUS_MSG("Invalid config data detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Invalid config data detection test successful.\r\n");
    }
    
    STATUS_MSG("Now testing detection of different GPIO configurations\r\n");
    /* In this test, the compare function should detect differences in each of GPIO configuration parameters*/
    results=0;
    memcpypgm2ram(&config,(const rom void *) &GPIOsDescription[5].config,sizeof(config));
    memcpypgm2ram(&compareConfig,(const rom void *) &GPIOsDescription[5].config,sizeof(compareConfig));
    compareConfig.exp=J6;
    results|=CompareGPIOConfig(&compareConfig, &config);
    memcpypgm2ram(&compareConfig,(const rom void *) &GPIOsDescription[5].config,sizeof(compareConfig));
    compareConfig.number+=1;
    results|=CompareGPIOConfig(&compareConfig, &config);
    memcpypgm2ram(&compareConfig,(const rom void *) &GPIOsDescription[5].config,sizeof(compareConfig));
    compareConfig.pullup=!compareConfig.pullup;
    results|=CompareGPIOConfig(&compareConfig, &config);
    memcpypgm2ram(&compareConfig,(const rom void *) &GPIOsDescription[5].config,sizeof(compareConfig));
    compareConfig.inverted=!compareConfig.inverted;
    results|=CompareGPIOConfig(&compareConfig, &config);
    memcpypgm2ram(&compareConfig,(const rom void *) &GPIOsDescription[5].config,sizeof(compareConfig));
    compareConfig.dir=!compareConfig.dir;
    results|=CompareGPIOConfig(&compareConfig, &config);
    if(results)
    {
        STATUS_MSG("Different GPIO configuration detection test unsuccessful.\r\n");
    }
    else
    {
        STATUS_MSG("Different GPIO configuration detection test successful.\r\n");
    }
    
    STATUS_MSG("Finished GPIO driver invalid input test.\r\n\r\n");
    
    //functional test
    currentTest=0;
    AckCallback=TEST_GPIODriverOperations;
    AckCallback();
}

/*
Tests GPIO sub-driver operations on individual IO expander pins.
Operates a sequential state-machine that mimics the GPIO sub-driver test procedure designed for the
GPIO operations part. Before the state machine runs, a GPIO configuration is set.
*/
void TEST_GPIODriverOperations(void)
{
    BYTE value=0xFF; //current value of the GPIO. Saves GetGPIO results
    BYTE i=0;
    
    //Used for one at a time testing
    static BYTE j=0;
    
    switch(currentTest)
    {
        case config:
            //run a set of GPIO configurations
            currentTest = TEST_GPIODriverConfig();
            break;
            
        case set:
            for(i=0;i<ngpios;i++)
            {
                if( !SetGPIO(GPIOsDescription[i].name))
                {
                    STATUS_MSG("SetGPIO failed.\r\n");
                }
            }
            STATUS_MSG("GPIO Test: Set complete. Please confirm if result is as expected.\
.\r\nPress 'G' <ENTER> to proceed to Reset operation.\r\n");
            currentTest=reset;
            break;
            
        case reset:
            for(i=0;i<ngpios;i++)
            {
                if( !ResetGPIO(GPIOsDescription[i].name))
                {
                    STATUS_MSG("ResetGPIO failed.\r\n");
                }
            }
            STATUS_MSG("GPIO Test: Reset complete. Please confirm if result is as expected.\
.\r\nPress 'G' <ENTER> to proceed to 1st Toggle operation.\r\n");
            currentTest=toggle1;
            break;
            
        case toggle1:
            for(i=0;i<ngpios;i++)
            {
                if( !ToggleGPIO(GPIOsDescription[i].name))
                {
                    STATUS_MSG("1st ToggleGPIO failed.\r\n");
                }
            }
            STATUS_MSG("GPIO Test: 1st Toggle complete. Please confirm if result is as expected.\
.\r\nPress 'G' <ENTER> to proceed to 2nd Toggle operation.\r\n");
            currentTest=toggle2;
            break;

        case toggle2:
            for(i=0;i<ngpios;i++)
            {
                if( !ToggleGPIO(GPIOsDescription[i].name))
                {
                    STATUS_MSG("2nd ToggleGPIO failed.\r\n");
                }
            }
            STATUS_MSG("GPIO Test: 2nd Toggle complete. Please confirm if result is as expected.\
.\r\nPress 'G' <ENTER> to proceed to Get operation.\r\n");
            currentTest=get;
            break;
            
        case get:
            for(i=0;i<ngpios;i++)
            {
                value=0xFF;
                if( !GetGPIO(GPIOsDescription[i].name,&value))
                {
                    STATUS_MSG("GetGPIO failed.\r\n");
                }
                STATUS_MSG("GPIO %d on expander J%d=%d.\r\n", i<16? i+1: i-15, (i<16?5:6), value);
            }
            STATUS_MSG("GPIO Test: Get complete. Please confirm if result is as expected.\
.\r\nPress 'G' <ENTER> to proceed to next operation\r\n");
            currentTest=config;
            break;
        
        case getOneAtATime1:
            while(j<(ngpios-1))
            {
                value=0xFF;
                if( !GetGPIO(GPIOsDescription[j].name,&value))
                {
                    STATUS_MSG("GetGPIO for individual pin failed.\r\n");
                }
                STATUS_MSG("GPIO %d on exp J%d=%d. Confirm it reads as expected.\r\n",
                j<16? j+1: j-15, (j<16?5:6), value);
                STATUS_MSG("Now feed pin %d on connector J%d with 5V and Press 'G' <ENTER> to read it\r\n",
                j<15? j+1+1: j-15+1, (j<16?5:6));
                j++;
                return;
            }
            value=0xFF;
            if( !GetGPIO(GPIOsDescription[j].name,&value))
            {
                STATUS_MSG("GetGPIO for individual pin failed.\r\n");
            }
            STATUS_MSG("GPIO %d on exp J%d=%d. Confirm it reads as expected.\r\n",
            j<16? j+1: j-15, (j<16?5:6), value);
            STATUS_MSG("GPIO Test: 5V input test complete. Connect now pin 1 on connector J5 to the\
ground and press 'G' <ENTER> to read it\r\n");
            j=0;
            currentTest=getOneAtATime2;
            break;
            
        case getOneAtATime2:
            while(j<(ngpios-1))
            {

                value=0xFF;
                if( !GetGPIO(GPIOsDescription[j].name,&value))
                {
                    STATUS_MSG("GetGPIO for individual pin failed\r\n");
                }
                STATUS_MSG("GPIO %d on exp J%d=%d. Confirm it reads as expected\r\n",
                j<16? j+1: j-15, (j<16?5:6), value);
                STATUS_MSG("Now feed pin %d on connector J%d to ground and Press 'G' <ENTER> to read it\r\n",
                j<15? j+1+1: j-15+1, (j<16?5:6));
                j++;
                return;
            }
            value=0xFF;
            if( !GetGPIO(GPIOsDescription[j].name,&value))
            {
                STATUS_MSG("GetGPIO for individual pin failed\r\n");
            }
            STATUS_MSG("GPIO %d on exp J%d=%d. Confirm it reads as expected\r\n",
            j<16? j+1: j-15, (j<16?5:6), value);
            STATUS_MSG("GPIO Test: 0V input test complete.\
\r\nPress 'G' <ENTER> to proceed to next configuration test\r\n");
            j=0;
            currentTest=config; //XXX: change when next states are ready
            break;
        
        case idle:
            break;
            
        default:
            FATAL_ERROR_MSG("Unexpected state of state machine\r\n");
            break;
    }
}

/*
Handles GPIO sub-driver configurations on individual IO expander pins.
Operates a sequential state-machine that mimics the GPIO sub-driver test procedure designed for the
configuration part.
*/
GPIOTestSM TEST_GPIODriverConfig(void)
{
    static GPIOConfigSM currentConfig = out;
    IOPinConfig configSet={0};
    IOPinConfig configGet={0};
    BYTE i=0;

    switch (currentConfig)
    {
        case out:
            STATUS_MSG("Starting GPIO driver testing.\r\nConfiguring all io expanders's pins as \
output non-inverted without pullup.\r\n");
            
            GPIOsDescription = OutNonInvNonPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'G' <ENTER> when ready to proceed to 'Set' operation.\r\n");
            currentConfig=outPullup;
            return set;
            break;
            
            
            
            
        case outPullup:
            STATUS_MSG("Configuring all io expanders pins as output non-inverted with pullup.\r\n");
            
            GPIOsDescription = OutNonInvPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'G' <ENTER> when ready to proceed to 'Set' operation.\r\n");
            currentConfig=outInverted;
            return set;
            break;
            
            
            
            
        case outInverted:
            STATUS_MSG("Configuring all io expanders pins as output inverted without pullup.\r\n");
            
            GPIOsDescription = OutInvNonPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'G' <ENTER> when ready to proceed to 'Set' operation.\r\n");
            currentConfig=outPullupInverted;
            return set;
            break;
            
            
            
            
        case outPullupInverted:
            STATUS_MSG("Configuring all io expanders pins as output inverted with pullup.\r\n");
            
            GPIOsDescription = OutInvPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'G' <ENTER> when ready to proceed to 'Set' operation.\r\n");
            currentConfig=in;
            return set;
            break;
            
            
            
            
        case in:
            STATUS_MSG("Configuring all io expanders pins as input non-inverted without pullup.\r\n");
            
            GPIOsDescription = InNonInvNonPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage. It should be floating.\r\nThen, repeat the following for each pin:\r\n\
>Feed each pin with 5V and confirm the computer measures it as '1'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm the computer measures it as '0'. \
Repeat for every pin.\r\nAfter the measurements, and while feeding 5V to pin 1 on expander J5, \
press 'G' <ENTER> to proceed to the first operation.\r\n");
            currentConfig=inPullup;
            return getOneAtATime1;
            break;
            
            
            
            
        case inPullup:
            STATUS_MSG("Configuring all io expanders pins as input non-inverted with pullup.\r\n");
            
            GPIOsDescription = InNonInvPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage. It should be 5V.\r\nThen, repeat the following for each pin:\r\n\
>Feed each pin with 5V and confirm the computer measures it as '1'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm the computer measures it as '0'. \
Repeat for every pin.\r\nAfter the measurements, and while feeding 5V to pin 1 on expander J5, \
press 'G' <ENTER> to proceed to the first operation.\r\n");
            currentConfig=inInverted;
            return getOneAtATime1;
            break;
            
            
            
            
        case inInverted:
            STATUS_MSG("Configuring all io expanders pins as input inverted without pullup.\r\n");
            
            GPIOsDescription = InInvNonPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage. It should be floating.\r\nThen, repeat the following for each pin:\r\n\
>Feed each pin with 5V and confirm the computer measures it as '0'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm the computer measures it as '1'. \
Repeat for every pin.\r\nAfter the measurements, and while feeding 5V to pin 1 on expander J5, \
press 'G' <ENTER> to proceed to the first operation.\r\n");
            currentConfig=inPullupInverted;
            return getOneAtATime1;
            break;
            
            
            
            
        case inPullupInverted:
            STATUS_MSG("Configuring all io expanders pins as input inverted with pullup.\r\n");
            
            GPIOsDescription = InInvPull;
            ngpios= nTestgpios;
            for(i=0;i<ngpios;i++)
            {
                memcpypgm2ram(&configSet,(const rom void *) &GPIOsDescription[i].config,sizeof(configSet));
                if( !SetGPIOConfig(GPIOsDescription[i].name,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error.\r\n");
                }
                if( !GetGPIOConfig(GPIOsDescription[i].name, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error.\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one  \
for pin %d on I/O expander J%d.\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage. It should be 5V.\r\nThen, repeat the following for each pin:\r\n\
>Feed each pin with 5V and confirm the computer measures it as '0'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm the computer measures it as '1'. \
Repeat for every pin.\r\nAfter the measurements, and while feeding 5V to pin 1 on expander J5, \
press 'G' <ENTER> to proceed to the first operation.\r\n");
            currentConfig=end;
            return getOneAtATime1;
            break;
            
            
            
            
        case end:
            currentTest = end;
            AckCallback = NULL;
            //ROVIM_T2D_Init();
            //ROVIM_T2D_Start();
            STATUS_MSG("GPIO testing completed. Reverting changes.\r\n");
            ROVIM_T2D_ConfigGPIOs();
            break;
            
            
            
            
        default:
            FATAL_ERROR_MSG("Unexpected state of state machine.\r\n");
            break;
    }
    return idle;
}

