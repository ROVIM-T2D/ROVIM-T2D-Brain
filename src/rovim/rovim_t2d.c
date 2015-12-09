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

#include "p18f6722.h"
#include "rovim.h"
#include "dalf.h"
#include <stdio.h>
#include <string.h>

#pragma romdata DefaultGPIOsDescription
/* Since MCC18 cannot create RAM objects larger than 256 bytes, we must create a specific section
for this data structure. This was done in ROM, since this is only read-only data.
Also, storing the GPIO name in ROM adds a lot of convenience when calling driver functions*/
rom const IOPinDescription DefaultGPIOsDescription[]={
    /*name,                             exp, number,   dir,   pullup, inverted*/
    /*controls progressive braking on SigmaDrive, when on auto mode. Used as PWM switch to feed 
analogue input. This should be used to slow down the vehicle during regular operation.*/
    { "decelerator",                    { J5,    1,      OUT,    OFF,   OFF }},
    /*detects when electric brake has reached unclamp side end-of-travel switch. Logic '1' indicates
 the electric brake is fully unclamped.*/
    { "brake unclamp switch",           { J5,    2,      IN,     OFF,   ON  }},
    /*controls accelerator on SigmaDrive, when on auto mode. Used as PWM switch to feed analogue input.*/
    { "accelerator",                    { J5,    3,      OUT,    OFF,   OFF }},
    /*detects when the SigmaDrive B+ pin is being powered. Logic '1' indicates the SigmaDrive 
is turned on and the fuse and contactor are OK.*/
    { "traction voltage sensor",        { J5,    4,      IN,     OFF,   OFF }},
    /*controls SigmaDrive foot switch, when on auto mode. Active low: logic '0' means the switch is 
pressed and the vehicle is ready to move.*/
    { "activate traction",              { J5,    5,      OUT,    OFF,   OFF }},
    /*detects when electric brake has reached clamp side end-of-travel switch. Logic '1' indicates 
the electric brake is fully clamped and a lockdown cause (emergency switch, dead man trigger, 
etc.) is active.*/
    { "brake clamp switch",             { J5,    6,      IN,     OFF,   OFF }},
    /*controls electric brake unclamp. To properly unclamp electric brake, this signal has to be 
active until unclamp side end-of-travel is reached.*/
    { "brake unclamper",                { J5,    7,      OUT,    OFF,   OFF }},
    /*detects when direction position reaches either of the side end-of-travel switches. Logic '1' 
means end-of-travel is switched on and direction is on an erroneous state.*/
    { "direction error switch",         { J5,    8,     IN,     OFF,   OFF  }},
    /*controls reverse switch on SigmaDrive, when on auto mode. Active low: logic '0' means the 
 switch is pressed and reverse drive is engaged. This switch cannot be active simultaneously with 
 "engage forward", pin 11.*/
    { "engage reverse",                 { J5,    9,      OUT,    OFF,   OFF }},
    /*detects when there is a command to unclamp the brake, either by sw or hw. This may regardless 
off the brake clamp/unclamp switches state. Logic '1' indicates there is such a command.*/
    { "brake unclamp command",          { J5,    10,     IN,     OFF,   OFF }},
    /*controls forward switch on SigmaDrive, when on auto mode. Active low: logic '0' means the 
 switch is pressed and forward drive is engaged. This switch cannot be active simultaneously with 
 "engage reverse", pin 9.*/
    { "engage forward",                 { J5,    11,     OUT,    OFF,   OFF }},
    /*detects when manual/auto switch is on auto mode. When auto mode is engaged, the
 control signals to the SigmaDrive controller must be provided by this program. Logic '1' means auto
 mode is engaged*/
    { "auto mode switch",               { J5,    12,     IN,     OFF,   OFF }},
    /*controls electric brake clamping. To brake electric brake, only a pulse from this signal is 
needed.*/
    { "brake clamper",                  { J5,    13,     OUT,    OFF,   OFF }},
    /*detects an emergency stop condition, regarless of the source. Logic '1' indicates the switch 
is pressed and the brake is trying to clamp.*/
    { "emergency stop condition",       { J5,    14,     IN,     OFF,   OFF }},
    /*Unused*/
    { "engage handbrake",               { J5,    15,     OUT,    OFF,   OFF }},
    /*Unused*/
    { "",                               { J5,    16,     IN,     OFF,   OFF }},
};

const rom BYTE nDefaultgpios=(BYTE) (sizeof(DefaultGPIOsDescription)/sizeof(DefaultGPIOsDescription[0]));
#pragma romdata //resume rom data allocation on the standard section

static BOOL ResourcesLockFlag=FALSE;
static const BYTE ROVIM_T2D_CommandsToAllow[]={
    ROVIM_T2D_LOCKDOWN_CMD_CODE,
    ROVIM_T2D_RELEASE_CMD_CODE,
    ROVIM_T2D_DECELERATE_CMD_CODE,
    ROVIM_T2D_SET_MOVEMENT_CMD_CODE,
    ROVIM_T2D_DEBUG_CTRL_CMD_CODE
    };
static const BYTE ROVIM_T2D_nCommandsToAllow=(BYTE) (sizeof(ROVIM_T2D_CommandsToAllow)/sizeof(ROVIM_T2D_CommandsToAllow[0]));

WORD    ROVIM_T2D_sysmonitorcount;            // ROVIM T2D system state monitoring timeout counter;
WORD    ROVIM_T2D_pwmrefreshcount;            // ROVIM T2D PWM refresh timeout counter;
//XXX: Change this when running normal
BOOL    ManualSysMonitoring=TRUE;
BYTE    DebugPWM=0;

//The pointer to easily switch configurations
rom const IOPinDescription* GPIOsDescription = DefaultGPIOsDescription;
BYTE ngpios=0;

//Duty cycle for the traction accelerador and decelerator 
static BYTE AccDutyCycle=0;
static BYTE DccDutyCycle=0;
static unsigned int WaitForAccDccDecay=0;
static BYTE PeriodCnt=0;

//Brake locl/unlock flags
static BOOL inLockdown=FALSE;
static BOOL UnlockingBrake=FALSE;
static WORD MotorStressMonitor[3]={0,0,0};

//vehicle movement description
static long settlingTime=0;
static movement desiredMovement={0};
static BYTE movementType=HILLHOLD;
long acc1=0,vel1=0;

//configure basic ROVIM features needed early on. To be called as soon as possible
void ROVIM_T2D_Init(void)
{
    ROVIM_T2D_sysmonitorcount=-1; //"disable" IO exp sampling for now
    SetVerbosity (INIT_VERBOSITY_LEVEL);
    ROVIM_T2D_ConfigSerialPort();
    ROVIM_T2D_ConfigGPIOs();
    ROVIM_T2D_LockUnusedResourcesAccess();
    //Initial values for the GPIOS will be set on the lockdown version
    ROVIM_T2D_ConfigDefaultParamBlock();

    DEBUG_MSG("ROVIM T2D initialization complete.\r\n");
    return;
}

void ROVIM_T2D_LockUnusedResourcesAccess(void)
{
    ERROR_MSG("LockUnusedResourcesAccess not implemented.\r\n");
}

void ROVIM_T2D_ConfigSerialPort(void)
{
    BYTE nBR=0;
    /*If the serial port isn't configured with the correct baud rate, configure it now and reboot, to
    force new configuration. This is only needed when the default parameter block is restored.*/
    nBR = ReadExtEE_Byte(0x006D);
    if(nBR != ROVIM_T2D_NBR)
    {
        WARNING_MSG("Configuring dalf baud rate parameter, nBR, to %d (see dalf owner's manual \
for details). If you're reading this, you have to reconfigure the terminal emulator to the new \
baud rate.\r\n", ROVIM_T2D_NBR);
        WriteExtEE_Byte(0x006D,ROVIM_T2D_NBR);
        Reset();
    }
    STATUS_MSG("Dalf baud rate parameter set to %d. If you can read this properly, you don't \
need to worry about it nor the gibberish printed above, if any.\r\n",ROVIM_T2D_NBR);
}

