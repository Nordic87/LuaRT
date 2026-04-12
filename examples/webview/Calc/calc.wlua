--! luart-extensions
import ui, webview

sys.currentdir = sys.File(arg[0]).path

local win = ui.Window("Calculator - web application with LuaRT", "fixed", 290, 310)
win:loadicon(sys.env.WINDIR.."/System32/calc.exe")

local wv = ui.Webview(win, { url = "file:///calc.html"})
wv.align = "all"

win:center()

function wv:onReady()
    wv.statusbar = false
    wv.devtools = false
    wv.contextmenu = false
    wv.acceleratorkeys = false
end

await win:showasync()