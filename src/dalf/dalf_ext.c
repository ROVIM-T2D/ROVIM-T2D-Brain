//#line 1 "dalf_ext.c"          //work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
//cannot set breakpoints if this directive is used:
//info: http://www.microchip.com/forums/m105540-print.aspx
//uncomment only when breakpoints are no longer needed
/******************************************************************************
*******************************************************************************
**
**
**                  dalf_ext.c - Generic software extensions and extended 
**                           hardware support for the original Dalf
**                           firmware.
**
**      This module extends the original firmware of the Dalf-1F motor 
**      control board to support generic hardware and software features
**      not included in the original version, that are not exclusive to
**      the T2D module.
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
#include "rovim.h"

//TODO: remove
//ExternalAppSupportFcts    ExternalFcts = {0};     //by default there is no external app
static BYTE verbosity = VERBOSITY_DISABLED;     //controls the verbosity of the debug information

WORD OL2Limit = 0;
WORD OL1Limit = 0;
WORD nol1 = 0;                      // Motor1 Open Loop step response output count
WORD nol2 = 0;                      // Motor2 Open Loop step response output count
///////////////////////////////////////////////////////////////////////////////
//Debug reporting features.
///////////////////////////////////////////////////////////////////////////////

void DEBUG_PrintCmd(void)
{
    BYTE i;

    if ((!(verbosity & VERBOSITY_LEVEL_DEBUG)) || (SCFG != TEcfg))
        return;

    if (CMD == 0xA) return;
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
    if (VERBOSITY_LEVEL_STATUS & level)
        verbosity |= VERBOSITY_LEVEL_STATUS;
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

/*
To convert ticks to ms we divide by 32 instead of 33, to speed the calculation.
This creates an error that maxes at 24ms, or 2.4%. This is an acceptable trade-off
*/
DWORD CalculateDelayMs(PTIME start, PTIME end)
{
    ULONG i=0,j=0;
    if ((start==NULL) || (end==NULL))
    {
        return -1;
    }
    i=TIME_TO_MSEC((*end));
    j=TIME_TO_MSEC((*start));
    if (j > i)
    {
        return -1;
    }
    return (DWORD)(i-j);
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
//system functions                                                           //
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
                /*custom functions index:
                this is to be used if we want other custom functions
                other than ROVIM_T2D ones. They must be inserted here, 
                to be parsed before calling the ROVIM dispatch*/
                default:
                    break;
            }
            return ROVIM_T2D_CustomCmdDispatch();
            break;
        /* XXX: this is an advanced feature. For now is unused
        case 0xA:
            return TeProcessAck();
            break;*/
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
    return TE_CmdParse();
    
    /*XXX: this is an advanced feature. For now is unused
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
    }*/
}

BYTE I2C2CmdDispatchExt(void)
{
    return I2C2CmdDispatch();
}

/*XXX: this is an advanced feature. For now is unused
BYTE TeProcessAck(void)
{
    return eDisable;
}*/

#ifdef HELP_ENABLED
// Help submodule ----------------------------------------------------------------------------------
BYTE ShowHelp(void)
{
    return eDisable;
}

void RegisterCmdhelp(void)
{
    return;
}

#endif

