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

#include <stdio.h>
#include "dalf.h"

#ifndef __TEST_H
#define __TEST_H

//Verbosity level mask
#define VERBOSITY_DISABLED				0x00
#define VERBOSITY_LEVEL_ERROR 			0x01
#define VERBOSITY_LEVEL_WARNING			0x02
#define VERBOSITY_LEVEL_STATUS 			0x04
#define VERBOSITY_USE_TIMESTAMP			0x10

//Verbosity print macros
//Q: Why on earth would you do such a long macro?
//A: Because idk how to properly pass variable # of arguments to a subfunction...
#define ERROR_MSG(ARGS)				do { \
										if(GetVerbosity() & VERBOSITY_LEVEL_ERROR) { \
											printf("ERROR!: "); \
											if (GetVerbosity() & VERBOSITY_USE_TIMESTAMP) { \
												PrintTime(NULL); \
											} \
											printf("\t"); \
											printf(ARGS); \
										} \
									} while(0)
#define WARNING_MSG(ARGS)			do { \
										if(GetVerbosity() & VERBOSITY_LEVEL_WARNING) { \
											printf("WARNING!: "); \
											if (GetVerbosity() & VERBOSITY_USE_TIMESTAMP) { \
												PrintTime(NULL); \
											} \
											printf("\t"); \
											printf(ARGS); \
										}\
									} while(0)
#define STATUS_MSG(ARGS)				do { \
										if(GetVerbosity() & VERBOSITY_LEVEL_STATUS) { \
											printf("STATUS: "); \
											if (GetVerbosity() & VERBOSITY_USE_TIMESTAMP) { \
												PrintTime(NULL); \
											} \
											printf("\t"); \
											printf(ARGS); \
										} \
									} while(0)

//prototypes
void TEST_TestInit(void);
void STATUS_CmdReceived(void);
void SetVerbosity(BYTE level);
BYTE GetVerbosity(void);
void TEST_InDevelopmentTesting(void);
void LOG_LogInit(void);

#endif /*__TEST_H*/
