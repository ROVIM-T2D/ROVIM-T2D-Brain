/******************************************************************************
*******************************************************************************
**
**
**                  rovim_t2d.h - Definitions for the "tracção, travagem e 
**                          direcção" (T2D) module of the ROVIM project.
**
**      This module builds on and extends the firmware of the Dalf-1F motor 
**      control board to implement the T2D module of the ROVIM project.
**
**      It comprises, for the T2D, its describing structures and 
**      definitions.
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

#ifndef __ROVIM_T2D_H
#define __ROVIM_T2D_H
#include    "dalf.h"

//Function prototypes
void ROVIM_T2D_Init(void);
void ROVIM_T2D_Start(void);
void ROVIM_T2D_ConfigGPIOs(void);
void ROVIM_T2D_Greeting(void);
void ROVIM_T2D_ServiceIO(void);
BOOL ROVIM_T2D_LockBrake(void);
BOOL ROVIM_T2D_UnlockBrake(void);
BOOL ROVIM_T2D_ValidateState(void);

//functions accessible from the command line
BYTE ROVIM_T2D_CmdDispatch(void);
BYTE ROVIM_T2D_Lockdown(void);
BYTE ROVIM_T2D_ReleaseFromLockdown(void);
BYTE ROVIM_T2D_ControlGPIO(void);
BYTE ROVIM_T2D_Accelerate(void);
BYTE ROVIM_T2D_Decelerate(void);
BYTE ROVIM_T2D_SetSpeed(void);


//definitions
//command codes
#define ROVIM_T2D_LOCKDOWN_CMD_CODE         (CUSTOM_CMD_ID_OFFSET)
#define ROVIM_T2D_RELEASE_CMD_CODE          (CUSTOM_CMD_ID_OFFSET+1)
#define ROVIM_T2D_CONTROL_GPIO_CMD_CODE     (CUSTOM_CMD_ID_OFFSET+2)
#define ROVIM_T2D_ACCELERATE_CMD_CODE       (CUSTOM_CMD_ID_OFFSET+3)
#define ROVIM_T2D_DECELERATE_CMD_CODE       (CUSTOM_CMD_ID_OFFSET+4)
#define ROVIM_T2D_SET_SPEED_CMD_CODE        (CUSTOM_CMD_ID_OFFSET+5)

//Traction accelerator and decelerator software PWM definitions
#define ROVIM_T2D_PWM_MIN_DUTY 10   //Minimum duty cycle and duty cycle increase. This should be a divider of 100(%)
#define ROVIM_T2D_PWM_CNTS_PER_PERIOD 100/ROVIM_T2D_PWM_MIN_DUTY   //number of duty cycles in a PWM period

#define ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC 220   //time constant of the low pass RC filter converting the accelerator and decelerator signals to analogue, in ms (47k*47u)
#define ROVIM_T2D_UPPER_SPEED_LIMIT 100  //maximum speed of the vehicle, in Km/h/10
#define ROVIM_T2D_LOWER_SPEED_LIMIT 10   //minimum speed of the vehicle, in Km/h/10

/*-wheel diameter of the axle where torque is applied: 55 cm
-number of teeth of the gear where the speed encoder is mounted: 11
-meters traveled by the vehicle per gear pitch (where the encoder is mounted): 2*pi*r/11=157mm*/
#define ROVIM_T2D_ENCODER_CNT_TO_MM 157 

#endif /*__ROVIM_T2D_H */
