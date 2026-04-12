--! luart-extensions
--
-- LuaRT C FFI example to access a LuaRT widget's internal handle to use Windows API with
-- Thanks to MediaRot for the pixelart.png file
--

import ui, c

-- Defines ShowWindow() function from user32.dll
local user32 = c.Library("user32.dll")
user32.ShowWindow = "(pB)B"

-- Defines LuaRT Widget C struct (only the first fields to get widget's handle)
local Widget = c.Struct("ip", "type", "hwnd")

-- Creates a Window with a centered Button
local win = ui.Window("Widget and Windows API example", 340, 220)
local btn = ui.Button(win, "Click me !")
btn:center()

function btn:onClick()
    -- widget contains a C Widget Struct using the btn widget as content
	local widget = Widget(self)
    -- uses the internal btn handle to call ShowWindow() to hide it
	-- user32.ShowWindow(widget.hwnd, false)
	btn:hide()
	-- sleep for 2sec
	sleep(2000)
    -- uses the internal btn handle to call ShowWindow() to show it again
	-- user32.ShowWindow(widget.hwnd, true)
	btn:show()
end

await win:showasync()
