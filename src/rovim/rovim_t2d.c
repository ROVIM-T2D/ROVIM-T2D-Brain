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
    { "engage handbrake",                               { J5,    15,     OUT,    OFF,   OFF }},
    /*Unused*/
    { "",                               { J5,    16,     IN,     OFF,   OFF }},
};

const rom BYTE nDefaultgpios=(BYTE) (sizeof(DefaultGPIOsDescription)/sizeof(DefaultGPIOsDescription[0]));
#pragma romdata //resume rom data allocation on the standard section

static BOOL CriticalResourcesToLock=FALSE;
static const BYTE ROVIM_T2D_CommandsToAllow[]={ROVIM_T2D_LOCKDOWN_CMD_CODE,ROVIM_T2D_RELEASE_CMD_CODE};
static const BYTE ROVIM_T2D_nCommandsToAllow=(BYTE) (sizeof(ROVIM_T2D_CommandsToAllow)/sizeof(ROVIM_T2D_CommandsToAllow[0]));

WORD    ROVIM_T2D_sysmonitorcount;            // ROVIM T2D system state monitoring timeout counter;
WORD    ROVIM_T2D_pwmrefreshcount;            // ROVIM T2D PWM refresh timeout counter;

//The pointer to easily switch configurations
rom const IOPinDescription* GPIOsDescription = DefaultGPIOsDescription;
BYTE ngpios=0;

//Duty cycle for the traction accelerador and decelerator 
static BYTE AccDutyCycle=0;
static BYTE DccDutyCycle=0;
static int WaitForAccDccDecay=0;

//Brake locl/unlock flags
static BOOL inLockdown=FALSE;
static BOOL Unlocking=FALSE;
static WORD MotorStressMonitor[3]={0,0,0};

//vehicle movement description
static long settlingTime=0;
static movement desiredMovement={0};
static BYTE movementType=HILLHOLD;
long acc1=0,vel1=0;
//vehicle direction movement description
long vel2=0;

//configure basic ROVIM features needed early on. To be called as soon as possible
void ROVIM_T2D_Init(void)
{
    ROVIM_T2D_sysmonitorcount=-1; //"disable" IO exp sampling for now
    SetVerbosity (INIT_VERBOSITY_LEVEL);
    ROVIM_T2D_ConfigGPIOs();
    ROVIM_T2D_LockUnusedResourcesAccess();
    //Initial values for the GPIOS will be set on the lockdown version
    ROVIM_T2D_ConfigDefaultParamBlock();

    DEBUG_MSG("ROVIM T2D initialization completed.\r\n");
    return;
}

void ROVIM_T2D_LockUnusedResourcesAccess(void)
{
    ERROR_MSG("LockUnusedResourcesAccess not implemented.\r\n");
}

void ROVIM_T2D_ConfigDefaultParamBlock(void)
{
    ROVIM_T2D_ConfigDirParamBlock();
    
    //Traction encoder set up
    //set TPR2 LSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x40;
    ARG[3]=ROVIM_T2D_TPR;
    TeCmdDispatchExt();
    //set TPR2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x41;
    ARG[3]=0;
    TeCmdDispatchExt();
    
    //Filtering configuration
    //VBATT
}

