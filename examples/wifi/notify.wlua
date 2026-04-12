--! luart-extensions
--
--  LuaRT notify.wlua example
--  Notifies user when a Wi-Fi connection is established/lost
-- (use desktop interpreter wluart.exe to see the notifications)
--

import ui
import wifi

-- Create a dummy hidden window
local win = ui.Window("Wi-Fi agent")

function wifi.onConnected()
    local msg = string.format("SSID: %s\nQuality: %d%%\nChannel: %d", wifi.name, wifi.quality, wifi.channel)
    win:notify("Wi-Fi connected", msg, "info")
end

function wifi.onDisconnected()
    win:notify("Wi-Fi disconnected", " ", "error")
end

-- Forces the ui Task to terminate in 60sec
ui.task.timeout = 60000

await ui.task