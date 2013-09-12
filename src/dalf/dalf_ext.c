/******************************************************************************
*******************************************************************************
**
**
**					dalf_ext.c - Generic software extensions and extended 
**							 hardware support for the original Dalf
**							 firmware.
**
**		This module extends the original firmware of the Dalf-1F motor 
**		control board to support generic hardware and software features
**		not included in the original version, that are not exclusive to
**		the T2D module.
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
#include "rovim.h"

extern	BYTE	CMD,ARG[16],ARGN;

void InitWatchdog(void)
{
	return;
}

void ServiceIO(void)
{
	return;
}

BYTE TeCmdDispatchExt(void)
{
	STATUS_CmdReceived();
	switch(CMD)
	{
		case 'H':
			return ShowHelp();
			break;
		case 'G':
			return CustomCmdDispatch();
			break;
		case 0xA:
			return TeProcessAck();
			break;
		default:
			return TeCmdDispatch();
			break;
	}
}

BYTE TE_CmdParseExt(void)
{
	BYTE err;
	//since, if the command is wrong, the CMD varible won't be updated, we have
	//to work around it. It isn't pretty, but the only option is to delve into
	//the asssembly. And we all know that is going to work out...
	CMD = 0xA;

	err = TE_CmdParse();

	if (CMD == 0xA)
	{
		return NoErr;
	}
	else
	{
		return err;
	}
}

BYTE I2C2CmdDispatchExt(void)
{
	return I2C2CmdDispatch();
}

BYTE ShowHelp(void)
{
	return eDisable;
}

BYTE TeProcessAck(void)
{
	return eDisable;
}

void RegisterCmdhelp(void)
{
	return;
}

void EmergencyStopMotors(void)
{
	CMD = 'O';
	ARGN = 0x00;
	TeCmdDispatch();
	return;
}

//XXX: MCC18 does not support inline functions. Should this be defined as a macro??
//XXX: measure
//WARNING: Make sure you do not call Any print macro inside this function. Otherwise...
void PrintTime(PTIME time)
{
	TIME now;
	if(NULL == time)
	{
		GetTime(&now);
		time = &now;
	}

	//no newlines nor carraige returns, because we want to print 
	printf("Elapsed Time (s): %lu.",time->secs);
	
}
