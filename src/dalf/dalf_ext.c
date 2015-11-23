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
#include <string.h>

static BOOL GetGPIOConfigbyName(const rom char* name, IOPinConfig* config);

static BYTE verbosity = VERBOSITY_DISABLED;     //controls the verbosity of the debug information

void (*AckCallback)(void) = NULL;

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

    DEBUG_MSG("Cmd received: %c,(%d in decimal). # arguments: %d. Arguments: ", CMD, CMD, ARGN);
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
//Logging features. This module is only compiled if the features are enabled.
///////////////////////////////////////////////////////////////////////////////


#ifdef LOG_ENABLED

void LOG_LogInit(void)
{
    ERROR_MSG("LOG_LogInit not implemented.\r\n");
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
    //TODO: find out if we just recovered from a hard reset (watchdog) or it's a power-on
    return;
}

#ifdef WATCHDOG_ENABLED

void InitWatchdog(void)
{
    ERROR_MSG("InitWatchdog not implemented.\r\n");
    return;
}

void KickWatchdog(void)
{
    ERROR_MSG("KickWatchdog not implemented.\r\n");
    return;
}

#endif

/**
\Brief Call the actions required to execute a command
*/
BYTE TeCmdDispatchExt(void)
{
    //XXX: create the lock motors access feature here
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
            if (ARGN==0)
            {
                return TeProcessAck();
            }
            switch (ARG[0])
            {
                /*custom functions index:
                this is to be used if we want other custom functions
                other than ROVIM_T2D ones. They must be inserted here, 
                to be parsed before calling the ROVIM dispatch*/
                case 0:
                    return TeProcessAck();
                case 1:
                    return TeDisableAck();
                default:
                    return ROVIM_T2D_CmdDispatch();
            }
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
    return TE_CmdParse();
}

BYTE I2C2CmdDispatchExt(void)
{
    return I2C2CmdDispatch();
}

BYTE TeProcessAck(void)
{
    if(ARGN > 1)
    {
        return eNumArgsErr;
    }

    if(AckCallback != NULL)
    {
        AckCallback();
    }
    //call registered functions to process the ack
    STATUS_MSG("There are no tasks pending user validation to resume.\r\n");
    return NoErr;
}

BYTE TeDisableAck(void)
{
    if(ARGN != 1)
    {
        return eNumArgsErr;
    }
    
    AckCallback=NULL;
    STATUS_MSG("Tasks pending user validation were terminated.\r\n");
}

#ifdef HELP_ENABLED
// Help submodule ----------------------------------------------------------------------------------
BYTE ShowHelp(void)
{
    ERROR_MSG("ShowHelp not implemented.\r\n");
    return eDisable;
}

void RegisterCmdhelp(void)
{
    ERROR_MSG("RegisterCmdhelp not implemented.\r\n");
    return;
}

#endif

void LockCriticalResourcesAccess(void)
{
    ERROR_MSG("LockCriticalResourcesAccess not implemented.\r\n");
    return;
}

void UnlockCriticalResourcesAccess(void)
{
    ERROR_MSG("UnlockCriticalResourcesAccess not implemented.\r\n");
    return;
}

/*void ScheduleTask(ROVIM_T2D_ConcludeLockdown,5500,FALSE);
{
    
}*/

// GPIO sub-driver ---------------------------------------------------------------------------------
/*Remark: Due to the nature of this application, where all possible GPIOs are known beforehand, 
(since they need to be physically connected) this driver works with a constant set of possible 
GPIOs, while allowing to change each one's configuration. Each GPIO has a default configuration,
that is the mos likely to be use during program operation.*/

