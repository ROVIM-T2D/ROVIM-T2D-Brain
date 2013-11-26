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

WORD OL2Limit = 0;
WORD OL1Limit = 0;
///////////////////////////////////////////////////////////////////////////////
//Debug reporting features.
///////////////////////////////////////////////////////////////////////////////

void DEBUG_PrintCmd(void)
{
	BYTE i;

	if ((!(verbosity & VERBOSITY_LEVEL_DEBUG)) || (SCFG != TEcfg))
		return;

	if (ARGN == 0) return;
	DEBUG_MSG("Cmd received: %c. # arguments: %d. Arguments: ", CMD, ARGN);
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
	if (VERBOSITY_LEVEL_DEBUG & level)
		verbosity |= VERBOSITY_LEVEL_DEBUG;
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

/**
\Brief Call the actions required to execute a command
*/
BYTE TeCmdDispatchExt(void)
{
	DEBUG_PrintCmd();
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
			switch (ARG[0])
			{
				case 1:
					return CmdExt_OpenLoopStepResp();
					break;
				default:
					break;
			}
			if (ExternalFcts.CmdExtensionDispatchFct)
			{
				return ExternalFcts.CmdExtensionDispatchFct();
			}
			return eParseErr;
			break;
		case 0xA:
			return TeProcessAck();
			break;
		default:
			return TeCmdDispatch();
			break;
	}
	return eParseErr;
}

/**
\Brief Parse the characters from the uart buffer into variables describing a command
*/
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

