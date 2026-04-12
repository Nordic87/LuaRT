--! luart-extensions
--
--  LuaRT tasks.wlua example
--  Concurrent Tasks that advance Progressbars
--

import ui

local win = ui.Window("Concurrent Tasks example", 320, 200)
local y = 30

async function launchTask(i, pb)
    while pb.position < 100 do
        sleep(75)
        pb:advance(1)
    end
end

-- Add 5 Progressbars
for i=1,5 do
    ui.Label(win, "Task #"..i, 12, y)
    local pb = ui.Progressbar(win, 60, y, 230)
    y = y + 30
    
    -- throw a task that increase the Progressbar position
    -- uses a different priority for each task (Task #1 has lowest priority, Task #2 has higher priority)
    launchTask(i, pb).priority = i
end

await win:showasync()