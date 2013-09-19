/******************************************************************************
*******************************************************************************
**
**
**					test.h - Definitions for the debug and test module of
**							the ROVIM T2D and Dalf-1F modules.
**
**		This module implements the debug information reporting and testing
**		funcionality of	the original Dalf-1F firmware and its ROVIM T2D
**		extension.
**		It covers status reports, warning and error information and
**		unitary and integrated test procedures.
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

#include "dalf.h"

#ifndef __TEST_H
#define __TEST_H

//prototypes
void TEST_TestInit(void);
void TEST_InDevelopmentTesting(void);

#endif /*__TEST_H*/