void ROVIM_T2D_ConfigDefaultParamBlock(void)
{
    ROVIM_T2D_ConfigDirParamBlock();
    
    //Traction encoder set up
    //set TPR1 LSB
    DEBUG_MSG("Setting TPR1 to %d ticks/rev. LSB=0x%02X, MSB=0x%02X.\r\n",ROVIM_T2D_TRACTION_TPR, ROVIM_T2D_TRACTION_TPR, 0);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x40;
    ARG[3]=ROVIM_T2D_TRACTION_TPR;
    TeCmdDispatchExt();
    //set TPR1 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x41;
    ARG[3]=0;
    TeCmdDispatchExt();
    //set VSP1
    DEBUG_MSG("Setting VSP1 to %d ms.\r\n",ROVIM_T2D_VSP1);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x37;
    ARG[3]=ROVIM_T2D_VSP1;
    TeCmdDispatchExt();
    //TODO: VBCAL
    //set VBWARN LSB
    DEBUG_MSG("Setting VBWARN to %d mV. LSB=0x%02X, MSB=0x%02X.\r\n",ROVIM_T2D_VBWARN, (BYTE) (ROVIM_T2D_VBWARN>>8), (BYTE) (ROVIM_T2D_VBWARN & 0xFF));
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x29;
    ARG[3]=(BYTE) (ROVIM_T2D_VBWARN>>8);
    TeCmdDispatchExt();
    //set VBWARN MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x2A;
    ARG[3]=(BYTE) (ROVIM_T2D_VBWARN & 0xFF);
    TeCmdDispatchExt();
}

void ROVIM_T2D_ConfigDirParamBlock(void)
{
    //set MAXERR LSB
    DEBUG_MSG("Setting MAXERR to 0x%02X. LSB=0x%02X, MSB=0x%02X.\r\n",
    (ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT), 
    (ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT), 0);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x2C;
    ARG[3]=(ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT);
    TeCmdDispatchExt();
    //set MAXERR MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x2D;
    ARG[3]=0;
    TeCmdDispatchExt();
    //set MTR2_MODE1
    DEBUG_MSG("Setting MTR2_MODE1 to 0x%02X.\r\n",ROVIM_T2D_DIR_MODE1);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x46;
    ARG[3]=ROVIM_T2D_DIR_MODE1;
    TeCmdDispatchExt();
    //set MTR2_MODE2
    DEBUG_MSG("Setting MTR2_MODE2 to 0x%02X.\r\n",ROVIM_T2D_DIR_MODE2);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x47;
    ARG[3]=ROVIM_T2D_DIR_MODE2;
    TeCmdDispatchExt();
    //set MTR2_MODE3
    DEBUG_MSG("Setting MTR2_MODE3 to 0x%02X.\r\n",ROVIM_T2D_DIR_MODE3);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x48;
    ARG[3]=ROVIM_T2D_DIR_MODE3;
    TeCmdDispatchExt();
    //set ACC2 LSB
    DEBUG_MSG("Setting ACC2 to %d ticks/VSP2^2. LSB=0x%02X, MSB=0x%02X.\r\n",ACC2, (BYTE) (ACC2>>8), (BYTE) (ACC2 & 0xFF));
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x49;
    ARG[3]=(BYTE) (ACC2 & 0xFF);
    TeCmdDispatchExt();
    //set ACC2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4A;
    ARG[3]=(BYTE) (ACC2>>8);
    TeCmdDispatchExt();
    //set VMID2 LSB
    DEBUG_MSG("Setting VMID2 to %d ticks/VSP2. LSB=0x%02X, MSB=0x%02X.\r\n",VMID2, (BYTE) (VMID2>>8), (BYTE) (VMID2 & 0xFF));
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4B;
    ARG[3]=(BYTE) (VMID2 & 0xFF);
    TeCmdDispatchExt();
    //set VMID2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4C ;
    ARG[3]=(BYTE) (VMID2>>8);
    TeCmdDispatchExt();
    //set VSP2
    DEBUG_MSG("Setting VSP2 to %d ms.\r\n",ROVIM_T2D_DIR_VSP);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4D;
    ARG[3]=ROVIM_T2D_DIR_VSP;
    TeCmdDispatchExt();
    //set VMIN2
    DEBUG_MSG("Setting VMIN2 to %d %%.\r\n",ROVIM_T2D_DIR_MIN_PWM);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x54;
    ARG[3]=ROVIM_T2D_DIR_MIN_PWM;
    TeCmdDispatchExt();
    //set VMAX2
    DEBUG_MSG("Setting VMAX2 to %d %%.\r\n",ROVIM_T2D_DIR_MAX_PWM);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x55;
    ARG[3]=ROVIM_T2D_DIR_MAX_PWM;
    TeCmdDispatchExt();
    //set TPR2 LSB
    DEBUG_MSG("Setting TPR2 to %d ticks/rev. LSB=0x%02X, MSB=0x%02X.\r\n",256, 0, 1);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x56;
    ARG[3]=0x00;    //According to Dalf OM, TRP must be set to 0x100 for analog encoders
    TeCmdDispatchExt();
    //set TPR2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x57;
    ARG[3]=0x01;    //According to Dalf OM, TRP must be set to 0x100 for analog encoders
    TeCmdDispatchExt();
    //set MIN2 LSB
    DEBUG_MSG("Setting MIN2 to 0x%2X. LSB=0x%02X, MSB=0x%02X.\r\n",ROVIM_T2D_DIR_TICK_LOWER_LIMIT, ROVIM_T2D_DIR_TICK_LOWER_LIMIT, 0);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x58;
    ARG[3]=ROVIM_T2D_DIR_TICK_LOWER_LIMIT;
    TeCmdDispatchExt();
    //set MIN2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x59;
    ARG[3]=0;
    TeCmdDispatchExt();
    //set MAX2 LSB
    DEBUG_MSG("Setting MAX2 to 0x%2X. LSB=0x%02X, MSB=0x%02X.\r\n",ROVIM_T2D_DIR_TICK_UPPER_LIMIT, ROVIM_T2D_DIR_TICK_UPPER_LIMIT, 0);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x5A;
    ARG[3]=ROVIM_T2D_DIR_TICK_UPPER_LIMIT;
    TeCmdDispatchExt();
    //set MAX2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x5B;
    ARG[3]=0;
    TeCmdDispatchExt();
    //set DMAX
    DEBUG_MSG("Setting DMAX to 0x%02X.\r\n",ROVIM_T2D_DMAX);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x75;
    ARG[3]=ROVIM_T2D_DMAX;
    TeCmdDispatchExt();
    //set FENBL
    DEBUG_MSG("Setting FENBL to 0x%02X.\r\n",ROVIM_T2D_FENBL);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x76;
    ARG[3]=ROVIM_T2D_FENBL;
    TeCmdDispatchExt();
    //set DECAY
    DEBUG_MSG("Setting DECAY to 0x%02X [If ADC sample time params are default: Fc=-ln(DECAY/256)*220/2/pi].\r\n",ROVIM_T2D_DECAY);
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x77;
    ARG[3]=ROVIM_T2D_DECAY;
    TeCmdDispatchExt();
}

//Start all remaining ROVIM features.
void ROVIM_T2D_Start(void)
{
    ROVIM_T2D_sysmonitorcount = ROVIM_T2D_SYSTEM_MONITOR_PERIOD;
    ROVIM_T2D_pwmrefreshcount = ROVIM_T2D_PWM_REFRESH_PERIOD;
}

void ROVIM_T2D_ConfigGPIOs(void)
{
    BYTE i=0;
    IOPinConfig config={0};
    
    GPIOsDescription = DefaultGPIOsDescription;
    ngpios= nDefaultgpios;

    for (i=0; i<ngpios; i++) {
        //copy configuration to data ram before calling setup function
        if(strcmppgm("",GPIOsDescription[i].name)==0)
        {
            DEBUG_MSG("GPIO number %d unused.\r\n",GPIOsDescription[i].config.number);
            continue;
        }
        memcpypgm2ram(&config,(const rom void *) &GPIOsDescription[i].config,sizeof(config));
        SetGPIOConfig(GPIOsDescription[i].name,&config);
    }
    
    return;
}

