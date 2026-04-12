# | LuaRT - A Windows programming framework for Lua
# | Copyright (c) Tine Samir 2026
# | See Copyright Notice in LICENSE.TXT
# |--------------------------------------------------------------
# | LuaRT Core Makefile for the toolchain, the Core framework and the tools
# | You can prefix all the following tasks with PLATFORM=x64|x86  
# | Don't forget to get all submodules :
# |--------------------------------------------------------------
# | Usage (default release build)			 : nmake
# | Usage (debug build) 		  			 : nmake debug
# | Usage (release with tools)		 		 : nmake all
# | Usage (build setup executable)	  		 : nmake setup
# | Usage (build tools)				  		 : nmake tools
# | Usage (clean all)	 				 	 : nmake clean
# |-------------------------------------------------------------
# | Or you can use default release build for any platform 
# | Usage (default x64 release build)		 : nmake x64
# | Usage (default x86 release build)		 : nmake x86
# |-------------------------------------------------------------

VERBOSE = 2>&1 >nul

PLATFORM=

!if ([cl /? 2>&1 | findstr /C:"x86" > nul] == 0)
PLATFORM=x86
!else
PLATFORM=x64
!endif

#---- Configuration
RM= del /Q
CC= cl
LD= link
LB= lib

DEST= ..\..\bin

!if "$(PLATFORM)" == "x64"
_ARCH = x64
!else
_ARCH = x86
!endif

#---- LuaRT version
LUART_MAJOR= 2
LUART_MINOR= 2
LUART_RELEASE= 0
!if "$(VERSION)" == ""
VERSION=$(LUART_MAJOR).$(LUART_MINOR).$(LUART_RELEASE)
!endif
DEFINES_RES= /d LUART_MINOR="$(LUART_MINOR)" /d LUART_MAJOR="$(LUART_MAJOR)" /d LUART_RELEASE="$(LUART_RELEASE)"

!if "$(BUILD)" == "debug"
CFLAGS = /nologo /D_WIN32_WINNT=0x0603 /DLUA_BUILD_AS_DLL /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DZIP_SHARED /DZIP_BUILD_SHARED /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"..\..\include" /I"." /Z7 /Od
LDFLAGS= /nologo /DEBUG /NODEFAULTLIB:msvcrt
VERBOSE= 
PROGRESS=
!else
CFLAGS = /nologo /D_WIN32_WINNT=0x0603 /DLUA_BUILD_AS_DLL /DLUA_COMPAT_5_3 /DLUA_ARCH=\"$(_ARCH)\" /DZIP_SHARED /DZIP_BUILD_SHARED /DLUART_MINOR=$(LUART_MINOR) /DLUART_MAJOR=$(LUART_MAJOR) /DLUART_RELEASE=$(LUART_RELEASE) /I"..\..\include" /I"." /MD /Gy /GF /Oi /O1 /GA
LDFLAGS= /nologo /OPT:REF /OPT:ICF
PROGRESS= "■"
!endif

DLL_LIBS =	shlwapi.lib advapi32.lib gdi32.lib comctl32.lib kernel32.lib user32.lib oleaut32.lib ole32.lib credui.lib crypt32.lib wininet.lib shell32.lib

LIBS	= 		..\..\lib\lua55.lib ole32.lib comctl32.lib shell32.lib user32.lib
LIBS_STATIC	= 	..\..\lib\lua55-static.lib ole32.lib comctl32.lib shell32.lib shlwapi.lib advapi32.lib gdi32.lib comctl32.lib kernel32.lib user32.lib oleaut32.lib crypt32.lib

