//#line 1 "dalf_test.c"			//work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
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
	
	BYTE GPIOBank1;
	BYTE GPIOBank2;
	
	BYTE InitVerbosity = GetVerbosity();

// command line test - got it
	CMD = 0xA;
	ARGN = 0;
	TeCmdDispatchExt();
	
	CMD = 'C';
	ARGN = 0;
	TeCmdDispatchExt();
//

// Testing the time and a little bit stressing on the main, low priority task
	
	GetTime(&now);
	if(SCFG == TEcfg)
	{
		printf("Elapsed time = %lu\r\n",now.secs);
		printf("Elapsed ticks = %u\r\n",now.ticks);
	}
	
	//Now let's wait 10.3 secs before reprinting the time
	SetDelay(&then,2,msec_300);
	while(!Timeout(&then));
	if(SCFG == TEcfg)
	{
		printf("New Elapsed time = %lu\r\n",then.secs);
		printf("New Elapsed ticks = %u\r\n",then.ticks);
	}
	printf("\r\n");
//

	//test the macros for file and line number - for now the __FILE__ macro isn't processing the backslashes correctly
	if(SCFG == TEcfg)
	{
		printf("__FILE__: "__FILE__"; line: %d\r\n\r\n",__LINE__);
	}

	// verbosity test
	SetVerbosity(VERBOSITY_DISABLED);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	
	//Now, let's enable it one by one
	SetVerbosity(VERBOSITY_USE_TIMESTAMP);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	
	SetVerbosity(VERBOSITY_USE_CALL_INFO);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	
	SetVerbosity(VERBOSITY_LEVEL_ERROR);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_WARNING);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_STATUS);
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_DEBUG);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_LEVEL_DEBUG | VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_TIMESTAMP | VERBOSITY_USE_CALL_INFO);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR | VERBOSITY_USE_CALL_INFO);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_USE_TIMESTAMP | VERBOSITY_LEVEL_DEBUG);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_LEVEL_WARNING);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	FATAL_ERROR_MSG("Not really a fatal error. Verbosity = %02X\r\n",GetVerbosity());
	DEBUG_MSG("Not really a status message! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	
	printf("\r\n");
// /* verbosity test */

	/*IO expander testing - understood*/
	/* Bank1 = J6*/

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
	
	SetVerbosity(InitVerbosity);
	printf("\r\n");
//
	return;
}