void ROVIM_T2D_ConfigDirParamBlock(void)
{
    //set MAXERR LSB
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
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x46;
    ARG[3]=ROVIM_T2D_DIR_MODE1;
    TeCmdDispatchExt();
    //set MTR2_MODE2
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x47;
    ARG[3]=ROVIM_T2D_DIR_MODE2;
    TeCmdDispatchExt();
    //set MTR2_MODE3
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x48;
    ARG[3]=ROVIM_T2D_DIR_MODE3;
    TeCmdDispatchExt();
    //set ACC2 LBS
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x49;
    ARG[3]=(BYTE) (ACC2>>8);
    TeCmdDispatchExt();
    //set ACC2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4A;
    ARG[3]=(BYTE) (ACC2 & 0xFF);
    TeCmdDispatchExt();
    //set VMID2 LSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4B;
    ARG[3]=(BYTE) (VMID2>>8);
    TeCmdDispatchExt();
    //set VMID2 MSB
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4C ;
    ARG[3]=(BYTE) (VMID2 & 0xFF);
    TeCmdDispatchExt();
    //set VSP2
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x4D;
    ARG[3]=ROVIM_T2D_DIR_VSP;
    TeCmdDispatchExt();
    //set VMIN2
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x54;
    ARG[3]=ROVIM_T2D_DIR_MIN_PWM;
    TeCmdDispatchExt();
    //set VMAX2
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x55;
    ARG[3]=ROVIM_T2D_DIR_MAX_PWM;
    TeCmdDispatchExt();
    //set TPR2 LSB
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
    CMD='W';
    ARGN=4;
    ARG[0]=1;
    ARG[1]=01;
    ARG[2]=0x75;
    ARG[3]=ROVIM_T2D_DMAX;
    TeCmdDispatchExt();    
}

//Start all remaining ROVIM features.
void ROVIM_T2D_Start(void)
{
    ROVIM_T2D_sysmonitorcount = ROVIM_T2D_SYSTEM_MONITOR_PERIOD;
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
    CriticalResourcesToLock=TRUE;
    return;
}

void ROVIM_T2D_UnlockCriticalResourcesAccess(void)
{
    CriticalResourcesToLock=FALSE;
    return;
}

BOOL ROVIM_T2D_IsCommandLocked(BYTE cmd)
{
    BYTE i;
    
    if(!CriticalResourcesToLock)
    {
        return FALSE;
    }
    
    for(i=0;i<ROVIM_T2D_nCommandsToAllow;i++)
    {
        if( cmd==ROVIM_T2D_CommandsToAllow[i])
        {
            return FALSE;
        }
    }
    DEBUG_MSG("Command is locked.\r\n");
    return TRUE;
}

//Since this function is called before interrupts are enabled, and the arguments are
//not used, we simply ignore them
BYTE ROVIM_T2D_Lockdown(void)
{
    //XXX: this should be atomic
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
    
    //XXX: this should be atomic
    inLockdown=TRUE;
    DEBUG_MSG("ROVIM now in lockdown mode. Brake will finish locking and vehicle will not be able to move while on this state.\r\n");
    
    return NoErr;
}

BYTE ROVIM_T2D_ReleaseFromLockdown(void)
{
    if(!inLockdown)
    {
        STATUS_MSG("ROVIM is already good to go.\r\n");
        return NoErr;
    }
    
    if(!ROVIM_T2D_ValidateState())
    {
        ERROR_MSG("ROVIM is not ready to go. make sure \
every safety system is OK and try again.\r\nFor a list of the safety checks to pass before the \
vehicle can move, consult the user manual, or activate debug messages.\r\n");
        return eErr;
    }

    //Initial Hw version will require the brake unlock to be done manually. Later versions won't
    //need this instruction
    STATUS_MSG("Manually unclamp the brake, using the identified button on the control panel.\r\n\
The system will become operational once it detects the brake is fully unclamped.\r\n");
    //Replace previous instruction by the following in due time
    //STATUS_MSG("Releasing ROVIM from lock down mode.\r\n");
    ROVIM_T2D_UnlockBrake();
    //XXX:This should be atomic?
    Unlocking=TRUE;
    
    return NoErr;
}

BOOL ROVIM_T2D_FinishReleaseFromLockdown(void)
{
    UnlockCriticalResourcesAccess();
    ROVIM_T2D_ConfigDefaultParamBlock();
    _LED3_OFF;
    ResetGPIO("brake unclamper");
    //XXX:This should be atomic
    inLockdown=FALSE;
    STATUS_MSG("ROVIM is now ready to move. Restart SigmaDrive controller to clear any remaining \
    error\r\n");
    
    return TRUE;
}

