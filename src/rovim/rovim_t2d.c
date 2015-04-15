//#line 1 "rovim_t2d.c"			//work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
//cannot set breakpoints if this directive is used:
//info: http://www.microchip.com/forums/m105540-print.aspx
//uncomment only when breakpoints are no longer needed
/******************************************************************************
*******************************************************************************
**
**
**					rovim_t2d.c - "tracção, travagem e direcção" (T2D) 
**							module of the ROVIM project.
**
**		This module builds on and extends the firmware of the Dalf-1F motor 
**		control board to implement the T2D module of the ROVIM project.
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


#include "rovim.h"
#include "dalf.h"
#include <stdio.h>


void ROVIM_T2D_Init(void)
{
	//SetExternalAppSupportFcts(ROVIM_T2D_Greeting, ROVIM_T2D_CustomCmdDispatch, ROVIM_T2D_ServiceIO);
	ioexpcount = IO_SAMPLE_PERIOD;
	SetVerbosity (INIT_VERBOSITY_LEVEL);
	
	return;
}

void ROVIM_T2D_Greeting(void)
{
	Greeting();
	if(SCFG == TEcfg) 
	{ // If Terminal Emulator Interface
		printf("ROVIM T2D Brain\r\n");
		printf("ROVIM T2D Software Ver:%2u.%u\r\n",ROVIM_T2D_SW_MAJOR_ID, ROVIM_T2D_SW_MINOR_ID);	// ROVIM Software ID
		printf("ROVIM T2D Contact(s):\r\n"ROVIM_T2D_CONTACTS"\r\n");								// ROVIM Contacts
		printf("\r\n");
	}
}

BYTE ROVIM_T2D_CustomCmdDispatch(void)
{
	
	switch(CMD)
	{
		case CustomCmdIdOffset:
			break;
		case (CustomCmdIdOffset + 1):
			break;
		//Add more ROVIM commands here
		default:
			return eParseErr;
	}
	return eDisable;
}

BOOL ROVIM_T2D_LockBrake(void)
{
	return TRUE;
}

BOOL ROVIM_T2D_UnlockBrake(void)
{
	return TRUE;
}

BOOL ROVIM_T2D_ReadVehicleState(void)
{
	return TRUE;
}

BOOL ROVIM_T2D_ValidateInitialState(void)
{
	return TRUE;
}

void ROVIM_T2D_ServiceIO(void)
{
	return;
}
