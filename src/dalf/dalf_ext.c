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

ExternalAppSupportFcts	ExternalFcts = {0};		//by default there is no external app
static BYTE verbosity = VERBOSITY_DISABLED; //controls the verbosity of the debug information


///////////////////////////////////////////////////////////////////////////////
//Debug reporting features.
///////////////////////////////////////////////////////////////////////////////

void STATUS_PrintCmd(void)
{
	BYTE i;

	if ((!(verbosity & VERBOSITY_LEVEL_STATUS2)) || (SCFG != TEcfg))
		return;

	STATUS2_MSG("Cmd received: %c. # arguments: %d. Arguments: ", CMD, ARGN);
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
	if (VERBOSITY_LEVEL_STATUS1 & level)
		verbosity |= VERBOSITY_LEVEL_STATUS1;
	if (VERBOSITY_LEVEL_STATUS2 & level)
		verbosity |= VERBOSITY_LEVEL_STATUS2;
	if (VERBOSITY_USE_CALL_INFO & level)
		verbosity |= VERBOSITY_USE_CALL_INFO;
	if (VERBOSITY_USE_TIMESTAMP & level)
		verbosity |= VERBOSITY_USE_TIMESTAMP;
}

BYTE GetVerbosity(void)
{
	return verbosity;
}

PTIME GetCurrentTime(void)
{
	TIME now;
	GetTime(&now);
	return &now;
}


///////////////////////////////////////////////////////////////////////////////
//Logging features. This module is only compiled it the features are enabled.
///////////////////////////////////////////////////////////////////////////////


#ifdef LOG_ENABLED

void LOG_LogInit(void)
{
	return;
}

#endif


///////////////////////////////////////////////////////////////////////////////
//system functions															 //
///////////////////////////////////////////////////////////////////////////////

void SystemInitExt(void)
{
	//This has to be the first action of this function!!
	SystemInit();
	//TODO: Set io sample period to a default, if the io poll is defined.
	//TODO: find out if we just recovered from a hard reset (watchdog) or it's a power-on
	return;
}

#ifdef WATCHDOG_ENABLED

void InitWatchdog(void)
{
	return;
}

void KickWatchdog(void)
{
	return;
}

#endif

BYTE SetExternalAppSupportFcts(greeting GreetingFctPtr, cmdExtensionDispatch
	CmdExtensionDispatchFctPtr, serviceIO ServiceIOFctPtr)
{
	if( (GreetingFctPtr == NULL) || (CmdExtensionDispatchFctPtr == NULL) || (ServiceIOFctPtr == NULL))
	{
		ERROR_MSG("Trying to register a NULL function\r\n");
		return eParmErr;
	}

	ExternalFcts.GreetingFct = GreetingFctPtr;
	ExternalFcts.CmdExtensionDispatchFct = CmdExtensionDispatchFctPtr;
	ExternalFcts.ServiceIOFct = ServiceIOFctPtr;

	return NoErr;
}

ExternalAppSupportFcts* GetExternalAppSupportFcts(void)
{
	return &ExternalFcts;
}

BYTE TeCmdDispatchExt(void)
{
	STATUS_PrintCmd();
	switch(CMD)
	{
		case 'H':
		#ifdef HELP_ENABLED
			return ShowHelp();
		#else
			return eDisable;
		#endif
			break;
		case 'G':
			if (ExternalFcts.CmdExtensionDispatchFct)
			{
				return ExternalFcts.CmdExtensionDispatchFct();
			}
			else
			{
				return CmdExtensionDispatch();
			}
			break;
		case 0xA:
			return TeProcessAck();
			break;
		default:
			return TeCmdDispatch();
			break;
	}
	return eDisable;
}

//XXX: the parameters have to be passed in the function, because of the external ones.
BYTE CmdExtensionDispatch(void)
{
	return eDisable;
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

#ifdef HELP_ENABLED

BYTE ShowHelp(void)
{
	return eDisable;
}

#endif

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
	TeCmdDispatchExt();
	return;
}

void LockMotorsAccess(void)
{
	return;
}

void UnlockMotorsAccess(void)
{
	return;
}
