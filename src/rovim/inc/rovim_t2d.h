/******************************************************************************
*******************************************************************************
**
**
**                  rovim_t2d.h - Definitions for the "trac��o, travagem e 
**                          direc��o" (T2D) module of the ROVIM project.
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
void ROVIM_T2D_MonitorSystem(void);
BOOL ROVIM_T2D_LockBrake(void);
BOOL ROVIM_T2D_UnlockBrake(void);
BOOL ROVIM_T2D_ValidateState(void);
void ROVIM_T2D_ServicePWM(void);
BOOL ROVIM_T2D_FinishReleaseFromLockdown(void);
void ROVIM_T2D_MonitorSystem(void);
BOOL ROVIM_T2D_DetectTractionEror(void);
BOOL ROVIM_T2D_DetectBrakingError(void);
BOOL ROVIM_T2D_DetectDirectionEror(void);
void ROVIM_T2D_MonitorTractionWarning(void);
void ROVIM_T2D_MonitorBrakingWarning(void);
void ROVIM_T2D_MonitorDirectionWarning(void);
BOOL ROVIM_T2D_DetectFatalError(void);
void ROVIM_T2D_MonitorBrakeUnlock(void);
void ROVIM_T2D_LockUnusedResourcesAccess(void);
void ROVIM_T2D_UpdateVel1Acc1(void);
void ROVIM_T2D_UpdateVel2(void);
void ROVIM_T2D_ConfigDefaultParamBlock(void);
void ROVIM_T2D_ConfigDirParamBlock(void);
void ROVIM_T2D_LockCriticalResourcesAccess(void);
void ROVIM_T2D_UnlockCriticalResourcesAccess(void);
BOOL ROVIM_T2D_IsCommandLocked(BYTE cmd);
void ROVIM_T2D_MonitorManualMode(void);
void ROVIM_T2D_MonitorMovementChanges(void);
void ROVIM_T2D_FullBrake(void);

//functions accessible from the command line
BYTE ROVIM_T2D_CmdDispatch(void);
BYTE ROVIM_T2D_Lockdown(void);
BYTE ROVIM_T2D_ReleaseFromLockdown(void);
BYTE ROVIM_T2D_ControlGPIO(void);
BYTE ROVIM_T2D_Accelerate(void);
BYTE ROVIM_T2D_Decelerate(void);
BYTE ROVIM_T2D_SetMovement(void);
BYTE ROVIM_T2D_Turn(void);

extern WORD    ROVIM_T2D_sysmonitorcount;            // ROVIM T2D system state monitoring timeout counter;
extern WORD    ROVIM_T2D_pwmrefreshcount;            // ROVIM T2D PWM refresh timeout counter;

extern long vel2;
extern long acc1;
extern long vel1;

typedef struct{
    BYTE type;
    long speed;
}movement;

//definitions
//command codes
#define ROVIM_T2D_LOCKDOWN_CMD_CODE         (CUSTOM_CMD_ID_OFFSET)
#define ROVIM_T2D_RELEASE_CMD_CODE          (CUSTOM_CMD_ID_OFFSET+1)
#define ROVIM_T2D_CONTROL_GPIO_CMD_CODE     (CUSTOM_CMD_ID_OFFSET+2)
#define ROVIM_T2D_ACCELERATE_CMD_CODE       (CUSTOM_CMD_ID_OFFSET+3)
#define ROVIM_T2D_DECELERATE_CMD_CODE       (CUSTOM_CMD_ID_OFFSET+4)
#define ROVIM_T2D_SET_MOVEMENT_CMD_CODE     (CUSTOM_CMD_ID_OFFSET+5)
#define ROVIM_T2D_TURN_CMD_CODE             (CUSTOM_CMD_ID_OFFSET+6)


//Traction system related definitions
//Minimum duty cycle and duty cycle increase. This should be a divider of 100(%)
#define ROVIM_T2D_PWM_MIN_STEP 0
//relation between PWM period and duty cycle
#define ROVIM_T2D_PWM_CNTS_PER_PERIOD 100
//time constant of the low pass RC filter converting the accelerator and decelerator signals to analogue, in ms (47k*47u)
#define ROVIM_T2D_TIME_CONSTANT_ACC_DCC_DAC 220
//traction PWM signal refresh period, in ms
#define ROVIM_T2D_PWM_REFRESH_PERIOD 1
//maximum speed the user can order the vehicle to move, in Km/h/10
#define ROVIM_T2D_MAX_SPEED 80
//maximum speed the vehicle can achieve at any point in time, in Km/h/10
#define ROVIM_T2D_CRITICAL_SPEED (ROVIM_T2D_MAX_SPEED*12/10)
//minimum speed of the vehicle, in Km/h/10
/*minimum speed should be !=0. To have a closed loop trying to follow a too smal reference, may
cause stability problems (I think!). 0.2 km/h is a very slow speed and easy achievable by the 
system. It takes ~10% duty cycle to achieve, which seems a reasonable starting point.*/
#define ROVIM_T2D_LOWER_SPEED_LIMIT 2
//number of encoder gear teeth
#define ROVIM_T2D_TPR 11
//Average perimeter of the real wheel (depends on tire tread, load and tire pressure), in cm*/
#define ROVIM_T2D_WHEEL_PERIMETER 176
//sincronize this period with VSP for traction encoder
#define ROVIM_T2D_VSP1 250  //ms
#define ROVIM_T2D_SYSTEM_MONITOR_PERIOD   10000//test //VSP1          //ms


