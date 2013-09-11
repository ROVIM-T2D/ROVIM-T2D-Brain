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
#include "rovim_t2d.h"

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
	//TEST_ReverseEngineer();
	switch(CMD)
	{
		case 'H':
			return Help();
			break;
		case 'G':
			return CustomCmdDispatch();
			break;
		default:
			return TeCmdDispatch();
			break;
	}
}

BYTE I2C2CmdDispatchExt(void)
{
	return I2C2CmdDispatch();
}

BYTE Help(void)
{
	return eDisable;
}
