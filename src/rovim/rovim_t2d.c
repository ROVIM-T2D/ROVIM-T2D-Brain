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
    /*Unused*/
    { "",                               { J5,    14,     IN,     OFF,   OFF }},
    /*Unused*/
    { "",                               { J5,    15,     OUT,    OFF,    ON }},
    /*detects an emergency stop condition, regarless of the source. Logic '1' indicates the switch 
is pressed and the brake is trying to clamp.*/
    { "emergency switch",               { J5,    16,      IN,     OFF,   OFF  }},
};

const rom BYTE nDefaultgpios=(BYTE) (sizeof(DefaultGPIOsDescription)/sizeof(DefaultGPIOsDescription[0]));
#pragma romdata //resume rom data allocation on the standard section

//The pointer to easily switch configurations
rom const IOPinDescription* GPIOsDescription = DefaultGPIOsDescription;
BYTE ngpios=0;

//Duty cycle for the traction accelerador and decelerator 
BYTE AccDutyCycle=0;
BYTE DccDutyCycle=0;
int WaitForAccDccDecay=0;

//configure basic ROVIM features needed early on. To be called as soon as possible
void ROVIM_T2D_Init(void)
{
    //SetExternalAppSupportFcts(ROVIM_T2D_Greeting, ROVIM_T2D_CustomCmdDispatch, ROVIM_T2D_ServiceIO);
    ioexpcount=-1; //disable IO exp sampling for now
    SetVerbosity (INIT_VERBOSITY_LEVEL);
    ROVIM_T2D_ConfigGPIOs();
    //Initial values for the GPIOS will be set on the lockdown version

    DEBUG_MSG("ROVIM T2D initialization completed.\r\n");
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
        case ROVIM_T2D_SET_SPEED_CMD_CODE:
            return ROVIM_T2D_SetSpeed();
        //Add more ROVIM commands here
        default:
            ERROR_MSG("Command does not exist.\r\n");
            return eParseErr;
    }
    ERROR_MSG("Unexpected program execution.\r\n");
    return eDisable;
}

//Since this function is called before the command interface is operational, and the arguments are
//not used, we simply ignore them
BYTE ROVIM_T2D_Lockdown(void)
{
    STATUS_MSG("Going to lock down mode.\r\n");
    
    //Once we engage the handbrake, the motors cannot work, so we're safe
    ROVIM_T2D_LockBrake();
    _LED3_ON;               // Visual error indication due to the brake being locked
    
    //Stop motors controlled through dalf's firmware
    CMD = 'O';
    ARGN = 0x00;
    TeCmdDispatchExt();
    
    //stop actuators controlled through other outputs
    //XXX: Identify changes to GPIOs state needed at this point
    
    LockCriticalResourcesAccess();
    
    DEBUG_MSG("ROVIM now in lock down mode. Should not be able to move while on this state.\r\n");
    
    return NoErr;
}

//Since this function is called before the command interface is operational, and the arguments are
//not used, we simply ignore them
BYTE ROVIM_T2D_ReleaseFromLockdown(void)
{
    if(!ROVIM_T2D_ValidateState())
    {
        ERROR_MSG("ROVIM is not ready to go. make sure \
every safety system is OK and try again.\r\nFor a list of the safety checks to pass before the \
vehicle can move, consult the user manual.\r\n");
        return eErr;
    }
    
    STATUS_MSG("Releasing ROVIM from lock down mode.\r\n");
    
    ROVIM_T2D_UnlockBrake();
    UnlockCriticalResourcesAccess();
    _LED3_OFF;
    
    DEBUG_MSG("ROVIM is now ready to move.\r\n");
    
    return NoErr;
}

BOOL ROVIM_T2D_LockBrake(void)
{
    BYTE BrakeState=0;

    ERROR_MSG("ROVIM_T2D_LockBrake not implemented.\r\n");
    /*DEBUG_MSG("Breaking now.\r\n");
    SetGPIO("brake clamper");
    ResetGPIO("brake unclamper");
    
    /* We cannot do a busy wait here, despite it being the most appropriate in this case, because
    it will interfere with system initialization, and will expire the watchdog timer, so we schedule
    a task*/
    //TODO: finish it! ScheduleTask(ROVIM_T2D_ConcludeLockdown,5500,FALSE);
    
    /*if(!GetGPIO("brake clamp switch",&BrakeState))
    {
        FATAL_ERROR_MSG("An error occurred when trying to enter lock down mode\r\n");
    }
    if(!BrakeState)
    {
        FATAL_ERROR_MSG("Brake should not be fully clamped\r\n");
    }*/
  
    return TRUE;
}