void ROVIM_T2D_Greeting(void)
{
    if(SCFG == TEcfg) 
    { // If Terminal Emulator Interface
        Greeting();
        printf("ROVIM T2D Brain\r\n");
        printf("ROVIM T2D Software Ver:%2u.%u\r\n",ROVIM_T2D_SW_MAJOR_ID, ROVIM_T2D_SW_MINOR_ID);   // ROVIM Software ID
        printf("ROVIM T2D Contact(s):\r\n"ROVIM_T2D_CONTACTS"\r\n");                                // ROVIM Contacts
        printf("\r\n");
    }
}

BYTE ROVIM_T2D_CmdDispatch(void)
{
    if( ROVIM_T2D_IsCommandLocked(ARG[0]) )
    {
        return eDisable;
    }
    switch(ARG[0])
    {
        case ROVIM_T2D_LOCKDOWN_CMD_CODE:
            return ROVIM_T2D_Lockdown();
        case ROVIM_T2D_RELEASE_CMD_CODE:
            return ROVIM_T2D_ReleaseFromLockdown();
        case ROVIM_T2D_CONTROL_GPIO_CMD_CODE:
            return ROVIM_T2D_ControlGPIO();
        case ROVIM_T2D_ACCELERATE_CMD_CODE:
            return ROVIM_T2D_Accelerate();
        case ROVIM_T2D_DECELERATE_CMD_CODE:
            return ROVIM_T2D_Decelerate();
        case ROVIM_T2D_SET_MOVEMENT_CMD_CODE:
            return ROVIM_T2D_SetMovement();
        case ROVIM_T2D_TURN_CMD_CODE:
            return ROVIM_T2D_Turn();
        case ROVIM_T2D_DEBUG_CTRL_CMD_CODE:
            return ROVIM_T2D_DebugControl();
        //Add more ROVIM commands here
        default:
            ERROR_MSG("Command does not exist.\r\n");
            return eParseErr;
    }
    ERROR_MSG("Unexpected program execution.\r\n");
    return eDisable;
}

void ROVIM_T2D_LockCriticalResourcesAccess(void)
{
    ResourcesLockFlag=TRUE;
    return;
}

void ROVIM_T2D_UnlockCriticalResourcesAccess(void)
{
    ResourcesLockFlag=FALSE;
    return;
}

BOOL ROVIM_T2D_IsCommandLocked(BYTE cmd)
{
    BYTE i;
    
    if(!ResourcesLockFlag)
    {
        return FALSE;
    }
    
    for(i=0;i<ROVIM_T2D_nCommandsToAllow;i++)
    {
        if( cmd==ROVIM_T2D_CommandsToAllow[i])
        {
            DEBUG_MSG("Found cmd=%d is in whitelist position %d.\r\n", cmd, i);
            return FALSE;
        }
    }
    ERROR_MSG("Command is locked.\r\n");
    return TRUE;
}

BOOL ROVIM_T2D_FinishReleaseFromLockdown(void)
{
    UnlockCriticalResourcesAccess();
    //ROVIM_T2D_ConfigDefaultParamBlock(); //see if with soft stop this isn't needed
    _LED3_OFF;
    ResetGPIO("brake unclamper");
    inLockdown=FALSE;
    STATUS_MSG("ROVIM is now ready to move. Restart traction controller to clear any remaining \
    error\r\n");
    
    return TRUE;
}

BOOL ROVIM_T2D_LockBrake(void)
{
    DEBUG_MSG("Locking emergency brake now.\r\n");
    SetGPIO("brake clamper");
    ResetGPIO("brake unclamper");
    
    return TRUE;
}

BOOL ROVIM_T2D_UnlockBrake(void)
{   
    //Uncomment this message when braking is done automatically
    //DEBUG_MSG("Unlocking brake now.\r\n");
    ResetGPIO("brake clamper");
    /*Depending on hw configuration, the unclamp pin may be unwired, so force this action to be done
    manually. Regardless of that, the unlock procedure is the same*/
    SetGPIO("brake unclamper");

    return TRUE;
}

//my birthday's june 16th. I have an amazon wishlist. No pressure, though...
BOOL ROVIM_T2D_ValidateState(void)
{
    BYTE directionError=0;
    BYTE emergencyStop=0;
    WORD delay=0;
    TIME now;
    //Since the system always goes to lockdown on power up, this initial value is accurate enough
    static TIME before={0};
    
    /*I'm choosing not to monitor the brake clamp & unclamp & unclamp command here because:
    the user may start unclamping before sending the command; while unclamping, there is nothing 
    I can do to stop that; after unclamping, if there is still a related (with these 3 GPIOs) 
    error condition, it will be picked up by the monitor task*/
    
    GetGPIO("direction error switch",&directionError);
    if(directionError)
    {
        ERROR_MSG("The direction end-of-travel switch is still ON.\r\n");
        DEBUG_MSG("directionError=%d.\r\n",directionError);
        return FALSE;
    }
    if(vel1)
    {
        ERROR_MSG("The traction is still moving.\r\n");
        DEBUG_MSG("vel1=%ld.\r\n",vel1);
        return FALSE;
    }
    if(Power2)
    {
        ERROR_MSG("The direction is still being powered.\r\n");
        DEBUG_MSG("Power2=%d, (Mtr2_Flags1 & pida_Msk)=%d.\r\n", Power2, (Mtr2_Flags1 & pida_Msk));
        return FALSE;
    }
    
    /*Get the brake clamper GPIO. Since it depends on the emergency stop condition being ON (as
    happens during lockdown) and it's pin is connected to a monostable circuit, we need to wait
    until the monostable brings the value down before we can get a clean reading*/
    GetTime(&now);
    delay=CalculateDelayMs(&before, &now);
    if(ROVIM_T2D_BRAKE_CLAMP_TIME > delay)
    {
        ERROR_MSG("Brake clamper monostable is still ON. Do not forget that it activates every \
time you call this command. Wait %d ms before trying again.\r\n",
        ROVIM_T2D_BRAKE_CLAMP_TIME);
        DEBUG_MSG("delay=%d,timeout=%d.\r\n",delay, ROVIM_T2D_BRAKE_CLAMP_TIME);
        return FALSE;
    }
    ResetGPIO("brake clamper");
    GetGPIO("emergency stop condition",&emergencyStop);
    /*We shouldn't decide here to release the brake, so we just put it back and start counting for the next time*/
    SetGPIO("brake clamper");
    GetTime(&before);   
    
    if(emergencyStop)
    {
        ERROR_MSG("There is still an emergency stop condition active.\r\n");
        return FALSE;
    }
    
    DEBUG_MSG("Inputs are good. Make sure outputs are, too.\r\n");
    //Stop motors
    SoftStop(2);
    SoftStop(3);

    DEBUG_MSG("Vehicle is ready to be unclamped.\r\n");
    return TRUE;    //the vehicle is good to go
}