#---- Source files
LUA_A=		..\..\bin\lua55.dll
LUA_O=		lua\lapi.obj lua\lcode.obj lua\lctype.obj lua\ldebug.obj lua\ldo.obj lua\ldump.obj lua\lfunc.obj lua\lgc.obj lua\llex.obj lua\lmem.obj lua\lobject.obj lua\lopcodes.obj lua\lparser.obj lua\lstate.obj lua\lstring.obj lua\ltable.obj lua\ltm.obj lua\lundump.obj lua\lvm.obj lua\lzio.obj	
LIB_O=		lua\lauxlib.obj lua\lbaselib.obj lua\lcorolib.obj lua\ldblib.obj lua\lmathlib.obj lua\loadlib.obj lua\ltablib.obj string\string.obj string\lstrlib.obj sys\sys.obj console\console.obj lua\liolib.obj lua\loslib.obj lua\lutf8lib.obj compression\compression.obj compression\Zip.obj compression\lib\zip.obj lpreprocess.obj lrtapi.obj lrtobject.obj sys\Date.obj sys\File.obj sys\Pipe.obj sys\Directory.obj sys\Buffer.obj sys\Com.obj lembed.obj sys\async.obj sys\Task.obj

BASE_O= 	$(LUA_O) $(LIB_O)
ALL_O= 		$(BASE_O)

LUART_T=	..\..\bin\luart.exe
LUAWRT_T=	..\..\bin\wluart.exe
RTC_T=		..\..\bin\rtc.exe
ALL_T= 		$(LUA_A) $(LUART_T) $(LUAWRT_T) $(RTC_T)

!if "$(BUILD)" != "debug"
!message 
!message --------------------  Building LuaRT $(LUART_MAJOR).$(LUART_MINOR).$(LUART_RELEASE) for $(PLATFORM)  --------------------
!endif

all: $(RTC_T) toolchain

debug:
	@$(MAKE) /nologo "BUILD=debug"

utf8:
	@chcp 65001 >nul 2>&1

default: toolchain

infolua: utf8
	@cmd /c echo.
	@echo|set /p dummy="▸  Building Lua 5.5.0 VM          "

infocore:
	@cmd /c echo.
	@echo|set /p dummy="▸  Building LuaRT core framework  "

infostatic:
	@cmd /c echo.
	@echo|set /p dummy="▸  Building static interpreters   "

infotoolchain:
	@cmd /c echo.
	@echo|set /p dummy="▸  Building LuaRT toolchain       "

infosetup: utf8
	@cmd /c echo.
	@echo|set /p dummy="▸  Building LuaRT installer       "

.cpp.obj:
	@$(CC) /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Fo$@ $< $(VERBOSE) /EHsc
	@echo|set /p dummy=$(PROGRESS)

.c.obj: 
	@$(CC) /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Fo$@ $< $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)

lua: infolua $(LUA_O) 

core: infocore $(LIB_O)

toolchain: lua core $(LUA_A) infotoolchain $(LUART_T) $(LUAWRT_T) static

lua\lvm.obj: 
!if "$(BUILD)" == "debug"
	@$(CC) /c $(CFLAGS) /Fo$@ lua\lvm.c $(VERBOSE)
!else
	@$(CC) /nologo /MD /Gy /GF /Oi /GA /c /O2 /Fo$@ lua\lvm.c $(VERBOSE)
!endif

..\..\lib\lua55-static.lib: $(LUART_T) 
	@..\..\bin\luart.exe lua\static\export_h.lua $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /c lua\static\lib.cpp $(CFLAGS) /Folua\static\lib.obj $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@$(LB) lua\static\lib.obj /OUT:lua55-static.lib $(VERBOSE)
	@-copy /Y lua55-static.lib "..\..\lib\lua55-static.lib" >nul
	@echo|set /p dummy=$(PROGRESS)

..\..\bin\luart-static.exe: $(BASE_O)
	@$(CC) /DRTSTATIC /c $(CFLAGS) lua\loadlib.c /Folua\loadlib.obj $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@rc /fo .\\resource.res $(DEFINES_RES) /r /v /i "lua/" resources/resource.rc >nul 2>&1
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DRTSTATIC $(CFLAGS) luart.c $(BASE_O) $(LIBS_STATIC) /link /ignore:4217 $(LDFLAGS) resource.res /OUT:luart-static.exe $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-copy /Y luart-static.exe "$(DEST)\luart-static.exe" >nul

