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

//Testing features. This module is only compiled it the features are enabled.

void TEST_TestInit(void)
{
    TIME start={0}, stop={0};
    GetTime(&stop);
    start.secs=0;
    start.ticks=0;

    if(SCFG != TEcfg)
    {
        return; //we need serial port to use the printf() in these tests
    }
    printf("Testing boot-up time: %ld ms\r\n",CalculateDelayMs(&start,&stop));
    
    printf("\r\nAll testing done. Reverting all configurations and resuming normal operation\r\n");
    ROVIM_T2D_Init();
    ROVIM_T2D_Start();
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
    printf("Starting some testing of the dalf features\r\n");

    printf("Testing the command line now\r\n");
    /* not used for now
    CMD = 0xA;
    ARGN = 0;
    TeCmdDispatchExt();
    printf("Sent ack (ENTER) as a command\r\n");*/
    
    printf("Sending 'C' as a command. Should see the A/D readings\r\n");
    CMD = 'C';
    ARGN = 0;
    TeCmdDispatchExt();
    printf("Command line testing done\r\n");

    printf("Testing the time now\r\n");
    //Now lets wait x.3 secs before reprinting the time
    //Unfortunately this is a busy wait and cannot be used in real applications

    printf("Doing a busy wait of %d.3 secs\r\n",x);
    GetTime(&now);
    ms=TIME_TO_MSEC(now);
    printf("secs before = %lu; ticks before = %u; msecs before= %ld\r\n",now.secs, now.ticks, ms);
    SetDelay(&then,x,msec_300);
    while(!Timeout(&then));
    ms=TIME_TO_MSEC(then);
    printf("secs after= %lu; ticks after= %u; msecs after= %ld\r\n",then.secs, then.ticks, ms);
    printf("delay (ms)= %ld\r\n", CalculateDelayMs(&now, &then));
    printf("Timing testing done\r\n");

    //test the macros for file and line number - for now the __FILE__ macro isn't processing the backslashes correctly
    printf("Testing __FILE__ and __LINE__ macros. The __FILE__ macro may not be eating backslashes. If so, uncomment 1st line of file.\r\n");
    printf("File: "__FILE__"; line: %d\r\n",__LINE__);
    printf("Macros testing done\r\n");

    // verbosity test
    //Remember that fatal error includes always timestamp and caller information
    printf("Starting verbosity macros test. Remember that fatal error always includes timestamp and call info.\r\n");
    SetVerbosity(VERBOSITY_DISABLED); //Prints fatal error
    printf("Prints fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_USE_TIMESTAMP); //Prints fatal error
    printf("Prints fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_USE_CALL_INFO); //Prints fatal error
    printf("Prints fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    
    SetVerbosity(VERBOSITY_LEVEL_ERROR); //Prints error & fatal error
    printf("Prints error & fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING); //Prints warning & fatal error
    printf("Prints warning & fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS); //Prints fatal error & status
    printf("Prints fatal error & status:\r\n");
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_DEBUG); //Prints fatal error & debug
    printf("Prints fatal error & debug:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_LEVEL_DEBUG | VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_TIMESTAMP | VERBOSITY_USE_CALL_INFO); //Prints status, debug, warning, error & fatal error, all with timestamp & call info
    printf("Prints status, debug, warning, error & fatal error, all with timestamp & call info:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_CALL_INFO); //Prints warning w/call info, fatal error & error w/call info
    printf("Prints warning w/call info, fatal error & error w/call info:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_USE_TIMESTAMP | VERBOSITY_LEVEL_DEBUG); //Prints status & debug w/timestamp & fatal error
    printf("Prints status & debug w/timestamp & fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

    SetVerbosity(VERBOSITY_LEVEL_WARNING); //Prints warning & fatal error
    printf("Prints warning & fatal error:\r\n");
    STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
    FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
    DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
    WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
    ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
    
    printf("Now testing the delay in a message.\r\n");
    GetTime(&then);
    FATAL_ERROR_MSG("Just measuring a FATAL_ERROR_MSG processing time\r\n");
    GetTime(&later);
    printf("Worst case scenario verbosity message processing time (does not include serial port\
flushing)=%ld ms\r\n",CalculateDelayMs(&then,&later));
    
    printf("Ends verbosity tests\r\n");

    printf("Testing expandable I/O now\r\n");
    
    printf("Now testing I/O pins to be used for input\r\n");
    printf("Configuring all registers as input\r\n");
    WriteIOExp1(0x00,0xFF); //I/O Exp1, Bank A - set as Input
    WriteIOExp1(0x01,0xFF); //I/O Exp1, Bank B - set as Input
    WriteIOExp2(0x00,0xFF); //I/O Exp2, Bank A - set as Input
    WriteIOExp2(0x01,0xFF); //I/O Exp2, Bank B - set as Input
    
    printf("Configuring expander 1's registers with inverted polarity, and expander 2's with \
non-inverted\r\n");
    WriteIOExp1(0x02,0xFF); //I/O Exp1, Bank A - set as inverted
    WriteIOExp1(0x03,0xFF); //I/O Exp1, Bank B - set as inverted
    WriteIOExp2(0x02,0x00); //I/O Exp2, Bank A - set as Non-inverted
    WriteIOExp2(0x03,0x00); //I/O Exp2, Bank B - set as Non-inverted
    
    printf("Disabling bank B's register pull-ups, and enabling Bank A's\r\n");
    WriteIOExp1(0x0C,0xFF); //I/O Exp1, Bank A - enable pull-ups
    WriteIOExp1(0x0D,0x00); //I/O Exp1, Bank B - disable pull-ups
    WriteIOExp2(0x0C,0xFF); //I/O Exp2, Bank A - enable pull-ups
    WriteIOExp2(0x0D,0x00); //I/O Exp2, Bank B - disable pull-ups
    printf("All pins ready to be used as input. Expected readings per register/bank/expander \
using this configuration:\r\n");
    printf("Exp 1 - Bank A: should read '1' only if grounded. '0' otherwise\r\n");
    printf("Exp 1 - Bank B: should read '0' only if fed with 5V. '1' otherwise\r\n");
    printf("Exp 2 - Bank A: should read '0' only if grounded. '1' otherwise\r\n");
    printf("Exp 2 - Bank B: should read '1' only if fed with 5V. '0' otherwise\r\n");
    
    GPIOBankA=ReadIOExp1(0x12);
    GPIOBankB=ReadIOExp1(0x13);
    OLATBankA=ReadIOExp1(0x14);
    OLATBankB=ReadIOExp1(0x15);
    printf("Expander 1 readings: GPIOA=%08b; GPIOB=%08b; OLATA=%08b; OLATB=%08b\r\n",GPIOBankA, GPIOBankB,
    OLATBankA, OLATBankB);
    GPIOBankA=ReadIOExp2(0x12);
    GPIOBankB=ReadIOExp2(0x13);
    OLATBankA=ReadIOExp2(0x14);
    OLATBankB=ReadIOExp2(0x15);
    printf("Expander 2 readings: GPIOA=%08b; GPIOB=%08b; OLATA=%08b; OLATB=%08b\r\n",GPIOBankA, GPIOBankB,
    OLATBankA, OLATBankB);
    //Cannot let the user measure signal fed to the pins, because we must restore the original state
    //of the I/O expanders after testing. If you want to do that, you must comment that action below
    
    printf("\r\nAll testing done. Reverting all configurations and resuming normal operation\r\n");
    ROVIM_T2D_Init();
    ROVIM_T2D_Start();
    
    return;
}