BOOL ROVIM_T2D_LockBrake(void)
{
    DEBUG_MSG("Breaking now.\r\n");
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

BOOL ROVIM_T2D_ValidateState(void)
{
    BYTE directionError=0;
    BYTE emergencyStop=0;
    TIME now;
    //Since the system always goes to lockdown on power up, this initial value is accurate enough
    static TIME before={0};
    
    //my birthday's june 16th. I have an amazon wishlist. No pressure, though...
    /*I'm choosing not to monitor the brake clamp & unclamp & unclamp command here because:
    the user may start unclamping before sending the command; while unclamping, there is nothing 
    I can do to stop that; After unclamping, if there is still a related (with these 3 GPIOs) 
    error condition, it will be picked up by the monitor task*/
    
    GetGPIO("direction error switch",&directionError);
    if(directionError)
    {
        DEBUG_MSG("The direction end-of-travel switch is still ON.\r\n");
    }
    if(vel1)
    {
        DEBUG_MSG("The traction is still moving.\r\n");
        return FALSE;
    }
    if(vel2)
    {
        ERROR_MSG("The direction is still moving.\r\n");
        return FALSE;
    }
    
    /*Get the emergency stop condition GPIO. Since it depends on the brake clamper being engaged (as
    happens during lockdown) and it's pin is connected to a monostable circuit, we need to wait
    until the monostable brings the value down before we can get a clean reading*/
    GetTime(&now);
    if(ROVIM_T2D_BRAKE_CLAMP_TIME > CalculateDelayMs(&before, &now))
    {
        DEBUG_MSG("Brake clamper monostable is still ON. Do not forget that it activates every \
time you call this command. Wait %d ms before trying again.\r\n",
        ROVIM_T2D_BRAKE_CLAMP_TIME);
        return FALSE;
    }
    ResetGPIO("brake clamper");
    GetGPIO("emergency stop condition",&emergencyStop);
    SetGPIO("brake clamper");
    GetTime(&before);   
    if(emergencyStop)
    {
        DEBUG_MSG("There is still an emergency stop condition active.\r\n");
        return FALSE;
    }
    
    //Inputs are good. Make sure outputs are, too
    //Stop motors
    SoftStop(2);
    SoftStop(3);

    return TRUE;    //the vehicle is good to go
}


//--------------------------------Periodic tasks of system monitoring-------------------------------
void ROVIM_T2D_MonitorSystem(void)
{
    BYTE previousVerbosity=0;
    BYTE error=0;
    
    TIME start={0}, stop={0};
    DWORD delay=0;
    static DWORD maxDelay=0;
    GetTime(&start);
    
    previousVerbosity = GetVerbosity();
    //XXX: commented for debug purposes only
    //SetVerbosity(VERBOSITY_LEVEL_ERROR | VERBOSITY_LEVEL_WARNING);
    
    //Look for unrecoverable error conditions
    if( ROVIM_T2D_DetectFatalError())
    {
        //ERROR_MSG("Fatal error action not implemented.\r\n");
        //TODO: What should be done here??
    }
    
    //Look for severe error conditions
    error  = ROVIM_T2D_DetectTractionEror();
    error |= ROVIM_T2D_DetectBrakingError();
    error |= ROVIM_T2D_DetectDirectionEror();
    if(error)
    {
        ROVIM_T2D_Lockdown();
    }
    else
    {
        DEBUG_MSG("Yay, there is no need to go to lockdown.\r\n");
    }
    
    //look for and act on non-severe error conditions
    ROVIM_T2D_MonitorTractionWarning();
    ROVIM_T2D_MonitorBrakingWarning();
    ROVIM_T2D_MonitorDirectionWarning();
    
    //look for and act on brake unlock completion
    ROVIM_T2D_MonitorBrakeUnlock();

    //Detect and act on assynchronous switch to manual mode
    ROVIM_T2D_MonitorManualMode();
    
    ROVIM_T2D_MonitorMovementChanges();
    
    SetVerbosity(previousVerbosity);
    
    GetTime(&stop);
    delay=CalculateDelayMs(&start,&stop);
    if(delay > maxDelay)
    {
        maxDelay=delay;
        DEBUG_MSG("Monitor task max delay so far: %d ms.\r\n",maxDelay);
    }
    
    return;
}

void ROVIM_T2D_MonitorMovementChanges(void)
{
    BYTE dutyCycle=0;
    
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
    
    if ( (desiredMovement.type==FORWARD) && (!vel1)  && (movementType!=FORWARD) )
    {
        //pins are active low. Order of activation is important here.
        SetGPIO("engage handbrake");
        ResetGPIO("activate traction");
        SetGPIO("engage reverse");
        ResetGPIO("engage forward");
        dutyCycle=desiredMovement.speed*15/10;  //XXX: do this properly
        CMD='G';
        ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
        ARG[1]=dutyCycle;
        ARGN=2;
        TeCmdDispatchExt();
        movementType=FORWARD;
        STATUS_MSG("Vehicle moving forward at approximately %d km/h/10.\r\n",desiredMovement.speed);
    }
    
    if ( (desiredMovement.type==FORWARD) && (!vel1)  && (movementType!=FORWARD) )
    {
        //pins are active low. Order of activation is important here.
        SetGPIO("engage handbrake");
        ResetGPIO("activate traction");
        SetGPIO("engage forward");
        ResetGPIO("engage reverse");
        dutyCycle=desiredMovement.speed*12/10;  //XXX: do this properly
        CMD='G';
        ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
        ARG[1]=dutyCycle;
        ARGN=2;
        TeCmdDispatchExt();
        movementType=REVERSE;
        STATUS_MSG("Vehicle moving backwards at approximately %d km/h/10.\r\n",desiredMovement.speed);
    }
}

void ROVIM_T2D_MonitorManualMode(void)
{
    static BOOL manualMode=FALSE;
    BYTE autoSwitch=0;
    
    GetGPIO("auto mode switch",&autoSwitch);
    
    if((!autoSwitch) && (!manualMode))
    {
        //Make sure the outputs are in a good state
        manualMode=TRUE;
        SoftStop(3);
    }
    if (autoSwitch) 
    {
        manualMode=FALSE;
    }
    
}

//detects when traction system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectTractionEror(void)
{
    static WORD movingOnHoldTimeout=0;
    
    /*XXX: Not ready yet. Must be done only with closed loop speed control
    if ((desiredMovement.speed < vel1) && (settlingTime > ROVIM_T2D_MAX_SETTLING_TIME))
    {
        ERROR_MSG("Vehicle is taking too long to reduce its speed.\r\n");
        DEBUG_MSG("Desired vel=%ld, vel=%ld, settlingTime=%d.\r\n", desiredMovement.speed, vel1, settlingTime);
        return TRUE;
    }*/
    if (vel1 > ROVIM_T2D_CRITICAL_SPEED)
    {
        ERROR_MSG("Vehicle is moving too fast.\r\n");
        DEBUG_MSG("vel=%ld.\r\n", vel1);
        return TRUE;
    }
    if ((abs(acc1)) > ROVIM_T2D_CRASH_ACC_THRESHOLD)
    {
        ERROR_MSG("Vehicle is accelerating too fast.\r\n");
        DEBUG_MSG("acc=%ld.\r\n", acc1);
        return TRUE;
    }
    if ((desiredMovement.type==HILLHOLD) && (vel1))
    {
        movingOnHoldTimeout--;
    }
    else
    {
        movingOnHoldTimeout=ROVIM_T2D_MOVING_ON_HOLD_TIMEOUT;
    }
    if (movingOnHoldTimeout)
    {
        ERROR_MSG("Vehicle should be on hold, yet it is moving.\r\n");
        DEBUG_MSG("movement type=%d, vel=%ld, timeout=%d.\r\n", desiredMovement.type, vel1, movingOnHoldTimeout);
        return TRUE;
    }

    return FALSE;
}

//detects when braking system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectBrakingError(void)
{
    BYTE endOfTravelClamp=0;
    BYTE emergencyStop=0;
    
    GetGPIO("brake clamp switch",&endOfTravelClamp);
    GetGPIO("emergency stop condition",&emergencyStop);
    
    if(emergencyStop && (!endOfTravelClamp) )
    {
        //detected emergency condition - Error!
        ERROR_MSG("There is an emergency stop condition and the brake is not fully clamped yet.\r\n");
        DEBUG_MSG("Switch=%d, Command=%d.\r\n", endOfTravelClamp, emergencyStop);
        return TRUE;
    }
    return FALSE;
}

//detects when direction system has encountered a serious error. It is in a Not OK state
BOOL ROVIM_T2D_DetectDirectionEror(void)
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
    
    return FALSE;
}

