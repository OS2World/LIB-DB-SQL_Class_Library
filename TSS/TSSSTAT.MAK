# Created by IBM WorkFrame/2 MakeMake at 17:53:18 on 09/11/95
#
# This makefile should be run in the following directory:
#   d:\ibk\class\tss
#
# The actions included in this makefile are:
#   COMPILE::C++
#   LINK::ICC Link

.all: \
  .\tss.exe

.SUFFIXES:

.SUFFIXES: .cpp

.cpp.obj:
      @echo WF::COMPILE::C++
      icc.exe /Tdp /Q /O /N30 /Gm /Ft- /C %s

.\tss.exe: \
    .\tss.obj \
    {$(LIB)}ksql001s.lib \
    {$(LIB)}ora7o32.lib \
    {$(LIB)}dde4muib.lib \
    {$(LIB)}dde4cc.lib \
    tssstat.MAK
      @echo WF::LINK::ICC Link
      icc.exe @<<
 /B" /f /packd /nologo"
 /Fetss.exe 
 ksql001s.lib 
 ora7o32.lib 
 dde4muib.lib 
 dde4cc.lib 
 .\tss.obj
<<

.\tss.obj: \
    d:\ibk\class\tss\tss.cpp \
    tssstat.MAK