//--------------------------------Periodic tasks of system monitoring-------------------------------
void ROVIM_T2D_MonitorSystem(void)
{
    BYTE previousVerbosity=0;
    BYTE error=0;
    
    TIME start={0}, stop={0};
    static TIME prev={0};
    DWORD delay=0;
    static DWORD maxDelay=0;
    GetTime(&start);
    
    previousVerbosity = GetVerbosity();
    if (!ManualSysMonitoring)   //in manual mode we do not need to restrict verbosity
    {
        SetVerbosity(VERBOSITY_LEVEL_ERROR | VERBOSITY_LEVEL_WARNING);
    }
    
    //Look for unrecoverable error conditions
    if( ROVIM_T2D_DetectFatalError(ROVIM_T2D_SYSTEM_MONITOR_PERIOD))
    {
        //XXX:What should we do on a fatal error?
        #ifdef WATCHDOG_ENABLED
        HardReset();
        #else //WATCHDOG_ENABLED
        //Soft reset
        CMD='I';
        ARGN=0;
        TeCmdDispatchExt();
        #endif //WATCHDOG_ENABLED
    }
    
    //Look for severe error conditions
    error  = ROVIM_T2D_DetectTractionEror(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    error |= ROVIM_T2D_DetectBrakingError(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    error |= ROVIM_T2D_DetectDirectionEror(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    if(error)
    {
        ROVIM_T2D_Lockdown();
    }
    else
    {
        DEBUG_MSG("Yay, there is no need to go to lockdown.\r\n");
    }
    
    //look for and act on non-severe error conditions
    ROVIM_T2D_MonitorTractionWarning(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    ROVIM_T2D_MonitorBrakingWarning(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    ROVIM_T2D_MonitorDirectionWarning(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    
    //look for and act on brake unlock completion
    ROVIM_T2D_MonitorBrakeUnlock(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);

    //Detect and act on assynchronous switch to manual mode
    ROVIM_T2D_MonitorManualMode(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    
    ROVIM_T2D_PendingCmd(ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    
    if (!ManualSysMonitoring)
    {
        SetVerbosity(previousVerbosity);
    }
    
    //time measurements - debug purposes
    GetTime(&stop);
    delay=CalculateDelayMs(&start,&stop);
    if(delay > maxDelay)
    {
        maxDelay=delay;
        DEBUG_MSG("Monitor task max delay so far: %ld ms.\r\n",maxDelay);
    }
    //DEBUG_MSG("Monitor task delay: %ld ms.\r\n",delay);
    delay=CalculateDelayMs(&prev,&start);
    if(delay < ROVIM_T2D_SYSTEM_MONITOR_PERIOD)
    {
        DEBUG_MSG("Monitoring period smaller than expected. measured=%ld, expected=%ld.\r\n", delay, ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    }
    //DEBUG_MSG("Monitoring period measured=%ld, expected=%ld.\r\n", delay, (long)ROVIM_T2D_SYSTEM_MONITOR_PERIOD);
    GetTime(&prev);
    
    return;
}

    //////////////////////////////////////////////////////////////////////
    //      P E N D I N G    C M D    S E R V I C E S                   //
    //                                                                  //
    //  If motor now stopped (Power1=0 or Power2=0), and pending        //
    //  cmd is awaiting motor stopped, do it now.                       //
    //////////////////////////////////////////////////////////////////////
void ROVIM_T2D_PendingCmd(BYTE period)
{
    BYTE dutyCycle=0;
    
    /* XXX:i don't think this is needed here- to test
    if ( (desiredMovement.type==HILLHOLD) && (!vel1)  && (movementType!=HILLHOLD) )
    {
        //engage hill holder when vehicle is set on hold and has reached a standstill. Do it only once
        ResetGPIO("engage handbrake");
        SetGPIO("engage forward");
        SetGPIO("engage reverse");
        SetGPIO("activate traction");
        movementType=HILLHOLD;
        STATUS_MSG("Vehicle on hold.\r\n");
    }
    */
    
    if ( (desiredMovement.type==FORWARD) && (!vel1)  && (movementType!=FORWARD) )
    {
        //pins are active low. Order of activation is important here.
        SetGPIO("engage handbrake");
        ResetGPIO("activate traction");
        SetGPIO("engage reverse");
        ResetGPIO("engage forward");
        ROVIM_T2D_SetSpeed(desiredMovement.speed);
        movementType=FORWARD;
        STATUS_MSG("Vehicle moving forward at approximately %d km/h/10.\r\n",desiredMovement.speed);
    }
    
    if ( (desiredMovement.type==REVERSE) && (!vel1)  && (movementType!=REVERSE) )
    {
        //pins are active low. Order of activation is important here.
        SetGPIO("engage handbrake");
        ResetGPIO("activate traction");
        SetGPIO("engage forward");
        ResetGPIO("engage reverse");
        ROVIM_T2D_SetSpeed(desiredMovement.speed);
        movementType=REVERSE;
        STATUS_MSG("Vehicle moving backwards at approximately %d km/h/10.\r\n",desiredMovement.speed);
    }
    DEBUG_MSG("desired type=%d, speed=%d, current type=%d, vel1=%ld, FW=%d,RV=%d.\r\n",
        desiredMovement.type, desiredMovement.speed, movementType, vel1, FORWARD,REVERSE);
    //neutral mode can be done immediately. There's no need to wait until the vehicle stops.
}

void ROVIM_T2D_MonitorManualMode(BYTE period)
{
    static BOOL manualMode=FALSE;
    BYTE autoSwitch=0;
    
    GetGPIO("auto mode switch",&autoSwitch);
    
    if((!autoSwitch) && (!manualMode))
    {
        //Make sure the outputs are in a good state
        DEBUG_MSG("Detected manual mode of operation.\r\n");
        manualMode=TRUE;
        SoftStop(2);
        SoftStop(3);
    }
    if (autoSwitch) 
    {
        if(manualMode)
        {
            DEBUG_MSG("Detected automatic mode of operation.\r\n");
        }
        manualMode=FALSE;
    }
    
}

//detects when traction system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectTractionEror(BYTE period)
{
    static WORD movingOnHoldTimeout=0;
    
    /*XXX: Not ready yet. Must be done only with closed loop speed control
    if ((desiredMovement.speed < vel1) && (settlingTime > ROVIM_T2D_MAX_SETTLING_TIME))
    {
        ERROR_MSG("Vehicle is taking too long to reduce its speed.\r\n");
        DEBUG_MSG("Desired vel=%d, vel=%ld, settlingTime=%d.\r\n", desiredMovement.speed, vel1, settlingTime);
        return TRUE;
    }*/
    if (vel1 > ROVIM_T2D_CRITICAL_SPEED)
    {
        ERROR_MSG("Vehicle is moving too fast.\r\n");
        DEBUG_MSG("vel=%ld, max speed=%ld.\r\n", vel1, ROVIM_T2D_CRITICAL_SPEED);
        return TRUE;
    }
    if ((abs(acc1)) > ROVIM_T2D_CRASH_ACC_THRESHOLD)
    {
        ERROR_MSG("Vehicle is accelerating too fast.\r\n");
        DEBUG_MSG("acc=%ld, max acc=%d.\r\n", acc1, ROVIM_T2D_CRASH_ACC_THRESHOLD);
        return TRUE;
    }
    if ((desiredMovement.type==HILLHOLD) && (vel1))
    {
        movingOnHoldTimeout+=period;
    }
    else
    {
        movingOnHoldTimeout=0;
    }
    if (movingOnHoldTimeout>ROVIM_T2D_MOVING_ON_HOLD_TIMEOUT)
    {
        ERROR_MSG("Vehicle should be on hold, yet it is moving.\r\n");
        DEBUG_MSG("movement type=%d, vel=%ld, timeout=%d.\r\n", desiredMovement.type, vel1, ROVIM_T2D_MOVING_ON_HOLD_TIMEOUT);
        return TRUE;
    }

    DEBUG_MSG("No traction errors detected.\r\n");
    return FALSE;
}

//detects when braking system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectBrakingError(BYTE period)
{
    BYTE endOfTravelClamp=0, endOfTravelUnclamp=0, unclampAction=0;
    BYTE emergencyStop=0;
    
    GetGPIO("brake clamp switch",&endOfTravelClamp);
    GetGPIO("brake unclamp switch",&endOfTravelUnclamp);
    GetGPIO("emergency stop condition",&emergencyStop);
    GetGPIO("brake unclamper",&unclampAction);
    
    if(emergencyStop)
    {
        //detected emergency condition - Error!
        ERROR_MSG("There is an emergency stop condition.\r\n");
        DEBUG_MSG("emergency command=%d. Remember this will occur while you have the 'brake clamper' ON.\r\n", emergencyStop);
        return TRUE;
    }
    if((!endOfTravelClamp) && (!endOfTravelUnclamp) && (!unclampAction))
    {
        ERROR_MSG("Brake is neither locked nor unlocked.\r\n");
        DEBUG_MSG("clamp end-of-travel=%d, unclamp end-of-travel=%d, unclamping=%d.\r\n",endOfTravelClamp, endOfTravelUnclamp,unclampAction);
        return TRUE;
    }
    
    DEBUG_MSG("No braking errors detected.\r\n");
    return FALSE;
}

//detects when direction system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectDirectionEror(BYTE period)
{
    BYTE endOfTravel=0;

    /* This check only makes sense if we can cut power to the direction motor when going to lockdown.
    Currently we can't do so, so this stays commented.
    pos=ADC0[3];
    if ((pos > ROVIM_T2D_DIRECTION_CRITICAL_UPPER_POSITION) || 
        (pos < ROVIM_T2D_DIRECTION_CRITICAL_LOWER_POSITION))
    {
        //reached soft direction position limit - Error!
        return TRUE;
    }*/
    
    GetGPIO("direction error switch",&endOfTravel);
    if (endOfTravel)
    {
        //reached hard direction position limit - Error!
        ERROR_MSG("Direction has reached one end-of-travel switch.\r\n");
        DEBUG_MSG("Switch=%d.\r\n", endOfTravel);
        return TRUE;
    }
    
    DEBUG_MSG("No direction errors detected.\r\n");
    return FALSE;
}

void ROVIM_T2D_MonitorTractionWarning(BYTE period)
{
    //DEBUG_MSG("No traction warnings detected.\r\n");
}

void ROVIM_T2D_MonitorBrakingWarning(BYTE period)
{
    //DEBUG_MSG("No braking warnings detected.\r\n");
}

void ROVIM_T2D_MonitorDirectionWarning(BYTE period)
{
    /* Detect if the motor is under a big load for a long period of time.
    Since we do not have current sensors for the motors, we try to emulate that monitoring the
    PWM duty cycle. If it stays for too long too high, it means the system is not being able to
    reach a stable state, and the behaviour is in some sort erroneous.
    We chose the absolute number (since power up) of stressed "samples" as a metric instead of
    the relative (since last move command) because it reduces complexity (communication between
    functions is simplified). Also we believe in the long term this count should accuratelly reflect
    the amount of stress the motor is having.
    NOTE: We should be doing this check every at VSP intervals...*/
    /*
    //Only detect the stress if the motor if PID is active
    if (Mtr2_Flags1 & pida_Msk)
    {
        // Due to the nature of PID control, the motor may not be at full power and still be in stress
        if (Power2 > (VMAX1/ROVIM_T2D_DIRECTION_MOTOR_STRESS_DUTY_CYCLE_THRESHOLD))
        {
            MotorStressMonitor[2]++;
        }
        if (MotorStressMonitor[2] > ROVIM_T2D_DIRECTION_MOTOR_STRESS_CNT_THRESHOLD)
        {
            //reached direction motor overstress condition - Error!
            ERROR_MSG("ROVIM direction motor is too stressed. Going to stop it for precaution.\r\n");
            DEBUG_MSG("MotorStressMonitor[2]=%d, threshold # stress events=%d.\r\n", 
            MotorStressMonitor[2], ROVIM_T2D_DIRECTION_MOTOR_STRESS_CNT_THRESHOLD);
            return;
        }
    }*/
    //DEBUG_MSG("No direction warnings detected.\r\n");
}

//detects when the system has encountered an error from which it cannot recover
BOOL ROVIM_T2D_DetectFatalError(WORD period)
{
    static WORD unclampActionActiveTime=0;
    BYTE unclampAction=0;
    BYTE endOfTravelClamp=0;
    BYTE emergencyStop=0;
    
    GetGPIO("brake clamp switch",&endOfTravelClamp);
    GetGPIO("emergency stop condition",&emergencyStop);
    GetGPIO("brake unclamp command",&unclampAction);
    
    /*XXX: Let's ignore this for now. It is not that relevant to the quality of the software
    if(unclampAction)
    {
        unclampActionActiveTime+=period;
    }
    else
    {
        unclampActionActiveTime=0;
    }
    if(unclampActionActiveTime > ROVIM_T2D_FATAL_UNCLAMP_TIMEOUT)
    {
        FATAL_ERROR_MSG("User is pressing the button for too long - or something else really, really \
bad happened.\r\n");
        DEBUG_MSG("Timeout=%d, time passed=%d.\r\n",ROVIM_T2D_FATAL_UNCLAMP_TIMEOUT, unclampActionActiveTime));
        return TRUE;
    }*/
    
    /* Need to have a timeout here, because of the inertia
    GetGPIO("traction voltage sensor", &tractionVoltage);
    if ((!tractionVoltage) && (vel1))
    {
        //This means either the contactor, fuse or battery for the traction system is NOK. The vehicle is not in neutral
        FATAL_ERROR_MSG("Vehicle is moving, yet is not being powered.");
        DEBUG_MSG("VB+=%d, vel=%ld.\r\n", tractionVoltage, vel1);
        return TRUE;
    }*/
    DEBUG_MSG("No fatal errors detected.\r\n");
    return FALSE;
}

void ROVIM_T2D_MonitorBrakeUnlock(WORD period)
{
    static WORD timeoutCount=0;
    BYTE endOfTravelUnclamp=0;
    
    if(!UnlockingBrake)
    {
        timeoutCount=0;
        return;
    }
    
    timeoutCount+=period;
    GetGPIO("brake unclamp switch",&endOfTravelUnclamp);
    if ((endOfTravelUnclamp) && (timeoutCount <= ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT))
    {
        //finished unlocking brake inside the safety timeout
        ROVIM_T2D_FinishReleaseFromLockdown();
        UnlockingBrake=FALSE;
    }
    if (timeoutCount > ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT)
    {
        //timeout expired. Going back to full lockdown
        SetGPIO("brake clamper");
        ResetGPIO("brake unclamper");
        ERROR_MSG("Brake unlocking took too long. Restart the procedure.\r\n");
        DEBUG_MSG("brake unlock end-of-travel=%d, time passed=%d > timeout=%d, unlocking=%d.\r\n",endOfTravelUnclamp, timeoutCount, ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT, UnlockingBrake);
        UnlockingBrake=FALSE;
    }
    return;
}


//--------------------------------------------------------------------------------------------------
//Maintain PWM signal used by ROVIM T2D motors controlled by GPIO
//motors controller by the Dalf defaullt motor interface are outside the scope of this function
void ROVIM_T2D_ServicePWM(void)
{
    static BYTE accDutyCtrl=0, dccDutyCtrl=0;
    static WORD iDebug=1, jDebug=1; //debug stuff
    static BYTE printctrl=0, dccprintctrl=0; //debug stuff
    BYTE previousVerbosity=0;
    TIME start={0}, stop={0};
    DWORD delay=0;
    static DWORD maxDelay=0;
    GetTime(&start);

    previousVerbosity = GetVerbosity();
    SetVerbosity(VERBOSITY_LEVEL_ERROR | VERBOSITY_LEVEL_WARNING);
    /*DebugPWM and printctrl mechanism:
    DebugPWM is the global debug enable. OFF= normal operation. ON= debug mode
    printctrl controls the prints, to avoid too much info. Only on the first time a new if case is
    entered is debug info printed. This flag is reset on the next if case entered.
    dccprintctrl is like printctrl, but for dcc only. This is needed because Acc and Dcc ifs are not
    mutually exclusive.
    DebugPWM is also a counter controlling the amount of time debug is enabled.*/
    if(DebugPWM)
    {
        //debug stuff
        if (!printctrl)
        {
            printctrl=0xFF;
        }
        if(!dccprintctrl)
        {
            dccprintctrl=0xFF;
        }
    }
    else
    {
        //debug stuff
        printctrl=0;
        dccprintctrl=0;
    }
    /*DEBUG_MSG("Running PWM refresh thread. accDutyCtrl=%d, dccDutyCtrl=%d, PeriodCnt=%d, \
movementType=%d, WaitForAccDccDecay=%d, AccDutyCycle=%d, DccDutyCycle=%d.\r\n", accDutyCtrl, 
dccDutyCtrl, PeriodCnt, movementType, WaitForAccDccDecay, AccDutyCycle, DccDutyCycle); //debug stuff*/
    
    /*
    //debug stuff
    //In order to not overload the serial with prints, we do some mambo-jambo here
    if(jDebug>100)iDebug=1;
    if(jDebug>10000){jDebug=1; DebugPWM--;}
    if(DebugPWM)
    {
        jDebug++;
        iDebug++;
    }*/
    
    if(movementType==HILLHOLD)
    {
        //if(!(iDebug%4))MSG("On HILLHOLD.\r\n"); //debug stuff
        if ((DebugPWM) && (printctrl & 0x01))
        {
            //debug stuff
            MSG("Vehicle on hill hold. type=%d.\r\n",movementType);
            printctrl=0xFE;
        }
        if(DebugPWM) DebugPWM--;//otherwise this if case will hang forever with no new information
        goto exit;
    }
    if(WaitForAccDccDecay!=0)
    {
        /* We should not have both signals active at the same time. We wait until the previous one
        goes to 0 (or close to it), to activate the new one*/
        //if(!(iDebug%4))MSG("Decay!=0,=%d.\r\n",WaitForAccDccDecay); //debug stuff
        if ((DebugPWM) && (printctrl & 0x02))
        {
            //debug stuff
            MSG("Waiting for Acc/Dcc signal to fall before rising the other. \
WaitForAccDccDecay=%d.\r\n", WaitForAccDccDecay);
            printctrl=0xFD;
        }
        WaitForAccDccDecay--;
        ResetGPIO("decelerator");
        ResetGPIO("accelerator");
        
        goto exit;
    }
    if(PeriodCnt >= 100)
    {
        //if(!(iDebug%4))MSG("PeriodCnt=%d, >= 100.\r\n",PeriodCnt); //debug stuff
        if ((DebugPWM) && (printctrl & 0x04))
        {
            //debug stuff
            MSG("Resetting period counter. PeriodCnt=%d.\r\n",PeriodCnt);
            printctrl=0xFB;
            DebugPWM--;
        }
        accDutyCtrl=0;
        dccDutyCtrl=0;
        PeriodCnt=0;
    }
    PeriodCnt++;
    //accelerator pwm
    if(AccDutyCycle <= accDutyCtrl)
    {
        //if(!(iDebug%4))MSG("AccDutyCycle<=Ctrl,Ctrl=%d,Duty=%d.\r\n",accDutyCtrl,AccDutyCycle); //debug stuff
        if ((DebugPWM) &&(printctrl & 0x08))
        {
            //debug stuff
            MSG("Acc=0 now. AccDutyCycle=%d, accDutyCtrl=%d, PeriodCnt=%d.\r\n", AccDutyCycle, accDutyCtrl, PeriodCnt);
            printctrl=0xF7;
        }
        ResetGPIO("accelerator");
    }
    else
    {
        //if(!(iDebug%4))MSG("AccDutyCycle>Ctrl,Ctrl=%d,Duty=%d.\r\n",accDutyCtrl,AccDutyCycle); //debug stuff
        if ((DebugPWM) && (printctrl & 0x10))
        {
            //debug stuff
            MSG("Acc=1 now. AccDutyCycle=%d, accDutyCtrl=%d, PeriodCnt=%d.\r\n", AccDutyCycle, accDutyCtrl, PeriodCnt);
            printctrl=0xEF;
        }
        SetGPIO("accelerator");
        accDutyCtrl++;
    }
    //decelerator pwm
    if(DccDutyCycle <= dccDutyCtrl)
    {
        //if(!(iDebug%4))MSG("DccDutyCycle<=Ctrl,Ctrl=%d,Duty=%d.\r\n",dccDutyCtrl,DccDutyCycle); //debug stuff
        if ((DebugPWM) && (dccprintctrl & 0x01))
        {
            //debug stuff
            MSG("Dcc=0 now. DccDutyCycle=%d, dccDutyCtrl=%d, PeriodCnt=%d.\r\n", DccDutyCycle, dccDutyCtrl, PeriodCnt);
            dccprintctrl=0xFE;
        }
        ResetGPIO("decelerator");
    }
    else
    {
        //if(!(iDebug%4))MSG("DccDutyCycle>Ctrl,Ctrl=%d,Duty=%d.\r\n",dccDutyCtrl,DccDutyCycle); //debug stuff
        if ((DebugPWM) && (dccprintctrl & 0x02))
        {
            //debug stuff
            MSG("Dcc=1 now. DccDutyCycle=%d, dccDutyCtrl=%d, PeriodCnt=%d.\r\n", DccDutyCycle, dccDutyCtrl, PeriodCnt);
            dccprintctrl=0xFD;
        }
        SetGPIO("decelerator");
        dccDutyCtrl++;
    }
    
    /*We use a label here because there are many exit points and the exit code is quite large*/
exit:
    SetVerbosity(previousVerbosity);
    GetTime(&stop);
    delay=CalculateDelayMs(&start,&stop);
    if(delay > maxDelay)
    {
        maxDelay=delay;
        DEBUG_MSG("PWM refresh task max delay so far: %d ms.\r\n",maxDelay);
    }
}

void ROVIM_T2D_UpdateVel1Acc1(void)
{
    long temp1, temp2, velRPM, prevVel;
    
    UpdateVelocity1();
    
    temp1 = (long)V1 * (long)60000;
    temp2 = (long)VSP1 * TPR1;
    velRPM = temp1/temp2;
    prevVel = vel1;
    vel1 = velRPM * ROVIM_T2D_WHEEL_PERIMETER * 60000;
    //acc(Km/10/h/s) = dV(km/h/10) / dt(ms) * 1000(ms/s)
    temp1 = (vel1-prevVel)*1000;
    temp2 = VSP1;
    acc1 = temp1/temp2;
    
    //XXX: remove
    //DEBUG_MSG("%ld, %ld, %ld.\r\n", (((long)VSP1) * ((long)TPR1)), ((long)(VSP1 * TPR1)), (VSP1 * TPR1));
    //DEBUG_MSG("%d, %ld, %ld, %d.\r\n",  (VSP1 * TPR1), ((long)VSP1 * TPR1), (VSP1 * (long)TPR1), (VSP1 * (long)TPR1));
    
    /*DEBUG_MSG("temp1=%ld, temp2=%ld, V1=%ld, TPR=%d, VSP1=%d, velRPM=%ld, vel1=%ld, prevVel=%ld, acc dt=%d, acc=%ld.\r\n", 
    temp1, temp2, (long)V1, TPR1, VSP1, velRPM, vel1, prevVel, VSP1, acc1);
    */
}

void ROVIM_T2D_FullBrake(void)
{
    DEBUG_MSG("Applying full brake.\r\n");
    CMD='G';
    ARG[0]=ROVIM_T2D_DECELERATE_CMD_CODE;
    ARG[1]=100;
    ARGN=2;
    TeCmdDispatchExt();
}

void ROVIM_T2D_SetSpeed(BYTE speed)
{
    BYTE dutyCycle=0;
    long temp=0;
    
    temp=(long)speed*12/10;  //XXX: do this properly
    dutyCycle=(BYTE) temp;
    CMD='G';
    ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
    ARG[1]=dutyCycle;
    ARGN=2;
    TeCmdDispatchExt();
}

//-----------------------------Commands accessible from the command line or I2C---------------------

BYTE ROVIM_T2D_Lockdown(void)
{
    //XXX: temp
    return 1;
    
    if(UnlockingBrake)
    {
        STATUS_MSG("Aborting current emergency brake unlock.\r\n");
        inLockdown=FALSE;
        UnlockingBrake=FALSE;
    }
    
    if(inLockdown)
    {
        STATUS_MSG("Already in Lockdown mode.\r\n");
        return NoErr;
    }
    
    STATUS_MSG("Going to lock down mode.\r\n");
    
    //Once we engage the handbrake, the traction motor (the most critical) cannot work, so we're safe
    ROVIM_T2D_LockBrake();
    _LED3_ON;               // Visual error indication due to the brake being locked
    
    //Stop motors controlled through dalf's firmware
    /*We're gonna try with SoftStop() now
    //XXX: this command messes up motor control configuration. We have to put it back on releasefromlockdown
    CMD = 'O';
    ARGN = 0x00;
    TeCmdDispatchExt();*/
    SoftStop(2);
    //Stop traction motor and set it to hold position
    SoftStop(3);
    
    LockCriticalResourcesAccess();
    
    inLockdown=TRUE;
    DEBUG_MSG("ROVIM now in lockdown mode. Emergency brake will finish locking and vehicle will \
not be able to move while on this state.\r\n");
    
    return NoErr;
}

BYTE ROVIM_T2D_ReleaseFromLockdown(void)
{
    if(!inLockdown)
    {
        STATUS_MSG("ROVIM is already good to go.\r\n");
        return NoErr;
    }
    if(UnlockingBrake)
    {
        STATUS_MSG("ROVIM is already ready - you have to manually unlock the emergency brake now.\r\n");
        return NoErr;
    }
    
    if(!ROVIM_T2D_ValidateState())
    {
        ERROR_MSG("ROVIM is not ready to go. make sure \
every safety system is OK and try again.\r\nFor a list of the safety checks to pass before the \
vehicle can move, consult the user manual, or activate debug messages.\r\n");
        return eErr;
    }

    STATUS_MSG("Manually unclamp the emergency brake within %d ms, using the identified button on the control panel.\r\n\
The system will become operational once it detects it is fully unclamped.\r\n", ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT);
    ROVIM_T2D_UnlockBrake();
    UnlockingBrake=TRUE;
    
    return NoErr;
}

/*Allows access to GPIO sub-driver*/
BYTE ROVIM_T2D_ControlGPIO(void)
{
    BYTE GPIONumber=0;
    BYTE value=0;
    BYTE option=0;
    
    if (ARGN != 3)
    {
        return eNumArgsErr;
    }
    GPIONumber=ARG[2];
    option=ARG[1];
    if ((GPIONumber == 0) || (GPIONumber > ngpios))
    {
        ERROR_MSG("GPIO %d doesn't exist.\r\n",GPIONumber);
        return eParmErr;
    }
    
    GPIONumber--;
    
    switch(option)
    {
        case 1: 
            SetGPIO(GPIOsDescription[GPIONumber].name);
            STATUS_MSG("\"%HS\" value set to 1.\r\n",GPIOsDescription[GPIONumber].name);
            return NoErr;
        case 2:
            ResetGPIO(GPIOsDescription[GPIONumber].name);
            STATUS_MSG("\"%HS\" value reset to 0.\r\n",GPIOsDescription[GPIONumber].name);
            return NoErr;
        case 3:
            ToggleGPIO(GPIOsDescription[GPIONumber].name);
            STATUS_MSG("\"%HS\" value toggled.\r\n",GPIOsDescription[GPIONumber].name);
            return NoErr;
        case 4:
            GetGPIO(GPIOsDescription[GPIONumber].name, &value);
            STATUS_MSG("\"%HS\" value is %d.\r\n",GPIOsDescription[GPIONumber].name,value);
            return NoErr;
        default:
            ERROR_MSG("GPIO action not valid.\r\\n");
            return eParmErr;
    }
}

BYTE ROVIM_T2D_Accelerate(void)
{
    BYTE dutyCycle=0;
    
    if (ARGN != 2)
    {
        return eNumArgsErr;
    }
    
    dutyCycle=ARG[1];
    if (dutyCycle > 100)
    {
        ERROR_MSG("Duty cycle can only vary between 0 and 100%%.\r\n");
        return eParmErr;
    }
    
    AccDutyCycle=dutyCycle;
    //Used to variate PWM led of motor1, to have a visual traction power indicator.
    Power1=AccDutyCycle;
    if (DccDutyCycle!=0)
    {
        //Wait 2*tau before starting to accelerate
        WaitForAccDccDecay=2*ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC/ROVIM_T2D_PWM_REFRESH_PERIOD;
        DEBUG_MSG("Waiting for %d ms to avoid raising accelerator and decelerator simultaneously. \
WaitForAccDccDecay=%u.\r\n", WaitForAccDccDecay, (WaitForAccDccDecay*ROVIM_T2D_PWM_REFRESH_PERIOD));
    }
    DccDutyCycle=0;
    /*If we don't reset the PWM period counter, we may have the duty cycle raise up to twice our spec
    (worst case) if the change happens when this counter is already very ahead. This way, worst case
    is the first run of the thread (ROVIM_T2D_PWM_REFRESH_PERIOD ms longer than spec).*/
    PeriodCnt=100;
    STATUS_MSG("Accelerator set to %d%%.\r\n",AccDutyCycle);
    
    return NoErr;
}

BYTE ROVIM_T2D_Decelerate(void)
{
    BYTE dutyCycle=0;
    
    if (ARGN != 2)
    {
        return eNumArgsErr;
    }
    dutyCycle=ARG[1];
    if (dutyCycle > 100)
    {
        ERROR_MSG("Duty cycle can only vary between 0 and 100%%.\r\n");
        return eParmErr;
    }
    
    DccDutyCycle=dutyCycle;
    if (AccDutyCycle!=0)
    {
        //Wait 2*tau before starting to decelerate
        WaitForAccDccDecay=2*ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC/ROVIM_T2D_PWM_REFRESH_PERIOD;
        DEBUG_MSG("Waiting for %d ms to avoid raising accelerator and decelerator simultaneously. \
WaitForAccDccDecay=%u.\r\n", WaitForAccDccDecay, (WaitForAccDccDecay*ROVIM_T2D_PWM_REFRESH_PERIOD));
    }
    AccDutyCycle=0;
    /*If we don't reset the PWM period counter, we may have the duty cycle raise up to twice our spec
    (worst case) if the change happens when this counter is already very ahead. This way, worst case
    is the first run of the thread (ROVIM_T2D_PWM_REFRESH_PERIOD ms longer than spec).*/
    PeriodCnt=100;
    STATUS_MSG("Decelerator set to %d%%.\r\n",DccDutyCycle);
    
    return NoErr;
}

BYTE ROVIM_T2D_SetMovement(void)
{
    BYTE speed=0;   //in km/h/10
    BYTE direction=0;
    
    switch (ARGN)
    {
        case 1:
            //Hold (handbrake) - default: No need for any arguments
            direction=HILLHOLD;
            speed=SPEEDZERO;
            break;
        case 2:
            //Neutral or hold: specify only the type of movement - speed is zero
            direction=ARG[1];
            speed=SPEEDZERO;
            break;
        case 3:
            //Any kind of movement: specify the type of movement and the desired speed
            direction=ARG[1];
            speed=ARG[2];
            break;
        default:
            DEBUG_MSG("Wrong number of arguments.\r\n");
            return eNumArgsErr;
    }
    
    if (direction > 3)
    {
        ERROR_MSG("Direction must be %d (forward), %d (reverse), %d (neutral) or %d (hold).\r\n", 
        FORWARD, REVERSE, NEUTRAL, HILLHOLD);
        return eParmErr;
    }
    if ( (speed > ROVIM_T2D_MAX_SPEED) || ((direction <2) && (speed < ROVIM_T2D_LOWER_SPEED_LIMIT)) )
    {
        //in neutral and hold cases, speed is allways 0, wich may be <ROVIM_T2D_LOWER_SPEED_LIMIT
        ERROR_MSG("Speed must vary in 1/10 km/h between %d km/h/10 and %d km/h/10.\r\n",
        ROVIM_T2D_LOWER_SPEED_LIMIT, ROVIM_T2D_MAX_SPEED);
        DEBUG_MSG("speed=%d, direction=%d.\r\n",speed, direction);
        return eParmErr;
    }
    //TODO: Add closed loop speed control and remove this warning.
    WARNING_MSG("Speed control done only in open loop currently. Check manually if the set speed matches your request.\r\n");
    
    switch(direction)
    {
        case HILLHOLD: //Hold
            SetGPIO("engage forward");
            SetGPIO("engage reverse");
            ResetGPIO("engage handbrake");
            //We must activate this switch, otherwise the power contactor won't arm
            ResetGPIO("activate traction");
            
            movementType=HILLHOLD;
            desiredMovement.type=HILLHOLD;
            desiredMovement.speed=SPEEDZERO;
            //XXX: test on the SgimaDrive if it actually stops the motorcycle once it sets the spd3
            //ROVIM_T2D_FullBrake();
            STATUS_MSG("Vehicle will hold position once it reaches a standstill.\r\n");
            DEBUG_MSG("desired type=%d, speed=%d, current type=%d.\r\n",desiredMovement.type, desiredMovement.speed, movementType);
            break;
        case NEUTRAL: //Neutral
            SetGPIO("engage handbrake");
            SetGPIO("engage forward");
            SetGPIO("engage reverse");
            //We must activate this switch, otherwise the power contactor won't arm
            ResetGPIO("activate traction");

            movementType=NEUTRAL;
            desiredMovement.type=NEUTRAL;
            desiredMovement.speed=SPEEDZERO;    //actually, we don't care
            CMD='G';
            ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
            ARG[1]=0;
            ARGN=2;
            TeCmdDispatchExt();
            STATUS_MSG("Vehicle set in neutral.\r\n");
            DEBUG_MSG("desired type=%d, speed=%d, current type=%d.\r\n",desiredMovement.type, desiredMovement.speed, movementType);
            break;
        case FORWARD: //Forward
            desiredMovement.type=FORWARD;
            desiredMovement.speed=speed;
            if ((vel1) && (movementType!=desiredMovement.type))
            {
                DEBUG_MSG("Bringing vehicle to a standstill before moving forward. Please wait.\r\n");
                ROVIM_T2D_FullBrake();
            }
            if (movementType==desiredMovement.type)
            {
                //If we're already moving in the direction we want, just accelerate
                ROVIM_T2D_SetSpeed(desiredMovement.speed);
                STATUS_MSG("Vehicle moving forward at approximately %d km/h/10.\r\n",desiredMovement.speed);
            }
            DEBUG_MSG("desired type=%d, speed=%d, current type=%d.\r\n",desiredMovement.type, desiredMovement.speed, movementType);
            break;
        case REVERSE: //Reverse
            desiredMovement.type=REVERSE;
            desiredMovement.speed=speed;
            if ((vel1) && (movementType!=desiredMovement.type))
            {
                DEBUG_MSG("Bringing vehicle to a standstill before moving in reverse. Please wait.\r\n");
                ROVIM_T2D_FullBrake();
            }
            if (movementType==desiredMovement.type)
            {
                //If we're already moving in the direction we want, just accelerate
                ROVIM_T2D_SetSpeed(desiredMovement.speed);
                STATUS_MSG("Vehicle moving backwards at approximately %d km/h/10.\r\n",desiredMovement.speed);
            }
            DEBUG_MSG("desired type=%d, speed=%d, current type=%d.\r\n",desiredMovement.type, desiredMovement.speed, movementType);
            break;
        default:
            ERROR_MSG("Unexpected argument value.\r\n");
            break;
    }
    
    return NoErr;
}

BYTE ROVIM_T2D_Turn(void)
{
    BYTE fullAngle=0;
    int centerAngle=0;
    short long y=0;
    long temp=0;
    
    if ((ARGN != 2) && (ARGN != 1))
    {
        return eNumArgsErr;
    }
    if (ARGN == 1)
    {
        SoftStop(2);
        STATUS_MSG("Direction motor stopped.\r\n");
        return NoErr;
    }
    
    fullAngle=ARG[1];
    if (fullAngle > ROVIM_T2D_DIR_ANGULAR_RANGE)
    {
        ERROR_MSG("Turn angle can only vary between 0º (full port) and %dº (full starboard).\r\n", ROVIM_T2D_DIR_ANGULAR_RANGE);
        return eParmErr;
    }
    if (!vel1)
    {
        ERROR_MSG("Vehicle must be moving when turning. Set the vehicle to move and retry.\r\n");
        WARNING_MSG("temporary override of moving limitation for testing purposes.\r\n");
        //return eErr;
    }
    
    centerAngle=fullAngle-(ROVIM_T2D_DIR_ANGULAR_RANGE/2);
    if (centerAngle > 0)
    {
        STATUS_MSG("Turning vehicle %dº to starboard.\r\n",centerAngle);
    }
    else if (fullAngle == 0)
    {
        STATUS_MSG("Pointing vehicle in a straight line.\r\n");
    }
    else
    {
        STATUS_MSG("Turning vehicle %dº to port.\r\n",fullAngle);
    }
    
    //We must repeat this every time because when going to lockdown this is lost
    //I'm gonna try with the SoftStop() fct to avoid reconfiguring every time. If not, uncomment next line
    //ROVIM_T2D_ConfigDirParamBlock();
    temp=(long) (ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT)*centerAngle;
    y=temp/ROVIM_T2D_DIR_ANGULAR_RANGE;
    y+=ROVIM_T2D_DIR_CENTER_TICK_CNT;
    
    DEBUG_MSG("y=%ld, temp=%ld, ang range=%d, tick range=%d, angle=%d, VMID2=%d, ACC2=%d.\r\n", 
    (long)y, (long)temp, (BYTE) ROVIM_T2D_DIR_ANGULAR_RANGE, 
    (BYTE)(ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT), (BYTE) centerAngle, VMID2, ACC2);
    
    if((y<ROVIM_T2D_DIR_TICK_LOWER_LIMIT) || (y>ROVIM_T2D_DIR_TICK_UPPER_LIMIT))
    {
       ERROR_MSG("Unexpected error: target position not with bounds.\r\n"); 
       return eErr;
    }
    MoveMtrClosedLoop(2, y, VMID2, ACC2);
    
    return NoErr;
}

BYTE ROVIM_T2D_DebugControl(void)
{
    BYTE option=0;
    BYTE resetNotKick=0;
    BYTE accNotDcc=0;
    BYTE dutyCycle=0;
    static BYTE prevVerbosity=0;
    
    if (ARGN < 2)
    {
        return eNumArgsErr;
    }
    option=ARG[1];
    switch(option)
    {
        case 1:
            prevVerbosity=GetVerbosity();
            SetVerbosity(prevVerbosity | VERBOSITY_LEVEL_DEBUG);
            //we just activated debug, so we use debug traces here
            DEBUG_MSG("Debug traces enabled.Verbosity=%d.\r\n", (prevVerbosity | VERBOSITY_LEVEL_DEBUG));
            break;
        case 2:
            prevVerbosity=prevVerbosity & (~VERBOSITY_LEVEL_DEBUG);
            SetVerbosity(prevVerbosity);
            STATUS_MSG("Debug traces disabled.\r\n");
            DEBUG_MSG("Verbosity=%d.\r\n", prevVerbosity);
            break;
        case 3:
            ManualSysMonitoring=~ManualSysMonitoring;
            if(ManualSysMonitoring)
            {
                STATUS_MSG("ROVIM T2D system monitoring done manually now. Use this command \
with the respective argument to run another time the T2D system monitoring task.\r\n");
            }
            else
            {
                STATUS_MSG("ROVIM T2D system monitoring reverted to default (automatic).\r\n");
            }
            break;
        case 4:
            STATUS_MSG("Running ROVIM T2D system monitoring one time.\r\n");
            ROVIM_T2D_MonitorSystem();
            break;
        case 5:
            if (ARGN == 5)
            {
                DebugPWM=ARG[2]+1;
                accNotDcc=ARG[3];
                dutyCycle=ARG[4];
                STATUS_MSG("Running ROVIM T2D PWM generator debug with period count %d. ACC?:%d, DCC?:%d, \
Dutycyle=%d.\r\n",(DebugPWM-1), accNotDcc, (!accNotDcc), dutyCycle);
                CMD='G';
                ARG[0]=accNotDcc?ROVIM_T2D_ACCELERATE_CMD_CODE:ROVIM_T2D_DECELERATE_CMD_CODE;
                ARG[1]=dutyCycle;
                ARGN=2;
                TeCmdDispatchExt();
                break;
            }
            else if (ARGN == 3)
            {
                DebugPWM=ARG[2]+1;
                STATUS_MSG("Running ROVIM T2D PWM generator debug with period count %d.\r\n",(DebugPWM-1));
                break;
            }
            return eNumArgsErr;
        case 6:
            #ifdef WATCHDOG_ENABLED
            if (ARGN < 3)
            {
                return eNumArgsErr;
            }
            resetNotKick=ARG[2];
            if(resetNotKick)
            {
                STATUS_MSG("Resetting system through watchdog.\r\n");
                HardReset();
            }
            else
            {
                STATUS_MSG("Kicking watchdog. Kick period=%d ms. For actual watchdog timer timeout period, consult config.h\r\n",WATCHDOG_PERIOD);
                KickWatchdog();
            }
            break;
            #else //WATCHDOG_ENABLED
            STATUS_MSG("Watchdog is disabled. Recompile.\r\n");
            break;
            #endif //WATCHDOG_ENABLED
        default:
            ERROR_MSG("Option does not exist.\r\n");
            break;
    }
    return NoErr;
}