void ROVIM_T2D_MonitorTractionWarning(void)
{
    
}

void ROVIM_T2D_MonitorBrakingWarning(void)
{
    
}

void ROVIM_T2D_MonitorDirectionWarning(void)
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
    //Motor used for direction is motor 2.
    //Only detect the stress if the motor if PID is active
    if (Mtr2_Flags1 & pida_Msk)
    {
        // Due to the nature of PID control, the motor may not be at full power and still be in stress
        if (Err2 > (VMAX1/ROVIM_T2D_DIRECTION_MOTOR_STRESS_DUTY_CYCLE_THRESHOLD))
        {
            MotorStressMonitor[2]++;
        }
        if (MotorStressMonitor[2] > ROVIM_T2D_DIRECTION_MOTOR_STRESS_CNT_THRESHOLD)
        {
            //reached direction motor overstress condition - Error!
            ERROR_MSG("ROVIM direction motor is too stressed. Going to stop it for precaution.\r\n");
            DEBUG_MSG("MotorStressMonitor[2]=%d.\\r\n", MotorStressMonitor[2]);
            return;
        }
    }
}

//detects when the system has encountered an error from which it cannot recover
BOOL ROVIM_T2D_DetectFatalError(void)
{
    static BYTE unclampActionActiveTime=0;
    BYTE unclampAction=0;
    BYTE endOfTravelClamp=0;
    BYTE emergencyStop=0;
    
    GetGPIO("brake clamp switch",&endOfTravelClamp);
    GetGPIO("emergency stop condition",&emergencyStop);
    GetGPIO("brake unclamp command",&unclampAction);
    
    if(unclampAction)
    {
        unclampActionActiveTime++;
    }
    else
    {
        unclampActionActiveTime=0;
    }
    if(unclampActionActiveTime > ROVIM_T2D_UNCLAMP_ACTION_ACTIVE_TIME_LIMIT)
    {
        //User is pressing the button for too long - or something else really bad happened
        return TRUE;
    }
    
    /* Need to have a timeout here, because of the inertia
    GetGPIO("traction voltage sensor", &tractionVoltage);
    if ((!tractionVoltage) && (vel1))
    {
        ERROR_MSG("Vehicle is moving, yet is not being powered.");
        DEBUG_MSG("VB+=%d, vel=%ld.\r\n", tractionVoltage, vel1);
        return TRUE;
    }*/
}

