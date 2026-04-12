--! luart-extensions
--
-- Blog application using Tailwind CSS - Powered by LuaRT
--

import ui, webview

sys.currentdir = sys.File(arg[0]).path

local win = ui.Window("Blog application using Tailwind CSS - Powered by LuaRT", "fixed", 414, 776)
local wv = ui.Webview(win, { url = "file:///Blog.html" })
wv.align = "all"

function wv:onReady()
    wv.statusbar = false
    wv.devtools = false
    wv.contextmenu = false
end

await win:showasync()