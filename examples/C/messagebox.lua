--! luart-extensions
--
-- LuaRT C FFI example calling Windows API MessageBoxW() function
--

import c

-- load the user32 library
local user32 = c.Library("user32.dll")

-- prototype of the Windows API function MessageBoxW()
user32.MessageBoxW = "(pWWS)B"

-- Execute MessageBox() with seamless Lua string to C wide string conversion
user32.MessageBoxW(c.NULL, "Hello World !", "LuaRT C FFI module", 1)
