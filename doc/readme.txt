<readme.txt>

This file is provided to assist in development when using standard Microchip
tools to create a hex image to be flashed into the PIC18F6722 part on the Dalf
Motor Control Board.


Build Environment:

<user.mcp>
==========
     Source Files
          main.c
     Header Files
          config.h
          dalf.h
          p18f6722.h
     Object Files
          bootload.o
     Library Files
          dalf.lib
     Linker Scripts
          useri.lkr
     Other Files
          (none)


Build Options
=============
     GENERAL
        Library Path: C:\mcc18\lib

     MPLAB C18
        Memory Model
          Code:  Large code model
          Data:  Large data model
          Stack: Mult-bank model

        Optimization
          Defaults

     MPASM/C17/C18 SUITE
        Build normal target

     MPLINK
        INHX32 Format
        Generate Map File

     MPASM
        Output
          INHX32
        General
          Default Radix = Hex


Notes:
1) The c018i.c file is provided for information only (compare with that provided
   in your mcc18 directory).  It is not necessary to compile it (in fact without
   good reason you shouldn't), just use the provided co18i.o object which should
   be located in the build root directory.  Do not include c018i.o in the
   project files (those listed in user.mcp), it will be used as the startup code
   by virtue of being in the project directory.

2) When you flash the image, I recommend that you -AVOID- reflashing the boot 
   block (0x000 - 0x7FF) section of your image.  This is really less important
   if you are flashing your code with ICD2 tool because you will always be able
   to restore the full image with the ICD2, but it won't hurt to follow this
   procedure always.  If the boot block is changed, you may not be able to use
   the COMM Port to obtain field upgrades using the boot loader feature.

        Assuming that you are using the MPLAB IDE Development Environment to
        create your image, just export your .hex image to a new file starting
        at 0x00800. Use your .map file to determine the ending address.  For
        example if your map file indicates your code ends at 0x0b731, just
        indicate an ending address of 0x0b7FF to end your image on a 2K
        boundary.  The bytes from 0x0b732 thru 0x0b7ff will be filled with
        0xFF's.


Embedded Electronics, LLC.