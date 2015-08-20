//#line 1 "rovim_t2d.c"         //work around the __FILE__ screwup on windows, http://www.microchip.com/forums/m746272.aspx
//cannot set breakpoints if this directive is used:
//info: http://www.microchip.com/forums/m105540-print.aspx
//uncomment only when breakpoints are no longer needed
/******************************************************************************
*******************************************************************************
**
**
**                  rovim_t2d.c - "tracção, travagem e direcção" (T2D) 
**                          module of the ROVIM project.
**
**      This module builds on and extends the firmware of the Dalf-1F motor 
**      control board to implement the T2D module of the ROVIM project.
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

#include    "p18f6722.h"
#include "rovim.h"
#include "dalf.h"
#include <stdio.h>
#include <string.h>


#pragma romdata defaultgpiosconfig
//there was no space in this file's standard initialized data section for this structure, so I had
//to move it to program data.
rom const IOPinId DefaultGPIOsConfig[]={
    /*name,                exp, number,   dir,   pullup, inverted*/
    { "brake",              J5,    1,      OUT,    OFF,   OFF }, //controls progressive braking on SigmaDrive, when on auto mode
    { "unused",             J5,    2,      INOUT,  OFF,   OFF },
    { "accelerate",         J5,    3,      OUT,    OFF,   OFF }, //controls accelerator on SigmaDrive, when on auto mode
    { "unused",             J5,    4,      INOUT,  OFF,   OFF },
    { "activate traction",  J5,    5,      OUT,    OFF,   ON  }, //activates SigmaDrive safety switch, when on auto mode (active low)
    { "error traction",     J5,    6,      IN,     OFF,   OFF }, //detects error on SigmaDrive controller
    { "unlock brake",       J5,    7,      OUT,    OFF,   OFF }, //fully unlocks rear-wheel brake
    { "emergency switch",   J5,    8,      IN,     OFF,   OFF }, //detects when emergency switch is pressed
    { "reverse",            J5,    9,      OUT,    OFF,   OFF }, //engages reverse on SigmaDrive, when on auto mode
    { "dead man trigger",   J5,    10,     IN,     OFF,   ON  }, //detects activation of dead man trigger (active low)
    { "forward",            J5,    11,     OUT,    OFF,   OFF }, //engages forward on SigmaDrive, when on auto mode
    { "error direction",    J5,    12,     IN,     OFF,   OFF }, //detects when direction position is out-of-bounds
    { "lock brake",         J5,    13,     OUT,    OFF,   OFF }, //fully locks rear-wheel brake
    { "unused",             J5,    14,     INOUT,  OFF,   OFF },
    { "unused",             J5,    15,     INOUT,  OFF,   OFF },
    { "auto mode",          J5,    16,     IN,     OFF,   OFF }  //detects when auto mode switch is turned on
};

const BYTE ngpios= (BYTE) (sizeof(DefaultGPIOsConfig)/sizeof(DefaultGPIOsConfig[0]));
#pragma romdata

//configure basic ROVIM features needed early on. To be called as soon as possible
void ROVIM_T2D_Init(void)
{
    //SetExternalAppSupportFcts(ROVIM_T2D_Greeting, ROVIM_T2D_CustomCmdDispatch, ROVIM_T2D_ServiceIO);
    ioexpcount=-1; //disable IO exp sampling for now
    SetVerbosity (INIT_VERBOSITY_LEVEL);
    ROVIM_T2D_ConfigGPIOs();
    return;
}

//Start all remaining ROVIM features.
void ROVIM_T2D_Start(void)
{
    ioexpcount = IO_SAMPLE_PERIOD;
}

void ROVIM_T2D_ConfigGPIOs(void)
{
    BYTE i=0;
    IOPinId config={0};
    
    for (i=0; i<ngpios; i++) {
        //copy configuration to data ram before calling setup function
        memcpypgm2ram(&config,&(DefaultGPIOsConfig)[i],sizeof(config));
        SetGPIOConfig(&config);
    }
    
    STATUS_MSG("GPIOs configuration complete \r\n");
    return;
}

void ROVIM_T2D_Greeting(void)
{
    Greeting();
    if(SCFG == TEcfg) 
    { // If Terminal Emulator Interface
        printf("ROVIM T2D Brain\r\n");
        printf("ROVIM T2D Software Ver:%2u.%u\r\n",ROVIM_T2D_SW_MAJOR_ID, ROVIM_T2D_SW_MINOR_ID);   // ROVIM Software ID
        printf("ROVIM T2D Contact(s):\r\n"ROVIM_T2D_CONTACTS"\r\n");                                // ROVIM Contacts
        printf("\r\n");
    }
}

BYTE ROVIM_T2D_CustomCmdDispatch(void)
{
    switch(ARG[0])
    {
        case CustomCmdIdOffset:
            ROVIM_T2D_LockBrake();
            return NoErr;
        case (CustomCmdIdOffset + 1):
            ROVIM_T2D_UnlockBrake();
            return NoErr;
            break;
        //Add more ROVIM commands here
        default:
            return eParseErr;
    }
    return eDisable;
}

void ROVIM_T2D_Lockdown(void)
{
    //XXX: Should I disable interrupts during this function? Since, in theory, a motor control
    //command can appear before I lock the access to motors...
    //I could also start by locking acess and then create a special command similar to 'O'
    //XXX is there a more failsafe method to stop the motors??
    CMD = 'O';
    ARGN = 0x00;
    TeCmdDispatchExt();
    
    //Once we engage the handbrake, the traction engine cannot work, so we're safe
    ROVIM_T2D_LockBrake();
    _LED3_ON;               // Visual error indication due to the brake being locked
    
    LockMotorsAccess();
}

void ROVIM_T2D_ReleaseFromLockdown(void)
{
    ROVIM_T2D_UnlockBrake();
    UnlockMotorsAccess();
    _LED3_OFF;
    
}

BOOL ROVIM_T2D_LockBrake(void)
{
    TIME now;
    TIME then;

    STATUS_MSG("Braking now\r\n");
    
    SetGPIO("lock brake");
    ResetGPIO("unlock brake");
    //Despite having a hw timer, we still need a sw timer, to make sure we don't start debraking
    //before time (debraking has priority over braking)
    //XXX: Set a timer to lock the brake
    
    return TRUE;
}

BOOL ROVIM_T2D_UnlockBrake(void)
{
    TIME now;
    TIME then;

    STATUS_MSG("Unlocking brake now\r\n");
    
    ResetGPIO("lock brake");
    SetGPIO("unlock brake");
    //XXX: Set a timer to reset the unlock brake GPIO.
    //XXX: Temporary solution. Wait 5.3s
    GetTime(&now);
    SetDelay(&then,5,msec_300);
    while(!Timeout(&then));
    ResetGPIO("unlock brake");
    
    return TRUE;
}

BOOL ROVIM_T2D_ValidateInitialState(void)
{
    //my birthday's june 16th. I have an amazon wishlist. No pressure, though...
    return TRUE;
}

void ROVIM_T2D_ServiceIO(void)
{
    BYTE J5A=0, J5B=0, J6A=0, J6B=0;

    if(!GetAllGPIO(&J5A, &J5B, &J6A, &J6B))
    {
        FATAL_ERROR_MSG("ROVIM_T2D_ReadVehicleState: Could not read GPIOs.\r\n");
    }
    return;
}
