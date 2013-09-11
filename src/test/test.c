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
#include "test.h"
#include <stdio.h>

extern	BYTE	CMD,ARG[16],ARGN;

static BYTE verbosity = VERBOSITY_DISABLED; //controls the verbosity of the debug information


//Debug reporting features.

void STATUS_CmdReceived(void)
{
	BYTE i;

	if (!(verbosity & VERBOSITY_LEVEL_STATUS))
		return;

	printf("STATUS: Cmd received: %c. # arguments: %d. Arguments: ", CMD, ARGN);
	for(i=0;i<ARGN;i++)
	{
		printf("%02X, ",ARG[i]);
	}
	printf("\r\n");
}

void SetVerbosity(BYTE level)
{
	verbosity = VERBOSITY_DISABLED;

	if (VERBOSITY_LEVEL_ERROR & level)
		verbosity |= VERBOSITY_LEVEL_ERROR;
	if (VERBOSITY_LEVEL_WARNING & level)
		verbosity |= VERBOSITY_LEVEL_WARNING;
	if (VERBOSITY_LEVEL_STATUS & level)
		verbosity |= VERBOSITY_LEVEL_STATUS;
}

BYTE GetVerbosity(void)
{
	return verbosity;
}


//Testing features. This module is only compiled it the features are enabled.
//XXX: For now it is allways enabled. To remove
#define TESTING_ENABLED

void TEST_TestInit(void)
{
//If you do not have testing enabled, we can stop right here
#ifndef TESTING_ENABLED
	return;
#endif

}

#ifdef TESTING_ENABLED

void TEST_GenericTesting(void)
{
/* verbosity test - passed
	//Let's garantee the verbosity is disabled
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
	
	//Now, let's enable it one by one
	SetVerbosity(VERBOSITY_LEVEL_ERROR);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_DISABLED);
	SetVerbosity(VERBOSITY_LEVEL_WARNING);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_DISABLED);
	SetVerbosity(VERBOSITY_LEVEL_STATUS);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_DISABLED);
	SetVerbosity(VERBOSITY_LEVEL_STATUS | VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_DISABLED);
	SetVerbosity(VERBOSITY_LEVEL_WARNING | VERBOSITY_LEVEL_ERROR);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());

	SetVerbosity(VERBOSITY_DISABLED);
	SetVerbosity(VERBOSITY_LEVEL_WARNING);
	STATUS_MSG("Hello, Dalf-1F World! Verbosity=%02X\r\n",GetVerbosity());
	WARNING_MSG("OMFG, it works! Verbosity=%02X\r\n",GetVerbosity());
	ERROR_MSG("Gotcha! It's not a real error, sherlock! Verbosity=%02X\r\n",GetVerbosity());
*/ /* verbosity test */
	return;
}

void TEST_SimulateSerialCommand(BYTE cmd, BYTE* arg, BYTE argn)
{
	return;
}

#endif
