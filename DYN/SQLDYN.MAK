# Created by IBM WorkFrame/2 MakeMake at 20:04:29 on 09/07/95
#
# This makefile should be run in the following directory:
#   d:\ibk\class\dyn
#
# The actions included in this makefile are:
#   COMPILE::C++
#   LIB::Import Lib
#   LINK::ICC Link

.all: \
  ..\ksql001d.LIB

.SUFFIXES:

.SUFFIXES: .dll .cpp

.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /O /N30 /Gm /Gd /Ge- /Ft- /C %s

{d:\ibk\class}.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /O /N30 /Gm /Gd /Ge- /Ft- /C %s

.dll.LIB:
      @echo WF::LIB::Import Lib
      implib.exe %|dpfF.LIB %s

{d:\ibk\class}.dll.LIB:
      @echo WF::LIB::Import Lib
      implib.exe %|dpfF.LIB %s

..\ksql001d.dll: \
    .\koracle.obj \
    d:\ibk\class\ksql001d.def \
    .\ksql.obj \
    {$(LIB)}ora7o32.lib \
    {$(LIB)}dde4muii.lib \
    {$(LIB)}dde4cci.lib \
    sqldyn.MAK
      @echo WF::LINK::ICC Link
      icc.exe @<<
 /B" /f /packd /nologo /m:full"
 /Fe..\ksql001d.dll 
 ora7o32.lib 
 dde4muii.lib 
 dde4cci.lib 
 .\koracle.obj
 d:\ibk\class\ksql001d.def
 .\ksql.obj
<<

.\koracle.obj: \
    d:\ibk\class\koracle.cpp \
    {d:\ibk\class;$(INCLUDE);}ksql.hpp \
    {d:\ibk\class;$(INCLUDE);}koracle.hpp \
    sqldyn.MAK

.\ksql.obj: \
    d:\ibk\class\ksql.cpp \
    {d:\ibk\class;$(INCLUDE);}ksql.hpp \
    sqldyn.MAK

..\ksql001d.LIB: \
    ..\ksql001d.dll \
    sqldyn.MAK

