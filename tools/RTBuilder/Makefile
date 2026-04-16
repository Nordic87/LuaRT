# | RTBuilder - a GUI RAD tool for LuaRT
# | Luart.org, Copyright (c) Tine Samir 2023.
# | See Copyright Notice in LICENSE.TXT
# |--------------------------------------------------------------
# | Makefile
# | Please set LUART_PATH to your LuaRT folder if autodetection fails
# |--------------------------------------------------------------
# | Usage (default release build)			 : make
# | Usage (create release package)		 	 : make package
# | Usage (clean all)	 				 	 : make clean
# |-------------------------------------------------------------

TARGET=		RTBuilder
VERSION=	0.5
SRC= 		src\main.wlua src\version.lua src\dump.lua src\widgets.wlua  
LIBS=		-L../../modules -L. -ltracker -ljson -lui
VERBOSE=	2>&1 >nul

ICON = resources\RTBuilder.ico

RM= del /Q
CP= copy /Y
LC= "$(LUART_PATH)\bin\rtc.exe"

all: $(TARGET).exe

infopackage:
	@chcp 65001 >nul 2>&1
	@cmd /c echo.
	@echo|set /p dummy="▸  Building release package       "

package: $(TARGET).exe infopackage
	"@$(LUART_PATH)"\bin\luart.exe contrib\package.lua
	@cmd /c echo.

setup: $(TARGET).exe
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v tracker.dll $(VERBOSE)
	@-signtool sign /sha1 "4b47f71722cd511bc491e11f5a4130cb779f98f6" /tr http://time.certum.pl /td sha256  /fd sha256 /v RTBuilder.exe $(VERBOSE)

infomodule:  
	@chcp 65001 >nul 2>&1
	@cmd /c echo.
	@echo|set /p dummy="▸  Building $(TARGET).exe $(VERSION)	  "

tracker.dll:
	@cd tracker
	@nmake.bat
	@-$(CP) /Y tracker.dll ..\tracker.dll 2>&1 >nul
	@cd ..

src\version.lua:
	@echo return 'v$(VERSION)' > src\version.lua

$(TARGET).exe: tracker.dll infomodule $(SRC)
	@$(LC) $(SRC) -o $@ -i $(ICON) $(LIBS) $(VERBOSE)
	@echo|set /p dummy=$(PROGRESS)
	@-$(CP) /Y "$(LUART_PATH)"\bin\lua55.dll 2>&1 >nul
	@-$(CP) /Y "$(LUART_PATH)"\modules\webview\webview.dll widgets\webview.dll 2>&1 >nul
	@-$(CP) /Y "$(LUART_PATH)"\modules\canvas\canvas.dll widgets\canvas.dll 2>&1 >nul
	@-$(CP) /Y "$(LUART_PATH)"\modules\capture\capture.dll widgets\capture.dll 2>&1 >nul
	@echo|set /p dummy=$(PROGRESS)
	@cmd /c echo.

clean:
	@echo|set /p dummy="▸  Cleaning $(TARGET)... "
	@-$(RM) $(TARGET).exe >nul 2>&1 
	@-$(RM) widgets/webview.dll >nul 2>&1 
	@-$(RM) widgets/canvas.dll >nul 2>&1 
	@-$(RM) lua55.dll >nul 2>&1 
	@cd tracker
	@nmake.bat clean >nul 2>&1
	@cd ..
	@echo ✓