void ROVIM_T2D_MonitorBrakeUnlock(void)
{
    static WORD timeoutCount=ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT;
    BYTE endOfTravelUnclamp=0;
    
    if(!Unlocking)
    {
        timeoutCount=ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT;
        return;
    }
    
    timeoutCount--;
    GetGPIO("brake unclamp switch",&endOfTravelUnclamp);
    if ((endOfTravelUnclamp) && (timeoutCount))
    {
        ROVIM_T2D_FinishReleaseFromLockdown();
        Unlocking=FALSE;
    }
    if (!timeoutCount)
    {
        //timeout expired. Going back to full lockdown
        SetGPIO("brake clamper");
        ResetGPIO("brake unclamper");
        ERROR_MSG("Brake unlocking took too long. Restart the procedure.\r\n");
        Unlocking=FALSE;
    }
    return;
}


//--------------------------------------------------------------------------------------------------
//Maintain PWM signal used by ROVIM T2D motors controlled by GPIO
//motors controller by the Dalf defaullt motor interface are outside the scope of this function
void ROVIM_T2D_ServicePWM(void)
{
    static BYTE accDutyCtrl=0, dccDutyCtrl=0;
    static BYTE periodCnt=0;
    static BYTE iDebug=0, jDebug=0;
    BYTE previousVerbosity=0;
    TIME start={0}, stop={0};
    DWORD delay=0;
    static DWORD maxDelay=0;
    GetTime(&start);

    previousVerbosity = GetVerbosity();
    SetVerbosity(VERBOSITY_LEVEL_ERROR | VERBOSITY_LEVEL_WARNING);

    //Debug features. To disable, just comment the next two lines
    //jDebug++;
    //iDebug++;
    //In order to not overload the serial with prints, we do some mambo-jambo here
    if(jDebug>50)iDebug=0;
    if(jDebug>1000)jDebug=0;
    
    if(movementType=HILLHOLD)
    {
        return;
    }
    if(WaitForAccDccDecay!=0)
    {
        /* We should not have both signals active at the same time. We wait until the previous one
        goes to 0 (or close to it), to activate the new one*/
        if(iDebug)MSG("WaitForAccDccDecay!=0,=%d.\r\n",WaitForAccDccDecay);
        WaitForAccDccDecay--;
        ResetGPIO("decelerator");
        ResetGPIO("accelerator");
        
        SetVerbosity(previousVerbosity);
        return;
    }
    if(periodCnt >= ROVIM_T2D_PWM_CNTS_PER_PERIOD)
    {
        if(iDebug)MSG("periodCnt >= ROVIM_T2D_PWM_CNTS_PER_PERIOD,periodCnt=%d.\r\n",periodCnt);
        accDutyCtrl=0;
        dccDutyCtrl=0;
        periodCnt=0;
    }
    periodCnt++;
    //accelerator pwm
    if(AccDutyCycle <= accDutyCtrl)
    {
        if(iDebug)MSG("AccDutyCycle <= accDutyCtrl,accDutyCtrl=%d,AccDutyCycle=%d.\r\n",accDutyCtrl,AccDutyCycle);
        ResetGPIO("accelerator");
    }
    else
    {
        if(iDebug)MSG("AccDutyCycle > accDutyCtrl,accDutyCtrl=%d,AccDutyCycle=%d.\r\n",accDutyCtrl,AccDutyCycle);
        SetGPIO("accelerator");
        accDutyCtrl+=ROVIM_T2D_PWM_MIN_STEP;
    }
    //decelerator pwm
    if(DccDutyCycle <= dccDutyCtrl)
    {
        if(iDebug)MSG("DccDutyCycle <= dccDutyCtrl,dccDutyCtrl=%d,DccDutyCycle=%d.\r\n",dccDutyCtrl,DccDutyCycle);
        ResetGPIO("decelerator");
    }
    else
    {
        if(iDebug)MSG("DccDutyCycle > dccDutyCtrl,dccDutyCtrl=%d,DccDutyCycle=%d.\r\n",dccDutyCtrl,DccDutyCycle);
        SetGPIO("decelerator");
        dccDutyCtrl+=ROVIM_T2D_PWM_MIN_STEP;
    }
    
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
    
    temp1 = (long)V1 * 60000;
    temp2 = (long)TPR1 * VSP1;
    velRPM = temp1/temp2;
    prevVel = vel1;
    vel1 = velRPM * ROVIM_T2D_WHEEL_PERIMETER * 60000;
    //acc(Km/10/h/s) = dV(km/h/10) / dt(ms) * 1000(ms/s)
    temp1 = (vel1-prevVel)*1000;
    temp2 = ROVIM_T2D_SYSTEM_MONITOR_PERIOD;
    acc1 = temp1/temp2;
}

