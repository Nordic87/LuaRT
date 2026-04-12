--! luart-extensions
--
-- LuaRT Window.startmoving() example
--

import ui

local win = ui.Window("Window.startmoving() example", "raw", 320, 200)
local label = ui.Label(win, "Click and drag to move the Window !")
label.align = "all"
label.textalign = "center"
win.bgcolor = 0xfbd422
win:center()

function label:onClick()
    win:startmoving()
end

await win:showasync()