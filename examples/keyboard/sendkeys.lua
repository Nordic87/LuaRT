--! luart-extensions
-- 
-- LuaRT keyboard.sendkeys() example
--

import keyboard

-- Send virtual keys to launch the Windows Run... command then type "notepad", press Enter, and enters slowly the letters Hello LuaRT !
keyboard.sendkeys("{DELAY=100}@rnotepad{ENTER}{DELAY=600}H{DELAY=100}ello LuaRT !")