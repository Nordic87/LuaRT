# | tracker module for LuaRT
# | Luart.org, Copyright (c) Tine Samir 2023.
# | See Copyright Notice in LICENSE.TXT
# |--------------------------------------------------------------
# | Makefile
# | Please set LUART_PATH to your LuaRT folder if autodetection fails
# |--------------------------------------------------------------
# | Usage (default release build)			 : make
# | Usage (debug build) 		  			 : make debug
# | Usage (clean all)	 				 	 : make clean
# |-------------------------------------------------------------
VERBOSE= >nul 2>&1

MODULE=		tracker
VERSION=	0.5
SRC= 		src\tracker.obj

LUALIB= "$(LUART_PATH)\lib\lua55.lib"
CFLAGS = 

!if "$(BUILD)" == "debug"
CFLAGS = /nologo /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"./src/include" /I"$(LUART_PATH)\include" /I"." /Z7 $(CFLAGS)
LDFLAGS= /nologo /DEBUG
MODNAME=$(MODULE)
VERBOSE=
PROGRESS= 
!else
!if "$(BUILD)" == "static"
LUALIB= "$(LUART_PATH)\lib\lua55-static.lib"
CFLAGS = /nologo /DRTSTATIC /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"$(LUART_PATH)\include" /I"." /I"./src/include" /MD /Gy /GF /GR- /GL /GS- /Oi /O1 /GA $(CFLAGS)
LDFLAGS= /nologo /ignore:4217 /LTCG /NODEFAULTLIB:libcmt
MODNAME=$(MODULE)-static
!else
CFLAGS = /nologo /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"$(LUART_PATH)\include" /I"." /I"./src/include" /MD /Gy /GF /GR- /GL /GS- /Oi /O1 /GA $(CFLAGS)
LDFLAGS= /nologo /OPT:REF /OPT:ICF /LTCG /NODEFAULTLIB:libcmt
MODNAME=$(MODULE)
!endif
PROGRESS= "■"
!endif

LIBS= $(LUALIB) gdi32.lib kernel32.lib user32.lib comctl32.lib
RM= del /Q
CP= copy /Y
LD= link.exe

all: infomodule $(MODNAME).dll

static: 
	@$(MAKE) /nologo "BUILD=static"

debug:
	@$(MAKE) /nologo "BUILD=debug"

setup: infomodule $(MODNAME).dll
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v $(LUART_PATH)\modules\$(MODULE)\*.dll $(VERBOSE)

infomodule:  
	@chcp 65001 >nul 2>&1
!if "$(BUILD)" != "static"
	@cmd /c echo.
	@echo|set /p dummy="▸  Building module $(MODNAME) $(VERSION)	  "
!endif

.c.obj: 
	@$(CC) $(CFLAGS) /c /Fo$@ $< $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)

.cpp.obj: 
	@$(CC) $(CFLAGS) /c /Fo$@ $< $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)

$(MODNAME).dll: $(SRC)
	@$(LD) /DLL $(LDFLAGS) $(LIBS) $(SRC) /OUT:$@ $(VERBOSE)
	@$(RM) $(SRC)
!if "$(BUILD)" != "static"
	@$(MAKE) /nologo static
!endif

clean:
	@echo|set /p dummy="▸  Cleaning $(MODULE) module... "
	@$(RM) $(MODULE)*.* >nul 2>&1
	@$(RM) $(SRC) >nul 2>&1
	@echo ✓