void ROVIM_T2D_UpdateVel2(void)
{
    long temp1, temp2;
    UpdateVelocity2();
    
    //vel2(º/10/s) = V(Ticks/VSP)*[1/VSP](VSP/ms)* [ANGULAR RANGE](º) * 1000(ms/s)*[1/TICK RANGE](1/Ticks) * 10 (º/º/10)
    temp1 = (long)V2 * ROVIM_T2D_DIR_ANGULAR_RANGE * 10000;
    temp2 = (long)VSP2 * (ROVIM_T2D_DIR_TICK_UPPER_LIMIT - ROVIM_T2D_DIR_TICK_LOWER_LIMIT);
    vel2 = temp1/temp2;
}

/*
//must be called periodically, with the same periodicity, with period ROVIM_T2D_SYSTEM_MONITOR_PERIOD
BOOL ROVIM_T2D_UpdateVelandAcc(void)
{
    long temp1, temp2;
    static long prevVel;

    temp1 = (long)V1 * 60000 * ROVIM_T2D_WHEEL_PERIMETER * 60000;
    temp2 = (long)TPR1 * VSP1;
    prevVel=vel1;
    vel1 = temp1/temp2;
    //acc(Km/10/h/s) = dV(km/h/10) / dt(ms) * 1000(ms/s)
    temp1 = (vel1-prevVel)*1000;
    temp2 = ROVIM_T2D_SYSTEM_MONITOR_PERIOD;
    acc1 = temp1/temp2;
    
    return TRUE;
}*/

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
    if (DccDutyCycle!=0)
    {
        //Wait 2*tau before starting to accelerate
        WaitForAccDccDecay=2*ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC/ROVIM_T2D_PWM_REFRESH_PERIOD;
    }
    DccDutyCycle=0;
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
    }
    AccDutyCycle=0;
    STATUS_MSG("Decelerator set to %d%%.\r\n",DccDutyCycle);
    
    return NoErr;
}

