# Created by IBM WorkFrame/2 MakeMake at 10:15:16 on 06/16/95
#
# This makefile should be run in the following directory:
#   d:\ibk\class\tss
#
# The actions included in this makefile are:
#   COMPILE::C++
#   LINK::ICC Link

.all: \
  .\tssd.exe

.SUFFIXES:

.SUFFIXES: .cpp

.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /Ti /N30 /Gm /Gd /Ft- /C %s

.\tssd.exe: \
    .\tss.obj \
    {$(LIB)}ksql001d.lib \
    {$(LIB)}dde4muii.lib \
    {$(LIB)}dde4cci.lib \
    tssdyn.MAK
      @echo WF::LINK::ICC Link
      icc.exe @<<
 /B" /de /nologo"
 /Fetssd.exe 
 ksql001d.lib 
 dde4muii.lib 
 dde4cci.lib 
 .\tss.obj
<<

.\tss.obj: \
    d:\ibk\class\tss\tss.cpp \
    tssdyn.MAK

