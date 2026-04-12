--! luart-extensions
--
-- LuaRT YouTube webview example
--

import ui, webview

local win = ui.Window("YouTube - Powered by LuaRT", 640, 540)
local wv = ui.Webview(win, { url = "https://www.youtube.com/?app=mobile" })

wv.align = "all"

function wv:onReady()
    wv.statusbar = false
    wv.devtools = false
    wv.contextmenu = false
    wv.acceleratorkeys = false
end

function wv:onFullScreenChange(isfullscreen)
    win.fullscreen = isfullscreen
end

function wv:onThemeChange(isDark)
    self:reload()
end

win:center()

await win:showasync()