BYTE ROVIM_T2D_SetMovement(void)
{
    BYTE speed=0;   //in km/h/10
    BYTE direction=0;
    BYTE dutyCycle=0;
    
    switch (ARGN)
    {
        case 1:
            //Hold (handbrake) - default: No need for any arguments
            direction=0;
            speed=SPEEDZERO;
            break;
        case 2:
            //Neutral or hold: specify only the type of movement - speed is zero
            direction=ARG[1];
            speed=SPEEDZERO;
            break;
        case 4:
            //Any kind of movement: specify the type of movement and the desired speed
            direction=ARG[1];
            speed=ARG[2];
            break;
        default:
            return eNumArgsErr;
    }
    
    if (direction > 3)
    {
        ERROR_MSG("Direction must be 0 (hold), 1 (neutral), 2 (forward) or 3 (reverse).\r\n");
        return eParmErr;
    }
    if ( (speed > ROVIM_T2D_MAX_SPEED) || ((direction >=2) && (speed < ROVIM_T2D_LOWER_SPEED_LIMIT)) )
    {
        ERROR_MSG("Speed must vary in 1/10 km/h between %d km/h/10 and %d km/h/10.\r\n",
        ROVIM_T2D_LOWER_SPEED_LIMIT, ROVIM_T2D_MAX_SPEED);
        return eParmErr;
    }
    //TODO: Add closed loop speed control and remove this warning.
    WARNING_MSG("Speed control done only in open loop currently. Check manually if the set speed matches your request.\r\n");
    
    switch(direction)
    {
        case 0: //Hold
            desiredMovement.type=HILLHOLD;
            desiredMovement.speed=SPEEDZERO;
            ROVIM_T2D_FullBrake();
            DEBUG_MSG("Vehicle will maintain position once it reaches a standstill.\r\n");
            break;
        case 1: //Neutral
            //For neutral movement type, we can just let the vehicle coast, so we can do everything now
            /*pins are active low. For safety reasons we also "depress" the "active traction" switch,
            although it wouldn't be necessary to set the vehicle in neutral*/
            SetGPIO("engage handbrake");
            SetGPIO("engage forward");
            SetGPIO("engage reverse");
            SetGPIO("activate traction");

            movementType=NEUTRAL;
            desiredMovement.type=NEUTRAL;
            desiredMovement.speed=SPEEDZERO;
            CMD='G';
            ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
            ARG[1]=0;
            ARGN=2;
            TeCmdDispatchExt();
            STATUS_MSG("Vehicle set in neutral.\r\n");
            break;
        case 2: //Forward
            desiredMovement.type=FORWARD;
            desiredMovement.speed=speed;
            if ((vel1) && (movementType!=FORWARD))
            {
                DEBUG_MSG("Bringing vehicle to a standstill before moving forward. Please wait.\r\n");
                ROVIM_T2D_FullBrake();
            }
            break;
        case 3: //Reverse
            desiredMovement.type=REVERSE;
            desiredMovement.speed=speed;
            if ((vel1) && (movementType!=REVERSE))
            {
                DEBUG_MSG("Bringing vehicle to a standstill before moving in reverse. Please wait.\r\n");
                ROVIM_T2D_FullBrake();
            }
            break;
        default:
            ERROR_MSG("Unexpected argument value.\r\n");
            break;
    }
    
    return NoErr;
}

