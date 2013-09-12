/******************************************************************************
*******************************************************************************
**
**
**					rovim_t2d.c - "trac��o, travagem e direc��o" (T2D) 
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
#include "test.h"

// only needed for testing purposed
#include <stdio.h>

extern	BYTE	CMD,ARG[16],ARGN;

void ROVIM_T2D_Init(void)
{
	return;
}

void ROVIM_T2D_Greeting(void)
{
		printf("ROVIM T2D Brain\r\n");
		printf("ROVIM T2D Software Ver:%2u.%u\r\n",ROVIM_T2D_SW_MAJOR_ID, ROVIM_T2D_SW_MINOR_ID);	// ROVIM Software ID
		printf("ROVIM T2D Contact(s):\r\n"ROVIM_T2D_CONTACTS"\r\n");								// ROVIM Contacts
		printf("\r\n");
}

BOOL ROVIM_T2D_LockMotorsAccess(void)
{
	return TRUE;
}

BOOL ROVIM_T2D_UnlockMotorsAccess(void)
{
	return TRUE;
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

BYTE CustomCmdDispatch(void)
{
	return eDisable;
}