//Set a config for an existing GPIO
BOOL SetGPIOConfig(const rom char* name, IOPinConfig* config)
{
    BYTE aux=0, i=0;
    BYTE dir=0;
    BYTE pullup=0;
    BYTE inverted=0;
    //These variables depecting the previous state exist to provide debug information
    BYTE previousDir=0;
    BYTE previousPullup=0;
    BYTE previousInverted=0;
    IOPinConfig defaultConfig={0};
    
    //parameter check
    if(config == NULL)
    {
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    if((config->number > GPIOS_PER_EXPANDER) || (config->number==0))
    {
        ERROR_MSG("GPIO description parameters invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(name == NULL)
    {
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    //Verify the GPIO exists and is coherent with default params
    if(!GetGPIOConfigbyName(name, &defaultConfig))
    {
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        return FALSE;
    }
    //XXX:this should be part of the GPIO id. The config should only the 3 non-constant parameters
    if((defaultConfig.number != config->number) || (defaultConfig.exp != config->exp))
    {
        ERROR_MSG("GPIO description does not match a valid GPIO's. Aborting operation.\r\n");
        return FALSE;
    }
    
    //Get current IOEXP bank config
    if(config->exp == J5){
        previousDir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        previousInverted=ReadIOExp2(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        previousPullup=ReadIOExp2(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    else{
        previousDir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        previousInverted=ReadIOExp1(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        previousPullup=ReadIOExp1(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    
    //Change the configuration of only the GPIO under treatment
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    CHANGE Nth BIT TO X: number ^= (-x ^ number) & (1 << n);*/
    aux= config->inverted==ON? 0xFF:0;
    inverted = previousInverted ^( (aux ^ previousInverted) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    aux= config->pullup==ON? 0xFF:0;
    pullup = previousPullup ^( (aux ^ previousPullup) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    aux= config->dir==IN? 0xFF:0;
    dir = previousDir ^( (aux ^ previousDir) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    
    //Write the new configuration
    if(config->exp == J5){
        WriteIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config->number), dir);
        WriteIOExp2(IPOLA + IOEXP_REG_BANK_OFFSET(config->number), inverted);
        WriteIOExp2(GPPUA + IOEXP_REG_BANK_OFFSET(config->number), pullup);
    }
    else{
        WriteIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config->number), dir);
        WriteIOExp1(IPOLA + IOEXP_REG_BANK_OFFSET(config->number), inverted);
        WriteIOExp1(GPPUA + IOEXP_REG_BANK_OFFSET(config->number), pullup);
    }
    
    DEBUG_MSG("SetGPIOConfig: exp=J%d, pin=%d, bank=%c, bank bit=%d. Previous: dir=%08b, \
inv=%08b, pull=%08b. Desired: dir=%01b, inv=%01b, pull=%01b. Current: dir=%08b, inv=%08b, pull=%08b.\r\n", \
  (config->exp==J5)?5:6, config->number, ('A'+IOEXP_REG_BANK_OFFSET(config->number)),\
(IOEXP_PIN_BIT_OFFSET(config->number)+1), previousDir, previousInverted, previousPullup, \
(config->dir==IN? 1:0), (config->inverted==ON? 1:0),(config->pullup==ON? 1:0), dir, inverted, pullup);

    /* Uncomment this section if you want to check intermediate calculations
    DEBUG_MSG("SetGPIOConfig: pin=%d, bank offset=%d, bit offset=%d. Previous: dir=%08b, inv=%08b, pull=%08b\
. Intermediate calculations (for dir only): aux=%08b, aux^previousDir=%08b, 1U<<offset=%08b, (aux^previousDir) & (1U<<offset)\
=%08b. Current: dir=%08b, inv=%08b, pull=%08b.\r\n", config->number, IOEXP_REG_BANK_OFFSET(config->number),\
IOEXP_PIN_BIT_OFFSET(config->number), previousDir, previousInverted, previousPullup, aux, (aux ^ previousDir),\
 (1U << IOEXP_PIN_BIT_OFFSET(config->number)), \
 ((aux ^ previousDir) & (1U << IOEXP_PIN_BIT_OFFSET(config->number))), dir, inverted, \
 pullup);*/
 
    return TRUE;
}

//both parameters must point to valid variables. Space won't be allocated here
BOOL GetGPIOConfig(const rom char* name, IOPinConfig* config)
{
    BYTE dir=0, pullup=0, inverted=0, aux=0, i=0;
    
    if(config == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(!GetGPIOConfigbyName(name, config)){
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        config=NULL;
        return FALSE;
    }
    
    //read the IOEXP bank configuration
    if(config->exp == J5){
        dir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        inverted=ReadIOExp2(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        pullup=ReadIOExp2(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    else{
        dir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        inverted=ReadIOExp1(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        pullup=ReadIOExp1(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    
    //get and translate the configuration for the GPIO under treatment
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    CHECK: bit = (number >> x) & 1;*/
    aux= (dir >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->dir= aux? IN: OUT;
    aux= (inverted >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->inverted= aux? ON: OFF;
    aux= (pullup >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->pullup= aux? ON: OFF;
    
    DEBUG_MSG("GetGPIOConfig: exp=J%d, pin=%d, bank=%c, bank bit=%d. dir=%08b,%d, inv=%08b,%d, pull=%08b,\
%d.\r\n",  (config->exp==J5)?5:6, config->number, ('A'+IOEXP_REG_BANK_OFFSET(config->number)),\
    (IOEXP_PIN_BIT_OFFSET(config->number)+1), dir, config->dir, inverted, config->inverted,\
    pullup, config->pullup);
    return TRUE;
}

BOOL CompareGPIOConfig(IOPinConfig* config1, IOPinConfig* config2)
{
    if(config1 == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    if(config2 == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    if((config1->number > GPIOS_PER_EXPANDER) || (config1->number==0))
    {
        ERROR_MSG("GPIO config parameters invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if((config2->number > GPIOS_PER_EXPANDER) || (config2->number==0))
    {
        ERROR_MSG("GPIO config parameters invalid. Aborting operation.\r\n");
        return FALSE;
    }
    
    DEBUG_MSG("CompareGPIOConfig: 1:exp=J%d,number=%d,dir=%d,pullup=%d,inverted=%d;  2:exp=J%d,\
number=%d,dir=%d,pullup=%d,inverted=%d\r\n",(config1->exp==J5)?5:6, config1->number,config1->dir,\
config1->pullup,config1->inverted, (config2->exp==J5)?5:6, config2->number,config2->dir,config2->pullup,config2->inverted);

    if( memcmp( (const void *)config1, (const void *)config2, sizeof(IOPinConfig)))
    {
        return FALSE; //configurations do not match
    }
    
    return TRUE;
}

//String constants are automatically stored in rom. See MPLAB C-18 Users guide, 2.73.
/*this function searches the list of Registered GPIO names (set during initialization) for a
match to the name provided. If it is found, it returns the GPIO's corresponding configuration,
If not, an error occurs.*/
//This is an sub-driver private function. Parameter check is done before calling it.
static BOOL GetGPIOConfigbyName(const rom char* name, IOPinConfig* config)
{
    BYTE i=0;
    
    if(config == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    
    for(i=0; i<ngpios; i++){
        //Do NOT use strncmppgm, since it requires the 3rd argument to be in rom and fails silently if it isn't correctly provided!
        if(strcmppgm(name, GPIOsDescription[i].name) == 0){
            memcpypgm2ram(config,(const rom void *) &GPIOsDescription[i].config,sizeof(*config));
            DEBUG_MSG("GetGPIOConfigbyName: matching name found for '%HS'.\r\n", name); //%HS prints a string located in far rom
            return TRUE;
        }
    }

    ERROR_MSG("GetGPIOConfigbyName could not find a matching name for '%HS'.\r\n",name);
    config=NULL;
    return FALSE;
}

//XXX: Should not be able to set, reset and toggle a GPIO configured as input,
//yet, I may want to get the value os a GPIO configured as output.
//XXX:"Rule of thumb: Always read inputs from PORTx and write outputs to LATx. If you need to read what you set an output to, read LATx."
BOOL SetGPIO(const rom char* name)
{
    BYTE i=0;
    BYTE previousValue=0, value;
    BYTE dir=0;
    IOPinConfig config;

    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(!GetGPIOConfigbyName(name, &config)){
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        return FALSE;
    }  
    //Check if the GPIO is configured as input
    if(config.exp == J5){
        dir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        dir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    dir = (dir >> IOEXP_PIN_BIT_OFFSET(config.number)) & 1U;
    if(dir){
        ERROR_MSG("GPIO configured as input. Aborting operation.\r\n");
        return FALSE;
    }

    //Get current IOEXP bank gpios
    if(config.exp == J5){
        previousValue=ReadIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        previousValue=ReadIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }

    //Change the configuration of only the GPIO under treatment
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    SET: number |= 1 << x;*/
    value = previousValue | (1U << IOEXP_PIN_BIT_OFFSET(config.number));
    
    //Write the new value only if needed
    if(previousValue!=value)
    {
        if(config.exp == J5){
            WriteIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
        }
        else{
            WriteIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
        }
    }
    else
    {
        DEBUG_MSG("GPIO value already matches pretended value. No action needed.\r\n");
    }
    
    DEBUG_MSG("SetGPIO: exp=J%d, pin=%d, bank %c, bank bit=%d. Previous bit value=%08b, current \
bit value=%08b.\r\n", (config.exp==J5)?5:6, config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)),\
    (IOEXP_PIN_BIT_OFFSET(config.number) +1), previousValue, value);
    return TRUE;
}

BOOL ResetGPIO(const rom char* name)
{
    BYTE i=0;
    BYTE previousValue=0, value=0;
    BYTE dir=0;
    IOPinConfig config;

    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(!GetGPIOConfigbyName(name, &config)){
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        return FALSE;
    }
    //Check if the GPIO is configured as input
    if(config.exp == J5){
        dir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        dir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    dir = (dir >> IOEXP_PIN_BIT_OFFSET(config.number)) & 1U;
    if(dir){
        ERROR_MSG("GPIO configured as input. Aborting operation.\r\n");
        return FALSE;
    }
    
    //Get current IOEXP bank gpios
    if(config.exp == J5){
        previousValue=ReadIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        previousValue=ReadIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    //Change the configuration of only the GPIO under treatment
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    RESET: number &= ~(1 << x);*/
    value = previousValue & (~(1U << IOEXP_PIN_BIT_OFFSET(config.number)));
    
    //Write the new value only if needed
    if(previousValue!=value)
    {
        if(config.exp == J5){
            WriteIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
        }
        else{
            WriteIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
        }
    }
    else
    {
        DEBUG_MSG("GPIO value already matches pretended value. No action needed.\r\n");
    }

    DEBUG_MSG("ResetGPIO: exp=J%d, pin=%d, bank %c, bank bit=%d. Previous bit value=%08b, current \
bit value=%08b.\r\n", (config.exp==J5)?5:6, config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)),\
    (IOEXP_PIN_BIT_OFFSET(config.number) +1), previousValue, value);
    return TRUE;
}

BOOL ToggleGPIO(const rom char* name)
{
    BYTE i=0;
    BYTE previousValue=0, value=0;
    BYTE dir=0;
    IOPinConfig config;

    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(!GetGPIOConfigbyName(name, &config)){
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        return FALSE;
    }
    //Check if the GPIO is configured as input
    if(config.exp == J5){
        dir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        dir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    dir = (dir >> IOEXP_PIN_BIT_OFFSET(config.number)) & 1U;
    if(dir){
        ERROR_MSG("GPIO configured as input. Aborting operation.\r\n");
        return FALSE;
    }
    
    //Get current IOEXP bank gpios
    if(config.exp == J5){
        previousValue=ReadIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        previousValue=ReadIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    TOGGLE: number ^= 1 << x;*/
    value = previousValue ^ (1U << IOEXP_PIN_BIT_OFFSET(config.number));
    
    //Write the new value
    if(config.exp == J5){
        WriteIOExp2(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
    }
    else{
        WriteIOExp1(OLATA + IOEXP_REG_BANK_OFFSET(config.number), value);
    }

    DEBUG_MSG("ToggleGPIO: exp=J%d, pin=%d, bank %c, bank bit=%d. Previous bit value=%08b, current \
bit value=%08b.\r\n", (config.exp==J5)?5:6, config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)),\
    (IOEXP_PIN_BIT_OFFSET(config.number) +1), previousValue, value);
    return TRUE;
}

BOOL GetGPIO(const rom char* name, BYTE* value)
{
    BYTE i=0;
    IOPinConfig config;
    BYTE aux=0;
    
    if(name == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    for(i=0;i<IO_PIN_NAME_MAX_LEN;i++)
    {
        if(name[i] == '\0') break;
    }
    if((i==0) || (i==IO_PIN_NAME_MAX_LEN))
    {
        ERROR_MSG("GPIO name invalid. Aborting operation.\r\n");
        return FALSE;
    }
    if(!GetGPIOConfigbyName(name, &config)){
        ERROR_MSG("GPIO does not exist. Aborting operation.\r\n");
        return FALSE;
    }
    if(value == NULL){
        ERROR_MSG("Received NULL input parameter. Aborting operation.\r\n");
        return FALSE;
    }
    
    if(config.exp == J5){
        aux=ReadIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        aux=ReadIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    /* For further info on these calculations see:
    http://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit-in-c-c
    CHECK: bit = (number >> x) & 1;*/
    *value= (aux>> IOEXP_PIN_BIT_OFFSET(config.number)) & 1U;

    DEBUG_MSG("GetGPIO: exp=J%d, pin=%d, bank %c, bank bit=%d. Value=%d.\r\n", (config.exp==J5)?5:6,\
    config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)),\
    (IOEXP_PIN_BIT_OFFSET(config.number) +1), *value);
    return TRUE;
}

BOOL GetAllGPIO(BYTE* J5A, BYTE* J5B, BYTE* J6A, BYTE* J6B)
{
    *J5A=ReadIOExp2(GPIOA);
    *J6A=ReadIOExp1(GPIOA);
    *J5B=ReadIOExp2(GPIOB);
    *J6B=ReadIOExp1(GPIOB);
    
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
        ERROR_MSG("Trying to register a NULL function.\r\n");
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
