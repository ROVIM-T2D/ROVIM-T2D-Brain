<readme.txt>

This file is provided to assist in development when using standard Microchip
tools to create a hex image to be flashed into the PIC18F6722 part on the Dalf
Motor Control Board.

The recommended project options are:
Language Toolsuite:
		Microchip C18 Toolsuite


Build Environment:

<user.mcp>
==========
     Source Files
          main.c
		  rovim_t2d.c
		  dalf_ext.c
		  test.c
     Header Files
          config.h
          dalf.h
          p18f6722.h
		  rovim.h
		  rovim_t2d.h
		  test.h
     Object Files
          bootload.o
		  c018i.o
     Library Files
          dalf.lib
     Linker Scripts
          useri.lkr
     Other Files
          (none)


Build Options for Project
=============
	 DIRECTORIES
		Include Search Path: <add path to the include directories. 
		Ex: "Z:\rovim\git\src\test\inc";
			"Z:\rovim\git\src\rovim\inc";
			"Z:\rovim\git\src\dalf\inc">
		Library Search Path: <add path to the c018i.o file. Ex: "Z:\rovim\git\binary">

     GENERAL
        Library Path: C:\mcc18\lib

     MPLAB C18
		General
		  Macro Definitions: <add the macro defining your system configuration 
		  profile. Ex: "INCLUDE_CONFIG_V0_1">
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
1) Install the MPLAB and MCC18 on your machine. The software versions of these
   tools are provided. It is strongly recommended to use only the versions
   provided. The working operating system should be Windows XP or older. This
   MPLAB version was unsuccessfully tested in Windows 7. Use a Virtual Machine.
   If you use a Virtual Machine, VMWare is recommended, as its USB handling is 
   much more straightforward than VirtualBox. The free version doesn't support 
   snapshots, though.

2) The c018i.c file is provided for information only (compare with that provided
   in your mcc18 directory).  It is not necessary to compile it (in fact without
   good reason you shouldn't), just use the provided co18i.o object. Since it is
   no longer in the root directory, you have to point the library search path 
   to it. Do not include c018i.o in the project files (those listed in 
   user.mcp), it will be used as the startup code by virtue of being in the 
   library search path.

3) When you flash the image, I recommend that you -AVOID- reflashing the boot 
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
Modified for the ROVIM Project.