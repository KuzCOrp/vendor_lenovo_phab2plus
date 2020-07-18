NXP TFA Host SW REAME
=====================

This document describes the folder structure and contents of the folder used for NXP TFA device Host SW.

Folders and Source Files 
========================
  |-- Makefile                        > generic Linux/cygwin make file
  |-- Android.mk                        > android/external make file
  |-- SConstruct                        > scons for Linux/IOS/cygwin build
  |-- doc                                > Documentation folder, containing this README.
  |-- app                                > The application layer code resides here. 
  |    |-- climax                        > The source for climax commandline utility
  |    |    |-- cli                        > cli parser
  |    |    |   |-- climax_cmdline.ggo        > gengetopt input to generate cmdline.[ch]
  |    |    |   |-- cmdline.c                > generated cliparser
  |    |    |   `-- testcmd.c                -    not used
  |    |    |-- cliCommands.c                > connects cli options to tfa98xx application 
  |    |    `-- climax.c                    > main entry
  |    `-- exTfa98xx                    > It includes the mono and stereo example. 
  |-- srv                                > The service layer code resides here. It supports the application layer for runtime features.
  |   |-- inc
  |   |-- tfa98xxCalibration.h    > Calibration      
  |   |-- tfa98xxDiagnostics.h    > Diagnostics      
  |   `-- tfa98xxLiveData.h        > life data demo code
  |   `-- src                            > Support/Implementation for calibration, live data collection and diagnostics.
  |       |-- tfa98xxCalibration.c           
  |       |-- tfa98xxDiagnostics.c            
  |       `-- tfa98xxLiveData.c        
  |-- tfa                                > The TFA device specific files are available here. This was formally known as API layer. It includes the register definitions.
  |   |-- inc
  |   |-- tfa98xx.h                > Tfa Device (formaerly the API layer)
  |   |   `-- Tfa9887_Registers.h
  |   `-- src
  |       |-- tfa98xx.c                >    
  |       |-- Tfa98*_TextSupport.c
  |       `-- Tfa98*_internals.h
  `-- hal                                > The hardware abstraction layer code resides here.
    |-- inc
    |   |-- NXP_I2C.h                > TFA hal
    |   |-- ErrorCodes.h        > scribo errorcodes
    |   `-- cmd.h                > scribo protocol parser
    `-- src
        |-- NXP_I2C_*.c            > It contains the I2C (and Scribo) interface used to communicate with the device.
        `-- lxScribo                    > --i2c device abstraction--
            |-- lxDummy.c                > device=dummy: simple tfa device simulator 
            |-- lxI2c.c                    > device=/dev/i2c-4: direct i2c bus
            |-- lxScribo.c                > lxScribo interface registry and wrapper
            |-- lxScriboSerial.c        > device=/dev/ttyUSB0: devkit serial (over USB) 
            |-- lxScriboSocket.c        > device=localhost:9887: remote socket interface
            `-- scribosrv                    > --socket server--
              |-- cmd.c                    > scribo protocol parser
              `-- i2cserver.c                > interface to TFA98xx hal
  
Building
========  
  The TFA uses the make utility to build executable programs and libraries. The make file calls individual make for each layer described above. The individaul make files can be found under bld/ sub folder. For windows, MS Visual Project 2010 can be used or if MSBuild and Cygwin is installed, then can be called directly from makefile. 
  Linux is currently the default build. This will be extended with andriod build. 