..\..\bin\wluart-static.exe: $(BASE_O)
	@rc /fo .\\resource.res $(DEFINES_RES) /d RTWIN /r /v /i "lua/" resources/resource.rc >nul 2>&1
	@$(CC) /DRTSTATIC /DRTWIN $(CFLAGS) luart.c resource.res $(BASE_O) $(LIBS_STATIC) /link /ignore:4217 $(LDFLAGS) /OUT:wluart-static.exe /SUBSYSTEM:windows $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-copy /Y wluart-static.exe "$(DEST)\wluart-static.exe" >nul

static: infostatic ..\..\lib\lua55-static.lib ..\..\bin\luart-static.exe ..\..\bin\wluart-static.exe
	@$(CC) /c $(CFLAGS) lua\loadlib.c /Folua\loadlib.obj $(VERBOSE)

tools: toolchain $(RTC_T)
	@echo|set /p dummy=" "

$(LUA_A): infotoolchain $(BASE_O)
	@$(LD) /ignore:4217 /DLL $(LDFLAGS) $(BASE_O) $(DLL_LIBS) /OUT:$@ $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-move /Y ..\..\bin\lua55.lib "..\..\lib\lua55.lib" >nul
	@-$(RM) ..\..\bin\lua55.exp >nul 2>&1

$(LUART_T): $(LUA_A)
	@rc /fo .\\resource.res $(DEFINES_RES) /r /v /i "lua/" resources/resource.rc >nul 2>&1
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) $(CFLAGS) luart.c /link $(LDFLAGS) resource.res $(LIBS) /OUT:$@ $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-$(RM) ..\..\bin\luart.exp >nul 2>&1
	@-$(RM) ..\..\bin\luart.pdb >nul 2>&1
	@-$(RM) ..\..\bin\luart.lib >nul 2>&1

..\..\modules\ui\ui.dll:
	@cd ..\modules\ui
	@$(MAKE) $(BUILD) /nologo
	@cd ..\..\core

$(LUAWRT_T): $(LUA_A)
	@rc /fo .\\resource.res $(DEFINES_RES) /r /v /d RTWIN /i "lua/" resources/resource.rc >nul 2>&1
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /c /DRTWIN $(CFLAGS) luart.c /Fowluart.obj $(VERBOSE)
	@$(LD) $(LDFLAGS) resource.res wluart.obj $(LIBS) /OUT:$@ /SUBSYSTEM:windows $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-$(RM) ..\..\bin\wluart.exp >nul 2>&1
	@-$(RM) ..\..\bin\wluart.pdb >nul 2>&1
	@-$(RM) ..\..\bin\wluart.lib >nul 2>&1

$(RTC_T): toolchain static ..\..\tools\rtc\src\rtc.lua ..\..\modules\ui\ui.dll
	@-$(RM) $(DEST)\rtc.exe >nul 2>&1
	@-$(RM) $(DEST)\wrtc.exe >nul 2>&1
	@-$(RM) $(DEST)\rtcheck.exe >nul 2>&1
	@-copy $(DEST)\*.exe ..\..\tools\rtc\src >nul
	@cmd /c echo.
	@echo|set /p dummy="▸  Building rtc compiler          "
	@rc /fo .\\resource.res $(DEFINES_RES) /r /v /d RTC /i "lua/" resources/resource.rc >nul 2>&1
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DRTSTATIC $(CFLAGS) /DRTC luart.c /link /ignore:4217 $(LDFLAGS) resource.res $(BASE_O) $(LIBS_STATIC) /OUT:luart.exe $(VERBOSE)
	@luart.exe ..\..\tools\rtc\src\rtc.lua ..\..\tools\rtc\src\rtc.lua -o rtc.exe ..\..\tools\rtc\src -L..\modules\ $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@rc /fo .\\resource.res $(DEFINES_RES) /r /v /d RTC /d RTWIN /i "lua/" resources/resource.rc >nul 2>&1
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DRTSTATIC /c $(CFLAGS) lua\loadlib.c /Folua\loadlib.obj $(VERBOSE)
	@$(CC) /c /DRTSTATIC /DRTWIN /DRTC $(CFLAGS) luart.c /Fowluart.obj $(VERBOSE)
	@$(CC) /DRTSTATIC /DRTWIN /DRTC $(CFLAGS) luart.c resource.res $(BASE_O) $(LIBS_STATIC) /link /ignore:4217 $(LDFLAGS) /OUT:wluart-static.exe /SUBSYSTEM:windows $(VERBOSE)
	@luart.exe ..\..\tools\rtc\src\rtc.lua ..\..\tools\rtc\src\rtc.lua -o wrtc.exe ..\..\tools\rtc\src -w -s -i ..\..\tools\rtc\src\img\rtc.ico -L..\modules -lui $(VERBOSE)
	@$(CC) /c $(CFLAGS) lua\loadlib.c /Folua\loadlib.obj $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-copy /Y rtc.exe "$(DEST)\rtc.exe" >nul	
	@-copy /Y wrtc.exe "$(DEST)\wrtc.exe" >nul	
	@-$(RM) luart.exe >nul 2>&1
	@-$(RM) wluart.exe >nul 2>&1
	@-$(RM) ..\..\tools\rtc\src\*.exe >nul 2>&1 

