--! luart-extensions
--
-- LuaRT capture module example with Camera widget in GUI app
--

import ui, capture

-- Create the main window
local win = ui.Window("LuaRT Camera Preview", "fixed", 640, 480)

-- Create the Camera widget
local cam = ui.Camera(win, 0, 0, 640, 400)
cam.align = "top" 

-- Create Controls (Enabled immediately since camera is ready)
local btnSnap = ui.Button(win, "Snapshot", 10, 410)
local btnRecord = ui.Button(win, "Start Recording", 120, 410)
local lblStatus = ui.Label(win, "Camera Ready", 250, 415)

-- Event: Take Snapshot
function btnSnap:onClick()
    local filename = "gui_snapshot.png"
    if cam:snapshot(filename) then
        lblStatus.text = "Saved: " .. filename
    else
        lblStatus.text = "Snapshot failed"
    end
end

-- Event: Toggle Recording
function btnRecord:onClick()
    if cam.recording then
        cam.record.stop()
        self.text = "Start Recording"
        lblStatus.text = "Recording stopped"
        btnSnap.enabled = true 
    else
        cam.record.start({
            file = "recorded_video.mp4",
            format = "HD720p",
            audio = "medium",
            video = "low"
        })
        self.text = "Stop Recording"
        lblStatus.text = "Recording..."
        btnSnap.enabled = false 
    end
end

await win:showasync()