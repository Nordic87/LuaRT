--! luart-extensions
-- 
-- LuaRT Webview Chrome DevTools Protocol : Capture the Webview content as a PNG file
--

import ui, webview
import sysutils, json

local win = ui.Window("Chrome Devtools Protocol example", "fixed", 320, 240)
local wv = ui.Webview(win, 0, 0, 320, 200)
local button = ui.Button(win, "Capture as PNG")
button:center()
button.y = 206
win:center()

function button:onClick()
   local success, result = await(wv.cdp.Page.captureScreenshot '{}')
   if not success then
      ui.error(result)
   else
      local buff = sys.Buffer(json.decode(result).data, "base64")
      local png = sys.File("capture.png"):open("write", "binary")
      png:write(buff:encode("utf8"))
      png:close()
      sysutils.shellexec("open", png)
   end
end

function wv:onReady()
    wv.statusbar = false
    wv:loadstring([[
<!DOCTYPE html>
<html>
<body>
   <div style="display: flex; justify-content: center; align-items: center; height: 90vh;">
      <svg xmlns="http://www.w3.org/2000/svg" width="100" height="100" viewBox="0 0 100 100">
         <circle cx="40" cy="50" r="35" fill="#014080" />                    
         <circle cx="50" cy="35" r="22" fill="#FFFFFF" />                    
         <circle cx="50" cy="35" r="18" fill="#EFB42C" />                    
      </svg>
      <h1 style="margin-left: -10px;color: #014080; font-family: Arial Narrow">Lua<sup style = "color: #EFB42C">rt</sup></h1>
   </div>
</body>
</html>]])
end

await win:showasync()