#define WATCHDOG_PERIOD 0xFA        //250 ms


//threshold of stressful motor operation (indicating some sort of error), in % of maximum PWM
//totally arbitrary number defined by me at this point (ah, but which point is it?? You'll never know. God, I need to see a shrink...)
#define ROVIM_T2D_DIRECTION_MOTOR_STRESS_DUTY_CYCLE_THRESHOLD 80    //% of VMAX

//threshold of stressful situations measured on the motor to formally declare an error
#define ROVIM_T2D_DIRECTION_MOTOR_STRESS_CNT_THRESHOLD 1024 //ex: at 0,2s sample time, it gives ~3 m of continuous operation at high stress before declaring an error

//time thresold of continuous press of unclamp button to declare something is wrong.
#define ROVIM_T2D_UNCLAMP_ACTION_ACTIVE_TIME_LIMIT  1024
//time it takes for the brake to go from fully unclamped to fully clamped, in ms
#define ROVIM_T2D_BRAKE_CLAMP_TIME 6000
//maximum time it may take for the velocity to reach the defined error band of the final value on closed loop traction speed control, in ms
#define ROVIM_T2D_MAX_SETTLING_TIME 10000
//traction acceleration threshold that defines an error situation, such as crash, or short circuit, in Km/10/h/s
#define ROVIM_T2D_CRASH_ACC_THRESHOLD 50
//maximum time needed to stop the vehicle once it has been set on hold, in ms
#define ROVIM_T2D_MOVING_ON_HOLD_TIMEOUT 5000
//maximum continuous time the user can be pressing the manual brake unlock button
#define ROVIM_T2D_BRAKE_UNLOCK_TIMEOUT 30000/ROVIM_T2D_SYSTEM_MONITOR_PERIOD //

//total direction safe travel, in degrees (should be an even number)
#define ROVIM_T2D_DIR_ANGULAR_RANGE 80
//upper tick count (potentiometer value) that the direction can safely reach
#define ROVIM_T2D_DIR_TICK_UPPER_LIMIT 0xC0
//lower tick count (potentiometer value) that the direction can safely reach
#define ROVIM_T2D_DIR_TICK_LOWER_LIMIT 0x38
//direction motor mode1 flags. See dalf owners manual
#define ROVIM_T2D_DIR_MODE1 0x32
//direction motor mode2 flags. See dalf owners manual
#define ROVIM_T2D_DIR_MODE2 0x00
//direction motor mode3 flags. See dalf owners manual
#define ROVIM_T2D_DIR_MODE3 0x21
//velocity sampling period for direction motor
#define ROVIM_T2D_DIR_VSP 20
//minimum PWM duty cycle for direction control
#define ROVIM_T2D_DIR_MIN_PWM 20
//maximum PWM duty cycle for direction control
#define ROVIM_T2D_DIR_MAX_PWM 100
//maximum analog error -  not important in this application
#define ROVIM_T2D_DMAX 255
#endif /*__ROVIM_T2D_H */
