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
void ROVIM_T2D_Greeting(void);
BOOL ROVIM_T2D_LockBrake(void);
BOOL ROVIM_T2D_UnlockBrake(void);
BOOL ROVIM_T2D_ReadVehicleState(void);
BOOL ROVIM_T2D_ValidateInitialState(void);
BYTE ROVIM_T2D_CustomCmdDispatch(void);
void ROVIM_T2D_ServiceIO(void);

#endif /*__ROVIM_T2D_H */
