/******************************************************************************
*******************************************************************************
**
**
**					rovim_t2d.h - Definitions for the "trac��o, travagem e 
**							direc��o" (T2D) module of the ROVIM project.
**
**		This module builds on and extends the firmware of the Dalf-1F motor 
**		control board to implement the T2D module of the ROVIM project.
**
**		It comprises, for the T2D, its describing structures and 
**		definitions.
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

#ifndef __ROVIM_T2D_H
#define __ROVIM_T2D_H
#include	"dalf.h"

//Default sample period for the expanded gpios
#define IO_SAMPLE_PERIOD 0x128		//200 ms

//Function prototypes
void ROVIM_T2D_Init(void);
BOOL ROVIM_T2D_LockMotorsAccess(void);
BOOL ROVIM_T2D_UnlockMotorsAccess(void);
BOOL ROVIM_T2D_LockBrake(void);
BOOL ROVIM_T2D_UnlockBrake(void);
BOOL ROVIM_T2D_ReadVehicleState(void);
BOOL ROVIM_T2D_ValidateInitialState(void);
BYTE CustomCmdDispatch(void);

#endif /*__ROVIM_T2D_H */
