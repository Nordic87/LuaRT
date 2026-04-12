# | sysutils module for LuaRT
# | Luart.org, Copyright (c) Tine Samir 2026.
# | See Copyright Notice in LICENSE.TXT
# |--------------------------------------------------------------
# | Makefile
# | Please set LUART_PATH to your LuaRT folder if autodetection fails
# |--------------------------------------------------------------
# | Usage (default release build)			 : make
# | Usage (debug build) 		  			 : make debug
# | Usage (clean all)	 				 	 : make clean
# |-------------------------------------------------------------
VERBOSE = 2>&1 >nul

MODULE=		sysutils
VERSION=	0.5

SRC_C= 		src\sysutils.c src\Process.c
SRC_CPP= 	

OBJ_CPP=    $(SRC_CPP:.cpp=.obj)
OBJ_C=		$(SRC_C:.c=.obj)
OBJ=		$(OBJ_CPP) $(OBJ_C)

LUALIB= "$(LUART_PATH)\lib\lua55.lib"
CFLAGS = 

!if "$(BUILD)" == "debug"
CFLAGS = /nologo /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"$(LUART_PATH)\include" /I"." /Z7 $(CFLAGS)
LDFLAGS= /nologo /DEBUG 
MODNAME=$(MODULE)
VERBOSE=
PROGRESS= 
!else
!if "$(BUILD)" == "static"
LUALIB= "$(LUART_PATH)\lib\lua55-static.lib"
CFLAGS = /nologo /DRTSTATIC /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"$(LUART_PATH)\include" /I"." /MD /Gy /GF /GR- /GL /GS- /Oi /O1 /GA $(CFLAGS)
LDFLAGS= /nologo /ignore:4217 /OPT:REF /OPT:ICF /LTCG /NODEFAULTLIB:libcmt
MODNAME=$(MODULE)-static
!else
CFLAGS = /nologo /DLUA_BUILD_AS_DLL /D_WIN32_WINNT=0x0603 /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"$(LUART_PATH)\include" /I"." /MD /Gy /GF /GR- /GL /GS- /Oi /O1 /GA $(CFLAGS)
LDFLAGS= /nologo /OPT:REF /OPT:ICF /LTCG
MODNAME=$(MODULE)
!endif
PROGRESS= "■"
!endif

LIBS= $(LUALIB) advapi32.lib shell32.lib ole32.lib user32.lib

RM= del /Q
CP= copy /Y
LD= link.exe
MV = move /Y

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

!if "$(SRC_C)" != ""
compile_c:
	@$(CC) $(CFLAGS) /c /MP /TC $(SRC_C) $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
!else
compile_c:
!endif

!if "$(SRC_CPP)" != ""
compile_cpp:
	@$(CC) $(CFLAGS) /c /MP $(SRC_CPP) $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
!else
compile_cpp:
!endif

$(MODNAME).dll: compile_c compile_cpp
	@-$(MV) *.obj src/  >nul 2>&1
	@$(LD) /DLL $(LDFLAGS) $(OBJ) $(LIBS) /OUT:$@ $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-mkdir $(LUART_PATH)\modules\$(MODULE) >nul 2>&1
	@$(CP) $@ $(LUART_PATH)\modules\$(MODULE) >nul 2>&1
	@$(RM) $(OBJ)
!if "$(BUILD)" != "static"
	@$(MAKE) /nologo static
!endif

clean:
	@echo|set /p dummy="▸  Cleaning $(MODULE) module... "
	@$(RM) $(MODULE)*.* >nul 2>&1
	@$(RM) $(OBJ) >nul 2>&1
	@echo ✓
	@-$(RM) $(LUART_PATH)\modules\$(MODULE)\$(MODULE).dll >nul 2>&1
	@-$(RM) $(LUART_PATH)\modules\$(MODULE)\$(MODULE)-static.dll >nul 2>&1
	@-rmdir /S /Q $(LUART_PATH)\modules\$(MODULE) >nul 2>&1 