buildtools: $(RTC_T)
	@cd ..\..\tools\RTBuilder
	@nmake.bat clean >nul 2>&1
	@nmake.bat $(BUILD) "LUART_PATH=%CD%/../.."
	@cd ..\..\src\core	
	@echo|set /p dummy="▸  Building QuickRT/LuaRT Studio  "
	@echo|set /p dummy=$(PROGRESS)
	@-..\..\bin\rtc -i ..\..\tools\QuickRT\contrib\quickrt.ico ..\..\tools\QuickRT\src\quickrt.lua -o ..\..\tools\QuickRT\QuickRT.exe ..\..\tools\QuickRT\src -lui $(VERBOSE)
	@-signtool.exe sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v "..\..\tools\QuickRT\QuickRT.exe" $(VERBOSE)
	@-copy /Y ..\..\bin\lua55.dll ..\..\tools\QuickRT\lua55.dll >nul	
	@echo|set /p dummy=$(PROGRESS)	
	@$(CC) /DRTCOMPAT /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Folrtapi.obj lrtapi.c $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DRTWIN /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Fosys\File.obj sys\File.c $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)	
	@$(LD) /DLL $(LDFLAGS) $(DLL_LIBS) $(BASE_O) /ignore:4217 /OUT:lua55.dll $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Folrtapi.obj lrtapi.c $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@$(CC) /DLUA_BUILD_AS_DLL $(CFLAGS) /c /Fosys\File.obj sys\File.c $(VERBOSE)
	@$(CC) $(CFLAGS) luart.c /link $(LDFLAGS) lua55.lib ole32.lib comctl32.lib shell32.lib resource.res /OUT:luart.exe  $(VERBOSE)
	@-xcopy "..\..\tools\LuaRT-Studio\$(PLATFORM)\*" "..\..\tools\LuaRT-Studio" /s /e /Y >nul 2>&1
	@-copy /Y luart.exe ..\..\tools\LuaRT-Studio\bin\luart.exe >nul
	@-copy /Y ..\..\bin\wrtc.exe ..\..\tools\LuaRT-Studio\bin\wrtc.exe >nul
	@-copy /Y lua55.dll ..\..\tools\LuaRT-Studio\bin\lua55.dll >nul	
	@echo|set /p dummy=$(PROGRESS)
	-@rd ..\..\tools\LuaRT-Studio\examples /s /q 
	@xcopy ..\..\examples ..\..\tools\LuaRT-Studio\examples\ /s /e /y >nul
	@echo|set /p dummy=$(PROGRESS)
	@xcopy ..\..\modules ..\..\tools\LuaRT-Studio\modules\ /s /e /y >nul
	@echo|set /p dummy=$(PROGRESS)
	@..\..\bin\rtc.exe -s -w -o rtcheck.exe -lnet -lui -i ..\..\setup\img\update.ico ..\..\setup\update.lua ..\..\setup\img $(VERBOSE)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v "rtcheck.exe" $(VERBOSE)
	@-copy /Y rtcheck.exe ..\..\tools\LuaRT-Studio\bin\rtcheck.exe 2>&1 >nul
	@-move /Y rtcheck.exe ..\..\bin 2>&1 >nul
	@echo|set /p dummy=$(PROGRESS)
	@cmd /c echo.

