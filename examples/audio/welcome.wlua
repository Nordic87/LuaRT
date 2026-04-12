--! luart-extensions
import ui, audio

local win = ui.Window("Audio module example", 320, 200)
local button = ui.Button(win, "Play Windows logon sound")
button:center()

function button:onClick()
    audio.play(sys.env.WINDIR.."\\Media\\Windows Logon.wav")
end

await win:showasync()