BOOL ROVIM_T2D_UnlockBrake(void)
{
    TIME then;
    BYTE BrakeState=0;
    
    ERROR_MSG("ROVIM_T2D_UnlockBrake not implemented.\r\n");
    //XXX: temp
    /*DEBUG_MSG("Waiting first for the brake to lock. This is a temporary solution");
    SetDelay(&then,5,msec_300);
    while(!Timeout(&then));
    
    DEBUG_MSG("Releasing brake now.\r\n");
    ResetGPIO("brake clamper");
    SetGPIO("brake unclamper");
    
    //XXX: temp
    SetDelay(&then,5,msec_300);
    while(!Timeout(&then));

    /* We cannot do a busy wait here, despite it being the most appropriate in this case, because
    it will interfere with system initialization, and will expire the watchdog timer, so we schedule
    a task*/

    /*if(!GetGPIO("brake unclamp switch",&BrakeState))
    {
        FATAL_ERROR_MSG("An error occurred when trying to enter lock down mode\r\n");
    }
    if(!BrakeState)
    {
        FATAL_ERROR_MSG("Expected brake to be fully unclamped.\r\n");
    }
    
    ResetGPIO("brake unclamper");*/
    
    return TRUE;
}

BOOL ROVIM_T2D_ValidateState(void)
{
    //my birthday's june 16th. I have an amazon wishlist. No pressure, though...
    ERROR_MSG("ROVIM_T2D_ValidateState not implemented.\r\n");
    return FALSE;   //The vehicle is not good to go.
}

void ROVIM_T2D_ServiceIO(void)
{
    //BYTE J5A=0, J5B=0, J6A=0, J6B=0;
    static BYTE accDutyCtrl=0, dccDutyCtrl=0;
    static BYTE periodCnt=0;
    BYTE previousVerbosity=0;
    static BYTE iDebug=0, jDebug=0;

    previousVerbosity = GetVerbosity();
    SetVerbosity(VERBOSITY_LEVEL_ERROR | VERBOSITY_LEVEL_WARNING);
    /*if(!GetAllGPIO(&J5A, &J5B, &J6A, &J6B))
    {
        FATAL_ERROR_MSG("ROVIM_T2D_ReadVehicleState: Could not read GPIOs.\r\n");
    }*/

    //Debug features. To disable, just comment the next two lines
    //jDebug++;
    //iDebug++;
    //In order to not overload the serial with prints, we do some mambo-jambo here
    if(jDebug>50)iDebug=0;
    if(jDebug>500)jDebug=0;
    
    #if 0
    //pwm timming control
    if(WaitForAccDccDecay!=0)
    {
        /* We cannot have both signals active at the same time. We must wait until the previous one
        goes to 0 (or close to it), to activate the new one*/
        if(iDebug)MSG("WaitForAccDccDecay!=0,=%d.\r\n",WaitForAccDccDecay);
        WaitForAccDccDecay--;
        ResetGPIO("decelerator");
        ResetGPIO("accelerator");
    }
    #endif
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
        accDutyCtrl+=ROVIM_T2D_PWM_MIN_DUTY;
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
        dccDutyCtrl+=ROVIM_T2D_PWM_MIN_DUTY;
    }
    
    //Testing encoder readings
    //TODO:REMOVE
    /*if(jDebug<50)
    {
        SetGPIO("brake unclamper");
    }
    else
    {
        ResetGPIO("brake unclamper");
    }
    if((jDebug>=1) && (jDebug<51))
    {
        SetGPIO("brake clamper");
    }
    else
    {
        ResetGPIO("brake clamper");
    }
    jDebug++;
    if(jDebug>=100)
    {
        jDebug=0;
    }*/
    //end of encoder readings testing

    SetVerbosity(previousVerbosity);
    return;
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
    
    if( dutyCycle%ROVIM_T2D_PWM_MIN_DUTY != 0)
    {
        //We assume the next possible duty cycle step in this case. This way it will allways be above the threshold.
        dutyCycle=(BYTE) (dutyCycle/ROVIM_T2D_PWM_MIN_DUTY+1)*ROVIM_T2D_PWM_MIN_DUTY;
        WARNING_MSG("Duty cycle can only vary in steps of %d%%. Going to use %d%% instead.\r\n",
ROVIM_T2D_PWM_MIN_DUTY, dutyCycle);
    }
    
    AccDutyCycle=dutyCycle;
    if (DccDutyCycle!=0)
    {
        //Wait before starting to accelerate
        WaitForAccDccDecay=2*ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC/IO_SAMPLE_PERIOD;
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
    
    if( dutyCycle%ROVIM_T2D_PWM_MIN_DUTY != 0)
    {
        //We assume the next possible duty cycle step in this case. This way it will allways be above the threshold.
        dutyCycle=(BYTE) (dutyCycle/ROVIM_T2D_PWM_MIN_DUTY+1)*ROVIM_T2D_PWM_MIN_DUTY;
        WARNING_MSG("Duty cycle can only vary in steps of %d%%. Going to use %d%% instead.\r\n",
ROVIM_T2D_PWM_MIN_DUTY, dutyCycle);
    }
    
    DccDutyCycle=dutyCycle;
    if (AccDutyCycle!=0)
    {
        //Wait before starting to decelerate
        WaitForAccDccDecay=2*ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC/IO_SAMPLE_PERIOD;
    }
    AccDutyCycle=0;
    STATUS_MSG("Decelerator set to %d%%.\r\n",DccDutyCycle);
    
    return NoErr;
}