// Motor stopping submodule ------------------------------------------------------------------------
void EmergencyStopMotors(void)
{
    //XXX is there a more failsafe method to stop the motors??
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

// GPIO sub-driver ---------------------------------------------------------------------------------

BOOL SetGPIOConfig(const IOPinId* config)
{
    //this is not yet parameter-defined, as intended.
    WriteIOExp2(0x00, 0xAA);
    WriteIOExp2(0x01, 0x55);
    WriteIOExp2(0x02,0x10); //Set pin 5 as inverted
    WriteIOExp2(0x03,0x40); //set pin 10 as inverted
    // Disable pull-ups
    WriteIOExp2(0x0C,0x00);
    WriteIOExp2(0x0D,0x00);
}

//both parameters must point to valid variables. Space won't be allocated here
BOOL GetGPIOConfig(char* name, IOPinId* config)
{
    BYTE dir=0, pullup=0, inverted=0;
    
    if(!GetDefaultGPIOConfigbyName(name, config)){
        config=NULL;
        return FALSE;
    }
    
    if(config.exp == J5){
        dir=ReadIOExp2(0x00 + PIN_IN_BANK_B_OFFSET(config.number));
        inverted=ReadIOExp2(0x02 + PIN_IN_BANK_B_OFFSET(config.number));
        pullup=ReadIOExp2(0x0C + PIN_IN_BANK_B_OFFSET(config.number));
    }
    else{
        dir=ReadIOExp1(0x00 + PIN_IN_BANK_B_OFFSET(config.number));
        inverted=ReadIOExp1(0x02 + PIN_IN_BANK_B_OFFSET(config.number));
        pullup=ReadIOExp1(0x0C + PIN_IN_BANK_B_OFFSET(config.number));
    }
    dir= (dir & PIN_ACCESS_MASK(config.number)) >> PIN_ACCESS_OFFSET;
    inverted= (inverted & PIN_ACCESS_MASK(config.number)) >> PIN_ACCESS_OFFSET;
    pullup= (pullup & PIN_ACCESS_MASK(config.number)) >> PIN_ACCESS_OFFSET;
    config.dir= dir? IN: OUT;
    config.inverted= inverted? ON: OFF;
    config.pullup= pullup? ON: OFF;
    
    return TRUE;
}

//both parameters must point to valid variables. Space won't be allocated here
BOOL GetDefaultGPIOConfigbyName(char* name, IOPinId* config)
{
    for(i=0; i<ngpios; i++){
        if(strcmp(DefaultGPIOsConfig[i].name, name) == 0){
            memcpy(config,DefaultGPIOsConfig[i],sizeof(DefaultGPIOsConfig[i]));
            return TRUE;
        }
    }

    config=NULL;
    return FALSE;
}

BOOL SetGPIO(char* name)
{
    return TRUE;
}

BOOL ResetGPIO(char* name)
{
    return TRUE;
}

BOOL ToggleGPIO(char* name)
{
    return TRUE;
}

BOOL GetGPIO(char* name)
{
    return TRUE;
}

//--------------------------------------------------------------------------------------------------
//-------------------------------------Unused section-----------------------------------------------
//--------------------------------------------------------------------------------------------------

/*TODO: remove
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
}*/

#if 0
//TODO: to remove. Functions no longer needed
BYTE CmdExt_OpenLoopStepResp(void)
{
    BYTE err = NoErr;
    BYTE memstack[2] = {0};     //stack for temporary save of memory configurations
    WORD* mempoint[2] = {0};    //memory mapping of values saved on the stack
    WORD samples = 0;
    //Open loop motor step response
    //Command format: "G 1 n_samplesH n_samplesL [CmdX args]"
    
    //validade input
    //#args check: we can recover if the acc is not specified
    if ( !((ARGN >= 6) && (ARGN <= 7)) ) return eNumArgsErr;
    
    samples = ARG[1];
    samples = (samples << 8) | ARG[2];
    if (samples > MAXSAMPLES)
    {
        WARNING_MSG("Number of samples will be limited to the maximum permited, %d.\r\n", MAXSAMPLES);
        samples = MAXSAMPLES;
    }
    else if (!samples) return eParmErr;

    if (ARG[3] == 1)
    {
        Mtr1_Flags2 |= OL_stepresp;
        OL1Limit = samples;
    }
    else    //erroneous values will be detecting when executing the 'X' command
    {
        ARG[3] = 2;
        Mtr2_Flags2 |= OL_stepresp;
        OL2Limit = samples;
    }
    if (ARGN == 7)
    {
        WARNING_MSG("Overriding acceleration control input. Using maximum acceleration possible\r\n");
    }
    //print important information about step response measurement
    mempoint[0] = (WORD*) 0x0137;       //VSP1
    mempoint[1] = (WORD*) 0x014D;       //VSP2
    memstack[0] = *(mempoint[(ARG[3]-1)]);
    STATUS_MSG("Sample time = %d ms; number of samples = %d\r\n",memstack[0], samples);     //Heartbeat timer
    
    nol1 = nol2 = 0;
    mempoint[0] = (WORD*) 0x012B;       //AMINP
    memstack[0] = *(mempoint[0]);       //push the current memory configuration
    *(mempoint[0]) = ARG[3];

    //Do the motor movement command
    DEBUG_MSG("applying step to system input\r\n");
    /*err = */MoveMtrOpenLoop(ARG[3],ARG[4],ARG[5],1); //Let's limit the acceleration a bit, to avoid possible damage to the drivetrain
    DEBUG_MSG("Step applied to system input\r\n");

    *(mempoint[0]) = memstack[0];       //pull the saved memory configuration
    if (err != NoErr)
    {
        OL2Limit = OL1Limit = 0;
        Mtr1_Flags2 &= ~OL_stepresp;
        Mtr2_Flags2 &= ~OL_stepresp;
    }

    return err;
}

void OpenLoopTune2(void)    // Mtr2 PID Tuning Aid: Open Loop Verbose output    
{
    //BYTE index=0;

    ///////////////////////////////////////////////////////////////////////
    // Conditionally display position for Mtr2.                          //
    //                                                                   //
    //  Primary usage: See "G1" Cmd.                                     //
    //     Does nothing unless all conditions are met:                   //
    //       1) open Loop Movement is happening.                         //
    //       2) Verbose mode active.                                     //
    //                                                                   //
    ///////////////////////////////////////////////////////////////////////
    if( (Mtr2_Flags2 & OL_stepresp) &&
        (nol2 < OL2Limit) &&
        (MTR2_MODE3 & VerboseMsk))
    { // If something to do, ..
        //----------------------------
        if(CmdSource == TE_SrcMsk)
        { // TE Mode: Xmit line to TE
            nol2++;
            if(nol2==1) printf("OPEN LOOP STEP RESPONSE:2\r\n");
            printf("%3d:%+6Hd\r\n", nol2,encode2);
        } // If TE
        //----------------------------
        /* TODO: I2C interface not yet supported
        else if(CmdSource == I2C2_SrcMsk)
        { // I2C2 Mode: Xmit in packets that contain 8 samples.
            //
            // index: ptr to 3-byte pos field within DATA portion of pkt
            index = 2 + 3*(BYTE)(npid2 & 0x07);

            npid2++;    
            Pkt[index]=(Err2 & 0xFF);               // Low Byte
            Pkt[index+1]=((Err2 >> 8) & 0xFF);      // Mid Byte
            Pkt[index+2]=(Err2 >> 16);              // Hi Byte

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
                Pkt[1]=24;          // 8 Err samples (3 bytes per sample)
                PktLen=27;
                SendI2C2Pkt();      // Transmit Pkt[].
            }
        } // If I2C2 */
        //----------------------------
    } // something to do
    
    if ((nol2 == OL2Limit) && (Mtr2_Flags2 & OL_stepresp))
    {
        OL2Limit = 0;
        Mtr2_Flags2 &= ~OL_stepresp;
        nol2 = 0;
        SoftStop(2);
    }
}

void OpenLoopTune1(void)    // Mtr1 PID Tuning Aid: Open Loop Verbose output    
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
    if( (Mtr1_Flags2 & OL_stepresp) &&
        (nol1 < OL1Limit) &&
        (MTR1_MODE3 & VerboseMsk))
    { // If something to do, ..
        //----------------------------
        if(CmdSource == TE_SrcMsk)
        { // TE Mode: Xmit line to TE
            nol1++;
            if(nol1==1) printf("OPEN LOOP STEP RESPONSE:1\r\n");
            printf("%3d:%+6Hd\r\n", nol1,encode1);
        } // If TE
        //----------------------------
        /* TODO: I2C interface not yet supported
        else if(CmdSource == I2C2_SrcMsk)
        { // I2C2 Mode: Xmit in packets that contain 8 samples.
            //
            // index: ptr to 3-byte pos field within DATA portion of pkt
            index = 2 + 3*(BYTE)(npid2 & 0x07);

            npid2++;    
            Pkt[index]=(Err2 & 0xFF);               // Low Byte
            Pkt[index+1]=((Err2 >> 8) & 0xFF);      // Mid Byte
            Pkt[index+2]=(Err2 >> 16);              // Hi Byte

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
                Pkt[1]=24;          // 8 Err samples (3 bytes per sample)
                PktLen=27;
                SendI2C2Pkt();      // Transmit Pkt[].
            }
        } // If I2C2 */
        //----------------------------
    } // something to do
    
    if ((nol1 == OL1Limit) && (Mtr1_Flags2 & OL_stepresp))
    {
        OL1Limit = 0;
        Mtr1_Flags2 &= ~OL_stepresp;
        nol1 = 0;
        SoftStop(1);

    }
}
#endif
