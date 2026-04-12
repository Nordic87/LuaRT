--! luart-extensions
-- 
-- LuaRT Webview Chrome DevTools Protocol : sets a Runtime.consoleAPICalled event handler
--

import ui, webview
import json

local win = ui.Window("Chrome Devtools Protocol events example", "fixed", 320, 240)
local wv = ui.Webview(win, 0, 0, 320, 200)
local button = ui.Button(win, "Add console.log entry")
button:center()
button.y = 206
win:center()

function button:onClick()
    -- defines a CDP event handler for the Runtime.consoleAPICalled event
    function wv.cdp.Runtime.consoleAPICalled(data)
        -- display the console message in the Webview
        wv:eval('document.getElementById("msg").textContent = "'..json.decode(data).args[1].value..'"')
    end

    -- write a message to the console
    wv:eval("console.log('This is a console log message')")
end

function wv:onReady()
    wv.statusbar = false
    wv:loadstring([[
<!DOCTYPE html>
<html>
<body>
   <div style="display: flex; justify-content: center; align-items: center; height: 90vh;">
      <h3 id="msg" style="margin-left: -10px;color: #014080; font-family: Arial Narrow"></h3>
   </div>
</body>
</html>]])
    -- Enable the CDP Runtime domain
    wv.cdp.Runtime.enable()
end

await win:showasync()
