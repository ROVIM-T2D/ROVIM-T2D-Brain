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
//Logging features. This module is only compiled if the features are enabled.
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

void LockMotorsAccess(void)
{
    return;
}

void UnlockMotorsAccess(void)
{
    return;
}

// GPIO sub-driver ---------------------------------------------------------------------------------

BOOL SetGPIOConfig(IOPinId* config)
{
    BYTE pre_dir=0, pre_pullup=0, pre_inverted=0, aux=0, post_dir=0, post_pullup=0, post_inverted=0;
    
    //parameter check
    if(config == NULL){
        return FALSE;
    }
    
    //Get current IOEXP bank config
    if(config->exp == J5){
        pre_dir=ReadIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        pre_inverted=ReadIOExp2(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        pre_pullup=ReadIOExp2(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    else{
        pre_dir=ReadIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config->number));
        pre_inverted=ReadIOExp1(IPOLA + IOEXP_REG_BANK_OFFSET(config->number));
        pre_pullup=ReadIOExp1(GPPUA + IOEXP_REG_BANK_OFFSET(config->number));
    }
    
    //Change the configuration of only the GPIO under treatment
    aux= config->inverted==ON? 0xFF:0;
    post_inverted = pre_inverted ^( (aux ^ pre_inverted) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    aux= config->pullup==ON? 0xFF:0;
    post_pullup = pre_pullup ^( (aux ^ pre_pullup) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    aux= config->dir==IN? 0xFF:0;
    post_dir = pre_dir ^( (aux ^ pre_dir) & (1U << IOEXP_PIN_BIT_OFFSET(config->number)) );
    
    //Write the new configuration
    if(config->exp == J5){
        WriteIOExp2(IODIRA + IOEXP_REG_BANK_OFFSET(config->number), post_dir);
        WriteIOExp2(IPOLA + IOEXP_REG_BANK_OFFSET(config->number), post_inverted);
        WriteIOExp2(GPPUA + IOEXP_REG_BANK_OFFSET(config->number), post_pullup);
    }
    else{
        WriteIOExp1(IODIRA + IOEXP_REG_BANK_OFFSET(config->number), post_dir);
        WriteIOExp1(IPOLA + IOEXP_REG_BANK_OFFSET(config->number), post_inverted);
        WriteIOExp1(GPPUA + IOEXP_REG_BANK_OFFSET(config->number), post_pullup);
    }
    
    DEBUG_MSG("SetGPIOConfig: pin=%d, bank offset=%d, bit offset=%d. Previous: dir=%08b, inv=%08b, pull=%08b\
. Intermediate calculations (for dir only): aux=%08b, aux^pre_dir=%08b, 1U<<offset=%08b, (aux^pre_dir) & (1U<<offset)\
=%08b. Current: dir=%08b, inv=%08b, pull=%08b\r\n", config->number, IOEXP_REG_BANK_OFFSET(config->number),\
IOEXP_PIN_BIT_OFFSET(config->number), pre_dir, pre_inverted, pre_pullup, aux, (aux ^ pre_dir),\
 (1U << IOEXP_PIN_BIT_OFFSET(config->number)), \
 ((aux ^ pre_dir) & (1U << IOEXP_PIN_BIT_OFFSET(config->number))), post_dir, post_inverted, \
 post_pullup);
    return TRUE;
}

//both parameters must point to valid variables. Space won't be allocated here
BOOL GetGPIOConfig(const rom char* name, IOPinId* config)
{
    BYTE dir=0, pullup=0, inverted=0, aux=0;
    
    if(config == NULL){
        return FALSE;
    }
    if(name == NULL){
        return FALSE;
    }
    
    if(!GetDefaultGPIOConfigbyName(name, config)){
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
    aux= (dir >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->dir= aux? IN: OUT;
    aux= (inverted >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->inverted= aux? ON: OFF;
    aux= (pullup >> IOEXP_PIN_BIT_OFFSET(config->number)) & 1U;
    config->pullup= aux? ON: OFF;
    
    DEBUG_MSG("GetGPIOConfig: pin=%d, bank offset=%d, bit offset=%d. dir=%08b,%d, inv=%08b,%d, pull=%08b,\
%d.\r\n", config->number, IOEXP_REG_BANK_OFFSET(config->number), IOEXP_PIN_BIT_OFFSET(config->number), \
dir, config->dir, inverted, config->inverted, pullup, config->pullup );
    return TRUE;
}

//String constants are automatically stored in rom. See MPLAB C-18 Users guide, 2.73.
//So, the first argument is in fact rom qualified.
//However, there is no strncmppgm2pgm - outstanding!
BOOL GetDefaultGPIOConfigbyName(const rom char* name, IOPinId* config)
{
    BYTE i=0;
    char aux[IO_PIN_NAME_MAX_LEN]={0};
    
    if(config == NULL){
        return FALSE;
    }
    if(name == NULL){
        return FALSE;
    }
   
    for(i=0; i<ngpios; i++){
        //copy the first argument to ram, to be able to use it on strncmppgm2ram
        strncpypgm2ram(aux, name, IO_PIN_NAME_MAX_LEN);
        if(strncmppgm2ram(aux, DefaultGPIOsConfig[i].name, IO_PIN_NAME_MAX_LEN) == 0){
            memcpypgm2ram(config,&(DefaultGPIOsConfig)[i],sizeof(*config));
            //cannot print config->name because printf like functions 
            //"expect the format string to be stored in program memory"
            DEBUG_MSG("GetDefaultGPIOConfigbyName: matching name found:%s\r\n", aux);
            return TRUE;
        }
    }

    ERROR_MSG("GetDefaultGPIOConfigbyName could not find a matching GPIO\r\n");
    config=NULL;
    return FALSE;
}

BOOL SetGPIO(const rom char* name)
{
    BYTE pre_value=0, post_value;
    IOPinId config;

    if(name == NULL){
        return FALSE;
    }

    if(!GetDefaultGPIOConfigbyName(name, &config)){
        return FALSE;
    }
    
    //Get current IOEXP bank gpios
    if(config.exp == J5){
        pre_value=ReadIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        pre_value=ReadIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    //Change the configuration of only the GPIO under treatment
    post_value = pre_value | (1U << IOEXP_PIN_BIT_OFFSET(config.number));
    
    //Write the new value
    if(config.exp == J5){
        WriteIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }
    else{
        WriteIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }
    
    DEBUG_MSG("SetGPIO: pin=%d, previous GPIO bank %c  value=%08b, new value=%08b\r\n",\
    config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)), pre_value, post_value);
    return TRUE;
}

BOOL ResetGPIO(const rom char* name)
{
    BYTE pre_value=0, post_value=0;
    IOPinId config;

    if(name == NULL){
        return FALSE;
    }

    if(!GetDefaultGPIOConfigbyName(name, &config)){
        return FALSE;
    }
    
    //Get current IOEXP bank gpios
    if(config.exp == J5){
        pre_value=ReadIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        pre_value=ReadIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    //Change the configuration of only the GPIO under treatment
    post_value = pre_value & (~(1U << IOEXP_PIN_BIT_OFFSET(config.number)));
    
    //Write the new value
    if(config.exp == J5){
        WriteIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }
    else{
        WriteIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }

    DEBUG_MSG("ResetGPIO: pin=%d, previous GPIO bank %c value=%08b, new value=%08b\r\n",\
    config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)), pre_value, post_value);
    return TRUE;
}

BOOL ToggleGPIO(const rom char* name)
{
    BYTE pre_value=0, post_value=0;
    IOPinId config;

    if(name == NULL){
        return FALSE;
    }

    if(!GetDefaultGPIOConfigbyName(name, &config)){
        return FALSE;
    }
    
    //Get current IOEXP bank gpios
    if(config.exp == J5){
        pre_value=ReadIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        pre_value=ReadIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    post_value = pre_value ^ (1U << IOEXP_PIN_BIT_OFFSET(config.number));
    
    //Write the new value
    if(config.exp == J5){
        WriteIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }
    else{
        WriteIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number), post_value);
    }

    DEBUG_MSG("ToggleGPIO: pin=%d, previous GPIO bank %c value=%08b, new value=%08b\r\n", \
    config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)), pre_value, post_value);
    return TRUE;
}

BOOL GetGPIO(const rom char* name, BYTE* value)
{
    IOPinId config;
    BYTE aux=0;
    
    if(name == NULL){
        return FALSE;
    }
    if(value == NULL){
        return FALSE;
    }
    
    if(!GetDefaultGPIOConfigbyName(name, &config)){
        return FALSE;
    }
    
    if(config.exp == J5){
        aux=ReadIOExp2(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    else{
        aux=ReadIOExp1(GPIOA + IOEXP_REG_BANK_OFFSET(config.number));
    }
    
    *value= (aux>> IOEXP_PIN_BIT_OFFSET(config.number)) & 1U;

    DEBUG_MSG("GetGPIO: pin=%d, GPIO bank %c value=%08b,%d\r\n", \
    config.number, ('A' + IOEXP_REG_BANK_OFFSET(config.number)), aux, *value);
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
