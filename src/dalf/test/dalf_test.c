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

//Testing features. This module is only compiled if the features are enabled.
static GPIOTestSM currentTest = idle;
static DWORD BootupDelay=0;
//used for GPIO driver test, because string arguments passed to functions must have rom qualifier
#pragma romdata cenas
rom char GPIONameInRom[30];
rom BYTE auxil;
#pragma romdata

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
    printf("\nStarting some testing of the dalf features\r\n");
    
    printf("Boot-up time: %ld ms\r\n",BootupDelay);

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
    //currentTest = config;
    BYTE value=0;
    IOPinConfig config={0};
    BYTE results=0;
    
    //let's have a valid GPIO to use whenever needed. We choose one input, as will be needed later.
    memcpypgm2ram(&config,(const rom void *) &DefaultGPIOsDescription[5].config,sizeof(config));
    //strcpypgm(GPIONameInRom,(const rom char *) DefaultGPIOsDescription[5].name);
    GPIONameInRom[0]='A';
    GPIONameInRom[1]='B';
    GPIONameInRom[2]='\0';
    auxil=20;
    printf("blacenas: %HS, %HS, %d.\r\n",DefaultGPIOsDescription[5].name, GPIONameInRom,auxil);

    STATUS_MSG("Starting GPIO driver test.\r\n\nFirst battery tests public driver functions for \
correct input validation\r\n");
    //invalid input test
 
    //1-NULL pointer
    STATUS_MSG("Now testing invalid pointer (NULL) arguments detection\r\n");
    /*In this test all functions should return error without executing any further*/
    results|=SetGPIOConfig(GPIONameInRom, NULL);
    results|=GetGPIOConfig(GPIONameInRom, NULL);
    results|=SetGPIOConfig(NULL, &config);
    results|=GetGPIOConfig(NULL, &config);
    results|=CompareGPIOConfig(NULL, &config);
    results|=CompareGPIOConfig(&config, NULL);
    results|=SetGPIO(NULL);
    results|=GetGPIO(NULL,&value);
    results|=GetGPIO(GPIONameInRom,NULL);
    results|=ToggleGPIO(NULL);
    results|=ResetGPIO(NULL);
    if(results)
    {
        STATUS_MSG("Invalid pointer detection test unsuccessful\r\n");
    }
    else
    {
        STATUS_MSG("Invalid pointer detection test successful\r\n");
    }
    
    //3-Non-existent GPIO name
    STATUS_MSG("Now testing non-existent name detection\r\n");
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
        STATUS_MSG("Non-existent name detection test unsuccessful\r\n");
    }
    else
    {
        STATUS_MSG("Non-existent name detection test successful\r\n");
    }
    
    //3-array out of bounds
    STATUS_MSG("Now testing array out of bounds detection\r\n");
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
        STATUS_MSG("Array out of bounds detection test unsuccessful\r\n");
    }
    else
    {
        STATUS_MSG("Array out of bounds detection test successful\r\n");
    }
    
    //4-Invalid Operations on GPIO
    STATUS_MSG("Now testing invalid operations detection\r\n");
    /*In this test the SetGPIO, ResetGPIO and ToggleGPIO operations should fail without interacting\
    with the I/O expanders, because the GPIO is configured as input*/
    SetGPIOConfig(GPIONameInRom,&config); //Configure the GPIo correctly as an input
    results=0;
    results|=SetGPIO(GPIONameInRom);
    results|=ToggleGPIO(GPIONameInRom);
    results|=ResetGPIO(GPIONameInRom);
    if(results)
    {
        STATUS_MSG("Invalid operations detection test unsuccessful\r\n");
    }
    else
    {
        STATUS_MSG("Invalid operations detection test successful\r\n");
    }
    
    //5-invalid config data
    STATUS_MSG("Now testing invalid config data detection\r\n");
    /*In this test all functions should detect the invalid configuration for the GPIO and return an\
    error, without interacting with the I/O expanders. Let's run this test around a valid GPIO name*/
    memset(&config,0x0,sizeof(config)); //invalid pin number
    results=0;
    memset(&config,0x1,sizeof(config)); //configuration data valid, but not matching GPIO's pin and expander
    results|=SetGPIOConfig(GPIONameInRom,&config);
    results|=SetGPIOConfig(GPIONameInRom,&config);
    memset(&config,0x5,sizeof(config)); //invalid exp, dir, pullup and inverted fields
    results|=SetGPIOConfig(GPIONameInRom,&config);
    memset(&config,0x11,sizeof(config)); //all fields invalid
    results|=SetGPIOConfig(GPIONameInRom,&config);
    if(results)
    {
        STATUS_MSG("Invalid config data detection test unsuccessful\r\n");
    }
    else
    {
        STATUS_MSG("Invalid config data detection test successful\r\n");
    }

    STATUS_MSG("Finished GPIO driver invalid input test\r\n\r\n");

    //functional test
    TEST_GPIODriverOperations();
}

