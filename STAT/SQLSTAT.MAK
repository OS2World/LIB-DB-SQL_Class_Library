# Created by IBM WorkFrame/2 MakeMake at 17:42:50 on 09/11/95
#
# This makefile should be run in the following directory:
#   d:\ibk\class\stat
#
# The actions included in this makefile are:
#   COMPILE::C++
#   LIB::Object Library Manager

.all: \
  ..\ksql001s.lib

.SUFFIXES:

.SUFFIXES: .cpp

.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /O /N30 /Gm /Gs /Ft- /C %s

{d:\ibk\class}.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /O /N30 /Gm /Gs /Ft- /C %s

..\ksql001s.lib: \
    .\koracle.obj \
    .\ksql.obj \
    sqlstat.MAK
      @echo WF::LIB::Object Library Manager
      lib.exe  /nol ..\ksql001s.lib -+.\koracle.obj -+.\ksql.obj;

.\koracle.obj: \
    d:\ibk\class\koracle.cpp \
    {d:\ibk\class;$(INCLUDE);}ksql.hpp \
    {d:\ibk\class;$(INCLUDE);}koracle.hpp \
    sqlstat.MAK

.\ksql.obj: \
    d:\ibk\class\ksql.cpp \
    {d:\ibk\class;$(INCLUDE);}ksql.hpp \
    sqlstat.MAK

