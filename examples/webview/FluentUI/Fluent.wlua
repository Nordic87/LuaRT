--! luart-extensions
--
-- FluentUI application with LuaRT
-- Displays some system information using FluentUI design in a Webview
--

import ui, webview

sys.currentdir = sys.File(arg[0]).path

local win = ui.Window("FluentUI application with LuaRT", "fixed", 400, 150)
win:loadicon(sys.env.WINDIR.."/System32/shell32.dll", 278)
win.bgcolor = 0xFFFFFF

local wv = ui.Webview(win, 0, 46)
wv.align = "all"

local ram = ".."

sys.Pipe("wmic computersystem get TotalPhysicalMemory"):read(2000).after = function(result)
    ram = result and math.floor(result:match("(%d+)")/1000000000) or "N/A"
    wv:postmessage('{ "id": "memorysize", "text": "'..ram.." Gb"..'"}', true)
end

function wv:onLoaded()
    wv:postmessage('{ "id": "memorysize", "text": "'..ram.." Gb"..'"}', true)
    wv:postmessage('{ "id": "cpuname", "text": "${sys.registry.read("HKEY_LOCAL_MACHINE", "Hardware\\Description\\System\\CentralProcessor\\0", "ProcessorNameString")}" }', true)
    wv:postmessage(' {"show": true}')
    wv:postmessage('{ "id": "graphic", "text": "${sys.registry.read("HKEY_LOCAL_MACHINE", "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\WinSAT", "PrimaryAdapterString") or "Not available"}" }', true)
    win:show()
    wv:show()
end

function wv:onReady()
    wv.statusbar = false
    wv.devtools = false
    wv.contextmenu = false
    wv.acceleratorkeys = false
    -- Add initialization JS script before any loaded page
    -- This JS script processes posted messages from Lua script
    wv:addinitscript([[
    window.chrome.webview.addEventListener('message', arg => {
        if ("id" in arg.data) {
            const item = document.getElementById(arg.data.id);
            item.innerHTML = arg.data.text;
        }
        if ("show" in arg.data) {
            document.body.style.visibility = "visible";
        }
    }); ]])
    wv.url = "file:///Fluent.html"
end

win:center()

await ui.task