void ROVIM_T2D_FullBrake(void)
{
    CMD='G';
    ARG[0]=ROVIM_T2D_DECELERATE_CMD_CODE;
    ARG[1]=100;
    ARGN=2;
    TeCmdDispatchExt();
}

BYTE ROVIM_T2D_Turn(void)
{
    BYTE angle=0;
    short long y=0;
    
    if ((ARGN != 2) && (ARGN != 1))
    {
        return eNumArgsErr;
    }
    if (ARGN == 1)
    {
        angle=ROVIM_T2D_DIR_ANGULAR_RANGE/2;
    }
    else
    {
        SoftStop(2);
        STATUS_MSG("Direction motor stopped.\r\n");
    }
    if (angle > ROVIM_T2D_DIR_ANGULAR_RANGE)
    {
        ERROR_MSG("Turn angle can only vary between 0º (full port) and %dº (full starboard.\r\n", ROVIM_T2D_DIR_ANGULAR_RANGE);
        return eParmErr;
    }
    if ( !vel1)
    {
        ERROR_MSG("Vehicle must be moving when turning. Set the vehicle to move and retry.\r\n");
        return eErr;
    }
    
    if (angle > (ROVIM_T2D_DIR_ANGULAR_RANGE/2))
    {
        STATUS_MSG("Turning vehicle %dº to starboard.\r\n",(angle-ROVIM_T2D_DIR_ANGULAR_RANGE/2));
    }
    else if (angle == (ROVIM_T2D_DIR_ANGULAR_RANGE/2))
    {
        STATUS_MSG("Pointing vehicle in a straight line.\r\n");
    }
    else
    {
        STATUS_MSG("Turning vehicle %dº to port.\r\n",angle);
    }
    
    //We must repeat this every time because when going to lockdown this is lost
    //I'm gonna try with the SoftStop() fct to avoid reconfiguring every time. If not, uncomment next line
    //ROVIM_T2D_ConfigDirParamBlock();
    y=(ROVIM_T2D_DIR_TICK_UPPER_LIMIT-ROVIM_T2D_DIR_TICK_LOWER_LIMIT)*angle;
    y=y/ROVIM_T2D_DIR_ANGULAR_RANGE;
    MoveMtrClosedLoop(2, y, VMID2, ACC2);
    
    return NoErr;
}