/*
Tests GPIO sub-driver operations on individual IO expander pins.
Operates a sequential state-machine that mimics the GPIO sub-driver test procedure designed for the
GPIO operations part. Before the state machine runs, a GPIO configuration is set.
*/
void TEST_GPIODriverOperations(void)
{
    char name[]="@"; //name of the GPIO, defined in the config stage
    BYTE value=0xFF; //current value of the GPIO. Saves GetGPIO results
    BYTE i=0;
    
    //Used for one at a time testing
    static char namePersistent[]="@";
    static BYTE j=0;
    static BOOL TestOneAtATime=FALSE;
    
    switch(currentTest)
    {
        case config:
            //run a set of GPIO configurations
            TestOneAtATime = TEST_GPIODriverConfig();
            currentTest=set;
            break;
            
        case set:
            name[0]='@';
            for(i=0;i<32;i++)
            {
                name[0]++;
                if( !SetGPIO(name))
                {
                    STATUS_MSG("SetGPIO failed\r\n");
                }
            }
            STATUS_MSG("GPIO Test: Set complete. Please confirm if result is as expected.\
\r\nPress 'GA' <ENTER> to proceed to Reset operation\r\n");
            currentTest=reset;
            break;
            
        case reset:
            name[0]='@';
            for(i=0;i<32;i++)
            {
                name[0]++;
                if( !ResetGPIO(name))
                {
                    STATUS_MSG("ResetGPIO failed\r\n");
                }
            }
            STATUS_MSG("GPIO Test: Reset complete. Please confirm if result is as expected.\
\r\nPress 'GA' <ENTER> to proceed to 1st Toggle operation\r\n");
            currentTest=toggle1;
            break;
            
        case toggle1:
            name[0]='@';
            for(i=0;i<32;i++)
            {
                name[0]++;
                if( !ToggleGPIO(name))
                {
                    STATUS_MSG("1st ToggleGPIO failed\r\n");
                }
            }
            STATUS_MSG("GPIO Test: 1st Toggle complete. Please confirm if result is as expected.\
\r\nPress 'GA' <ENTER> to proceed to 2nd Toggle operation\r\n");
            currentTest=toggle2;
            break;

        case toggle2:
            name[0]='@';
            for(i=0;i<32;i++)
            {
                name[0]++;
                if( !ToggleGPIO(name))
                {
                    STATUS_MSG("2nd ToggleGPIO failed\r\n");
                }
            }
            STATUS_MSG("GPIO Test: 2nd Toggle complete. Please confirm if result is as expected.\
\r\nPress 'GA' <ENTER> to proceed to Get operation\r\n");
            currentTest=get;
            break;
            
        case get:
            name[0]='@';
            for(i=0;i<32;i++)
            {
                name[0]++;
                value=0xFF;
                if( !GetGPIO(name,&value))
                {
                    STATUS_MSG("GetGPIO failed\r\n");
                }
                STATUS_MSG("GPIO %d on expander J%d=%d\r\n", IOEXP_PIN_BIT_OFFSET((i+1)), \
                (i<16?5:6), value);
            }
            STATUS_MSG("GPIO Test: Get complete. Please confirm if result is as expected.\
\r\nPress 'GA' <ENTER> to proceed to next operation");
            if (TestOneAtATime)
            {
                STATUS_MSG("Feed pin 1 on connector J5 with 5V and Press 'GA' <ENTER> to read it\r\n");
                currentTest=getOneAtATime1;
                break;
            }
            currentTest=config;
            break;
            
        case getOneAtATime1:
            while(j<32)
            {
                namePersistent[0]++;
                value=0xFF;
                if( !GetGPIO(namePersistent,&value))
                {
                    STATUS_MSG("GetGPIO for individual pin failed\r\n");
                }
                STATUS_MSG("GPIO %d on bank J%d=%d. Confirm it reads as expected\r\n",
                (j<16?(j+1):(j-15)), (j<16?5:6), value);
                j++;
                STATUS_MSG("Now feed pin %d on connector J%d with 5V and Press 'GA' <ENTER> to read it\r\n",
                (j<16?(j+1):(j-15)), (j<16?5:6));
                break;
            }
            STATUS_MSG("GPIO Test: 5V input test complete. Connect now pin 1 on connector J5 to the\
ground and press 'GA' <ENTER> to read it\r\n");
            j=0;
            namePersistent[0]='@';
            currentTest=getOneAtATime2;
            break;
            
        case getOneAtATime2:
            while(j<32)
            {
                namePersistent[0]++;
                value=0xFF;
                if( !GetGPIO(namePersistent,&value))
                {
                    STATUS_MSG("GetGPIO for individual pin failed\r\n");
                }
                STATUS_MSG("GPIO %d on bank J%d=%d. Confirm it reads as expected\r\n",
                (j<16?(j+1):(j-15)), (j<16?5:6), value);
                j++;
                STATUS_MSG("Now feed pin %d on connector J%d to ground and Press 'GA' <ENTER> to read it\r\n",
                (j<16?(j+1):(j-15)), (j<16?5:6));
            }
            STATUS_MSG("GPIO Test: 0V input test complete.\
\r\nPress 'GA' <ENTER> to proceed to next configuration test\r\n");
            j=0;
            namePersistent[0]='@';
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
//XXX: implement getconfig testing
BOOL TEST_GPIODriverConfig(void)
{
    static GPIOConfigSM currentConfig = out;
    IOPinConfig configSet={0};
    IOPinConfig configGet={0};
    BYTE i=0;
    switch (currentConfig)
    {
        case out:
            STATUS_MSG("Starting GPIO driver testing \r\nConfiguring all io expanders's pins as \
output non-inverted without pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=OUT;
            configSet.pullup=OFF;
            configSet.inverted=OFF;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'GA' <ENTER> when ready to proceed to 'Set' operation\r\n");
            currentConfig=outPullup;
            return FALSE;
            break;
            
            
            
            
        case outPullup:
            STATUS_MSG("Configuring all io expanders pins as output non-inverted with pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=OUT;
            configSet.pullup=ON;
            configSet.inverted=OFF;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 0V;\r\n>5V after 'Set' operation;\
\r\n>0V after 'Reset' operation;\r\n>5V after 1st 'Toggle' operation;\r\n>0V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'GA' <ENTER> when ready to proceed to 'Set' operation\r\n");
            currentConfig=outInverted;
            return FALSE;
            break;
            
            
            
            
        case outInverted:
            STATUS_MSG("Configuring all io expanders pins as output inverted without pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=OUT;
            configSet.pullup=OFF;
            configSet.inverted=ON;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 5V;\r\n>0V after 'Set' operation;\
\r\n>5V after 'Reset' operation;\r\n>0V after 1st 'Toggle' operation;\r\n>5V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'GA' <ENTER> when ready to proceed to 'Set' operation\r\n");
            currentConfig=outPullupInverted;
            return FALSE;
            break;
            
            
            
            
        case outPullupInverted:
            STATUS_MSG("Configuring all io expanders pins as output inverted with pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=OUT;
            configSet.pullup=ON;
            configSet.inverted=ON;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, measure \
each pin's individual voltage after each of the following operations, and confirm it is as \
expected:\r\n\>Voltage at present should be 5V;\r\n>0V after 'Set' operation;\
\r\n>5V after 'Reset' operation;\r\n>0V after 1st 'Toggle' operation;\r\n>5V after 2nd 'Toggle' \
operation;\r\n>Voltage should not change after the 'Get' operation.\r\n\
Press 'GA' <ENTER> when ready to proceed to 'Set' operation\r\n");
            currentConfig=in;
            return FALSE;
            break;
            
            
            
            
        case in:
            STATUS_MSG("Configuring all io expanders pins as input non-inverted without pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=IN;
            configSet.pullup=OFF;
            configSet.inverted=OFF;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, repeat the\
following for each pin:\r\n>Run through the 'Set', 'Reset', 'Toggle' and 'Get' operations.\
Every pin should be floating;>Feed each pin with 5V and confirm  the computer measures it as '1'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm  the computer measures it as '0'. \
Repeat for every pin.\r\n\Press 'GA' <ENTER> to proceed to first operation\r\n");
            currentConfig=inPullup;
            return TRUE;
            break;
            
            
            
            
        case inPullup:
            STATUS_MSG("Configuring all io expanders pins as input non-inverted with pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=IN;
            configSet.pullup=ON;
            configSet.inverted=OFF;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, repeat the\
following for each pin:\r\n>Run through the 'Set', 'Reset', 'Toggle' and 'Get' operations.\
Every pin should be at 5V;>Feed each pin with 5V and confirm  the computer measures it as '1'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm  the computer measures it as '0'. \
Repeat for every pin.\r\n\Press 'GA' <ENTER> to proceed to first operation\r\n");
            currentConfig=inInverted;
            return TRUE;
            break;
            
            
            
            
        case inInverted:
            STATUS_MSG("Configuring all io expanders pins as input inverted without pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=IN;
            configSet.pullup=OFF;
            configSet.inverted=ON;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, repeat the\
following for each pin:\r\n>Run through the 'Set', 'Reset', 'Toggle' and 'Get' operations.\
Every pin should be floating;>Feed each pin with 5V and confirm  the computer measures it as '0'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm  the computer measures it as '1'. \
Repeat for every pin.\r\n\Press 'GA' <ENTER> to proceed to first operation\r\n");
            currentConfig=inPullupInverted;
            return TRUE;
            break;
            
            
            
            
        case inPullupInverted:
            STATUS_MSG("Configuring all io expanders pins as input inverted with pullup\r\n");
            
            GPIONameInRom[0]='@';    //Because why not??
            GPIONameInRom[1]='\0';
            configSet.exp=J5;
            configSet.number=0;
            configSet.dir=IN;
            configSet.pullup=ON;
            configSet.inverted=ON;
            for(i=1;i<33;i++)
            {
                GPIONameInRom[0]+=1;
                configSet.number+=1;
                if(i==17)
                {
                    configSet.number-=16;
                    configSet.exp=J6;
                }
                if( !SetGPIOConfig(GPIONameInRom,&configSet))
                {
                    ERROR_MSG("SetGPIOConfig encountered an error\r\n");
                }
                if( !GetGPIOConfig(GPIONameInRom, &configGet))
                {
                    ERROR_MSG("GetGPIOConfig encountered an error\r\n");
                }
                if( !CompareGPIOConfig(&configSet,&configGet))
                {
                    ERROR_MSG("The GPIO configuration just set does not match the current one\
for pin %d on I/O expander J%d\r\n",configSet.number, (configSet.exp==J5)?5:6 );
                }
            }
            
            STATUS_MSG("GPIO configuration complete. With the IO expanders disconnected, repeat the\
following for each pin:\r\n>Run through the 'Set', 'Reset', 'Toggle' and 'Get' operations.\
Every pin should be at 5V;>Feed each pin with 5V and confirm  the computer measures it as '0'. \
Repeat for every pin;\r\n>Connect each pin to ground and confirm  the computer measures it as '1'. \
Repeat for every pin.\r\n\Press 'GA' <ENTER> to proceed to first operation\r\n");
            currentConfig=end;
            return TRUE;
            break;
            
            
            
            
        case end:
            currentTest = end;
            AckCallback = NULL;
            //ROVIM_T2D_Init();
            //ROVIM_T2D_Start();
            STATUS_MSG("GPIO testing completed\r\n");
            break;
            
            
            
            
        default:
            FATAL_ERROR_MSG("Unexpected state of state machine\r\n");
            break;
    }
    return FALSE;
}