BYTE ROVIM_T2D_SetSpeed(void)
{
    BYTE speed=0;   //in km/h/10
    BYTE direction=0;
    BYTE dutyCycle=0;
    
    if ((ARGN != 3) && (ARGN != 1))
    {
        return eNumArgsErr;
    }
    if (ARGN == 1)
    {
        direction=0;
        speed=ROVIM_T2D_LOWER_SPEED_LIMIT;
    }
    else
    {
        direction=ARG[1];
        speed=ARG[2];
    }
    
    if (direction > 2)
    {
        ERROR_MSG("Direction must be 0 (neutral), 1 (forward) or 2 (reverse).\r\n");
        return eParmErr;
    }
    if ((speed > ROVIM_T2D_UPPER_SPEED_LIMIT) || (speed < ROVIM_T2D_LOWER_SPEED_LIMIT))
    {
        ERROR_MSG("Speed must vary in 1/10 km/h between %d km/h/10 and %d km/h/10.\r\n",
        ROVIM_T2D_LOWER_SPEED_LIMIT, ROVIM_T2D_UPPER_SPEED_LIMIT);
        return eParmErr;
    }
    
    //TODO: Add closed loop speed control and remove this warning.
    WARNING_MSG("Speed control done only in open loop currently. Check manually if the set speed matches your request.\r\n");
    
    switch(direction)
    {
        case 0: //Neutral
            /*pins are active low. For safety reasons we also "depress" the "active traction" switch,
            although it wouldn't be necessary to set the vehicle in neutral*/
            SetGPIO("engage forward");
            SetGPIO("engage reverse");
            SetGPIO("activate traction");
            //Set the accelerator at 0%
            //This command will also set the deceleration to 0, so THE VEHICLE WILL COAST, as intended
            CMD='G';
            ARG[0]=ROVIM_T2D_ACCELERATE_CMD_CODE;
            ARG[1]=0;
            ARGN=2;
            TeCmdDispatchExt();
            STATUS_MSG("Vehicle set in neutral.\r\n");
            break;
        case 1: //Forward
            //pins are active low. Order of activation is important here.
            ResetGPIO("activate traction");
            SetGPIO("engage reverse");
            ResetGPIO("engage forward");
            ERROR_MSG("Feature not fully implemented yet. You must now use Accelerate (G2) command to move the vehicle.\r\n");
            /*dutyCycle=calculations?
            CMD='G';
            ARG[0]=ROVIM_T2D_ACCELERATE_CODE
            ARG[1]=dutyCycle;
            ARGN=2;
            TeCmdDispatchExt();
            STATUS_MSG("Vehicle moving forward at approximately %d km/h/10.\r\n",speed);*/
            break;
        case 2: //Reverse
            //pins are active low. Order of activation is important here.
            ResetGPIO("activate traction");
            SetGPIO("engage forward");
            ResetGPIO("engage reverse");
            ERROR_MSG("Feature not fully implemented yet. You must now use Accelerate (G2) command to move the vehicle.\r\n");
            /*dutyCycle=calculations?
            CMD='G';
            ARG[0]=ROVIM_T2D_ACCELERATE_CODE
            ARG[1]=dutyCycle;
            ARGN=2;
            TeCmdDispatchExt();
            STATUS_MSG("Vehicle moving backwards at approximately %d km/h/10.\r\n",speed);*/
            break;
        default:
            ERROR_MSG("Unexpected argument value.\r\n");
            break;
    }
    
    return NoErr;
}