sign: buildtools
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v  ..\..\bin\*.exe $(VERBOSE)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v ..\..\bin\lua55.dll $(VERBOSE)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v ..\..\tools\LuaRT-Studio\bin\*.dll $(VERBOSE)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v ..\..\tools\LuaRT-Studio\bin\*.exe $(VERBOSE)

setup: sign infosetup
	@..\..\bin\luart.exe makedist.lua $(VERSION) $(PLATFORM) $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v LuaRT-$(VERSION)-$(PLATFORM).exe $(VERBOSE)
	@..\..\bin\luart.exe -e "z = require('compression').Zip('LuaRT-$(VERSION)-$(PLATFORM).zip', 'write'); z:write('LuaRT-$(VERSION)-$(PLATFORM).exe'); z:close(); sys.File('LuaRT-$(VERSION)-$(PLATFORM).exe'):remove()" >nul 2>&1
	@-move /Y LuaRT-$(VERSION)-$(PLATFORM).zip ..\..\LuaRT-$(VERSION)-$(PLATFORM).zip >nul 2>&1
	@cmd /c echo.

clean: utf8
	@echo|set /p dummy="▸  Cleaning Lua 5.5 VM... "
	@-$(RM) $(BASE_O) >nul 2>&1
	@echo ✓
	@echo|set /p dummy="▸  Cleaning LuaRT Core framework... "
	@-$(RM) *.tmp >nul 2>&1
	@-$(RM) *.obj >nul 2>&1
	@-$(RM) *.pdb >nul 2>&1
	@-$(RM) *.exp >nul 2>&1
	@-$(RM) *.lib >nul 2>&1
	@-$(RM) *.ilk >nul 2>&1
	@-$(RM) ..\..\bin\*.pdb >nul 2>&1
	@-$(RM) ..\..\bin\*.exp >nul 2>&1
	@-$(RM) ..\..\bin\*.lib >nul 2>&1
	@-$(RM) ..\..\bin\*.ilk >nul 2>&1
	@-$(RM) lua55.* >nul 2>&1
	@-$(RM) resource.res >nul 2>&1
	@-$(RM) ..\..\lib\lua55.lib >nul 2>&1
	@-$(RM) $(ALL_T) >nul 2>&1
	@-$(RM) $(LUART_T) >nul 2>&1
	@-$(RM) $(LUAWRT_T)  >nul 2>&1
	@-$(RM) $(UI_O) >nul 2>&1
	@-$(RM) compression\lib\*.obj >nul 2>&1
	@echo ✓
	@cd ..\..\tools\RTBuilder
	@nmake.bat clean >nul 2>&1
	@cd ..\..\src\core
	@echo ✓	@echo|set /p dummy="▸  Cleaning LuaRT toolchain... "
	@-$(RM) $(DEST)\*.exe >nul 2>&1
	@-$(RM) rtc.exe >nul 2>&1
	@-$(RM) wrtc.exe >nul 2>&1
	@-$(RM) luart*.exe >nul 2>&1
	@-$(RM) wluart*.exe >nul 2>&1
	@-$(RM) quickrt.exe >nul 2>&1
	@-$(RM) ..\..\tools\rtc\src\*.exe >nul 2>&1
	@-$(RM) ..\..\setup\luaRT.zip >nul 2>&1
	@-$(RM) ..\..\tools\QuickRT\QuickRT.exe >nul 2>&1
	@-$(RM) ..\..\tools\QuickRT\lua55.dll >nul 2>&1
	@-$(RM) LuaRT-$(VERSION)-x64.* >nul 2>&1
	@-$(RM) LuaRT-$(VERSION)-x86.* >nul 2>&1
	@-$(RM) ..\..\LuaRT-$(VERSION)-x64.* >nul 2>&1
	@-$(RM) ..\..\LuaRT-$(VERSION)-x86.* >nul 2>&1
	@echo ✓