BYTE CmdExt_OpenLoopStepResp(void)
{
	BYTE err = NoErr;
	BYTE memstack[2] = {0};		//stack for temporary save of memory configurations
	BYTE* mempoint[2] = {0};	//memory mapping of values saved on the stack
	WORD samples = 0;
	//Open loop motor step response
	//Command format: "G 1 n_samples [CmdX args]"
	
	//validade input
	//#args check: we expect one or two bytes for the number of samples, and we can recover if the acc is not specified
	if ( !((ARGN >= 5) && (ARGN <= 7)) ) return eNumArgsErr;
	
	samples = (ARG[1] & 0xFF00) | (ARG[2] && 0x00FF);
	if (samples > MAXSAMPLES)
	{
		WARNING_MSG("Number of samples will be limited to the maximum permited, %c.\r\n", MAXSAMPLES);
		samples = MAXSAMPLES;
	}
	//prepare command for execution
	if (ARG[3] == '1')
	{
		Mtr1_Flags2 |= OL_stepresp;
		OL1Limit = samples;
	}
	else	//erroneous values will be detecting when executing the 'X' command
	{
		Mtr2_Flags2 |= OL_stepresp;
		OL2Limit = samples;
	}
	CMD = 'X';
	ARG[0] = ARG[2];
	ARG[1] = ARG[3];
	ARG[2] = ARG[4];
	ARG[3] = 1;			//Let's limit the acceleration a bit, to avoid possible damage to the drivetrain
	ARGN -= 2;
	if (ARGN == 3)
	{
		ARGN++;
	}
	else
	{
		ARGN = 4;		//just making sure
		WARNING_MSG("Overriding acceleration control input for maximum acceleration possible, 0\r\n");
	}
	DEBUG_PrintCmd();
	STATUS1_MSG("Sample time = 1ms; number of samples = %d\r\n",samples); 	//Heartbeat timer
	mempoint[0] = (BYTE*) 0x012B;		//AMINP
	memstack[0] = *(mempoint[0]);		//push the current memory configuration

	//Do the motor movement command
	err = TeCmdDispatchExt();
	
	*(mempoint[0]) = memstack[0];		//pull the saved memory configuration
	if (err != NoErr)
	{
		OL2Limit = OL1Limit = 0;
		Mtr1_Flags2 &= ~OL_stepresp;
		Mtr2_Flags2 &= ~OL_stepresp;
	}

	return err;
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

void OpenLoopTune2(void)	// Mtr2 PID Tuning Aid: Open Loop Verbose output	
{
	BYTE index=0;

	///////////////////////////////////////////////////////////////////////
	// Conditionally display position for Mtr2.                          //
	//                                                                   //
	//  Primary usage: See "G1" Cmd.		                             //
	//     Does nothing unless all conditions are met:                   //
	//       1) open Loop Movement is happening.                         //
	//       2) Verbose mode active.                                     //
	//                                                                   //
	///////////////////////////////////////////////////////////////////////
	if(	(Mtr2_Flags2 & OL_stepresp) &&
		(MTR2_MODE3 & VerboseMsk))
	{ // If something to do, ..
		//----------------------------
		if(CmdSource == TE_SrcMsk)
		{ // TE Mode: Xmit line to TE
			index++;
			if(index==1) printf("STEP RESPONSE:2\r\n");
			printf("%3d:%+6Hd\r\n", index,encode2);
		} // If TE
		//----------------------------
		/* TODO: I2C interface not yet supported
		else if(CmdSource == I2C2_SrcMsk)
		{ // I2C2 Mode: Xmit in packets that contain 8 samples.
			//
			// index: ptr to 3-byte pos field within DATA portion of pkt
			index = 2 + 3*(BYTE)(npid2 & 0x07);

			npid2++;	
			Pkt[index]=(Err2 & 0xFF);				// Low Byte
			Pkt[index+1]=((Err2 >> 8) & 0xFF);		// Mid Byte
			Pkt[index+2]=(Err2 >> 16);				// Hi Byte

			// Special case: Test for partial last packet
			if( ( npid2 == Pid2Limit ) && ( index != 23 ) )
			{
				for ( i = index; i <= 23; i+=3 )
				{ // Zero fill unused portion of DATA field
					Pkt[i]=0; Pkt[i+1]=0; Pkt[i+2]=0;
				}
				index = 23;
			}

			if( index == 23 )
			{ // Pkt full.  Time to send
				Pkt[0]='Q';
				Pkt[1]=24;			// 8 Err samples (3 bytes per sample)
				PktLen=27;
				SendI2C2Pkt();		// Transmit Pkt[].
			}
		} // If I2C2 */
		//----------------------------
	} // something to do
	
	if (index == OL2Limit)
	{
		OL2Limit = 0;
		CMD = 'O';
		ARGN = 0;
		TeCmdDispatch();
	}
}

void OpenLoopTune1(void)	// Mtr1 PID Tuning Aid: Open Loop Verbose output	
{
	BYTE index=0;

	///////////////////////////////////////////////////////////////////////
	// Conditionally display position for Mtr1.                          //
	//                                                                   //
	//  Primary usage: See "G1" Cmd.                                     //
	//     Does nothing unless all conditions are met:                   //
	//       1) open Loop Movement is happening.                         //
	//       2) Verbose mode active.                                     //
	//                                                                   //
	///////////////////////////////////////////////////////////////////////
	if(	(Mtr1_Flags2 & OL_stepresp) &&
		(MTR1_MODE3 & VerboseMsk))
	{ // If something to do, ..
		//----------------------------
		if(CmdSource == TE_SrcMsk)
		{ // TE Mode: Xmit line to TE
			index++;
			if(index==1) printf("STEP RESPONSE:1\r\n");
			printf("%3d:%+6Hd\r\n", index,encode1);
		} // If TE
		//----------------------------
		/* TODO: I2C interface not yet supported
		else if(CmdSource == I2C2_SrcMsk)
		{ // I2C2 Mode: Xmit in packets that contain 8 samples.
			//
			// index: ptr to 3-byte pos field within DATA portion of pkt
			index = 2 + 3*(BYTE)(npid2 & 0x07);

			npid2++;	
			Pkt[index]=(Err2 & 0xFF);				// Low Byte
			Pkt[index+1]=((Err2 >> 8) & 0xFF);		// Mid Byte
			Pkt[index+2]=(Err2 >> 16);				// Hi Byte

			// Special case: Test for partial last packet
			if( ( npid2 == Pid2Limit ) && ( index != 23 ) )
			{
				for ( i = index; i <= 23; i+=3 )
				{ // Zero fill unused portion of DATA field
					Pkt[i]=0; Pkt[i+1]=0; Pkt[i+2]=0;
				}
				index = 23;
			}

			if( index == 23 )
			{ // Pkt full.  Time to send
				Pkt[0]='Q';
				Pkt[1]=24;			// 8 Err samples (3 bytes per sample)
				PktLen=27;
				SendI2C2Pkt();		// Transmit Pkt[].
			}
		} // If I2C2 */
		//----------------------------
	} // something to do
	
	if (index == OL1Limit)
	{
		OL1Limit = 0;
		CMD = 'O';
		ARGN = 0;
		TeCmdDispatch();
	}
}