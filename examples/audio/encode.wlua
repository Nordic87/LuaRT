--! luart-extensions

import ui, audio, sysutils

local win = ui.Window("Audio recording sample", 333, 111, "single")

local recBtn = ui.Button(win, "Start recording")
recBtn:center()
recBtn.y = recBtn.y - 24

local stopBtn = ui.Button(win, "Stop recording")
stopBtn:center()
stopBtn.y = stopBtn.y+6
stopBtn.enabled = false

local list = ui.Combobox(win, false, {"wav", "mp3"}, recBtn.x + recBtn.width+18, recBtn.y + recBtn.height//2)
list.selected = list.items["mp3"]
list.width = 60


function recBtn:onClick()
    if not audio.record.start("sound."..list.text, list.text) then
        error(sys.error)
    end
    self.enabled = false
    list.enabled = false
    stopBtn.enabled = true
end

function stopBtn:onClick()
    audio.record.stop()
    self.enabled = false
    recBtn.enabled = true
    list.enabled = true
    sysutils.shellexec("open",sys.currentdir.."\\sound."..list.text)
end

await win:showasync()