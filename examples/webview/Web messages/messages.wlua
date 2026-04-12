--! luart-extensions
--
-- LuaRT Webview Web Messages example
--

import ui, webview
sys.currentdir = sys.File(arg[0]).path

local win = ui.Window("Webview Web Messages test", "fixed", 640, 540)

local button = ui.Button(win, "Change color !", 0, 0)
button:center()
button.y = 10

local wv = ui.Webview(win, { url = "file:///messages.html" }, 0, button.height+button.y+20, 640, 540)

function button:onClick()
    wv:postmessage('{ "SetColor" : "blue" }', true)
    win.title = wv.title
end

function wv:onMessage(str)
    local action = str:match('"(%w+)')
    if action == "SetTitleText" then
        win.title = str:match('"SetTitleText (.*)"')
    elseif action == 'GetWindowBounds' then
        wv:postmessage('{ "WindowBounds" : "'..wv.width..'x'..wv.height..'" }', true)
    end
end

function wv:onLoaded(success, status) 
    win.title = wv.title
end

function wv:onReady()
    wv.zoom = 0.8
end

win:center()

await win:showasync()

