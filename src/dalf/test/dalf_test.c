#line 1 "dalf_test.c"			//work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
//cannot set breakpoints if this directive is used:
//info: http://www.microchip.com/forums/m105540-print.aspx
//uncomment only when breakpoints are no longer needed
/******************************************************************************
*******************************************************************************
**
**
**					test.c - Debug and test module of the ROVIM T2D and
**							Dalf-1F modules.
**
**		This module implements the debug information reporting and testing
**		funcionality of	the original Dalf-1F firmware and its ROVIM T2D
**		extension.
**		It covers status reports, warning and error information and
**		unitary and integrated test procedures.
**
**		This code was designed originally for the Dalf-1F motor control
**		board, the brain of the T2D module.
**		Original Dalf-1F firmware revision was 1.73.
**		See Dalf-1F owner's manual and the ROVIM T2D documentation for more 
**		details.
**
**			The ROVIM Project
*******************************************************************************
******************************************************************************/

#include "dalf.h"
#include "dalf_test.h"
#include <stdio.h>
#include <string.h>

//Testing features. This module is only compiled it the features are enabled.

void TEST_TestInit(void)
{
	return;
}

void TEST_InDevelopmentTesting(void)
{
	TIME now;
	TIME then;
	TIME later;
	DWORD ms=0;
	BYTE x=1;
	BYTE GPIOBank1;
	BYTE GPIOBank2;
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
	SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_USE_TIMESTAMP);
	STATUS_MSG("Initial read from GPIO bank 1, port B1\r\n");
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now writting FF's on the same GPIOs\r\n");
	WriteIOExp1(0x13,0xFF);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now writting 0x0F on the same GPIOs\r\n");
	WriteIOExp1(0x13,0x0F);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now writing 0x0F on the direction register of the same GPIOs\r\n");
	WriteIOExp1(0x01,0x0F);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now, with half of GPIOs direction changed, writting 0x33 on them\r\n");
	WriteIOExp1(0x13,0x33);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now, with 0x0F on the direction and 0x33 on the GPIOs value, write 0xAA on their polarity\r\n");
	WriteIOExp1(0x03,0xAA);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now, let's change the pull up. write 0x6C on that register\r\n");
	WriteIOExp1(0x0D,0x6C);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now,let's try to write 0xCC to the output latch and see what happens\r\n");
	WriteIOExp1(0x15,0xCC);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	
	STATUS_MSG("Now, with the output latches all pumped up, let's change the direction of the GPIOs and see what happens\r\n");
	WriteIOExp1(0x01,0xF0);
	GPIOBank1 = ReadIOExp1(0x01);
	STATUS_MSG("GPIO Direction=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x03);
	STATUS_MSG("Input Polarity=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0B);
	STATUS_MSG("Configuration=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x0D);
	STATUS_MSG("Weak Pullup=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x13);
	STATUS_MSG("GPIO Value=%02X\r\n",GPIOBank1);
	GPIOBank1 = ReadIOExp1(0x15);
	STATUS_MSG("Output Latch=%02X\r\n",GPIOBank1);
	printf("Expandable I/O testing done\r\n");
	
	SetVerbosity(InitVerbosity);
	printf("All testing done. Resuming normal operation\r\n");
//
	return;
}
