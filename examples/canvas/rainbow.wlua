--! luart-extensions
import ui, canvas

-- create a simple Window
local win = ui.Window("Canvas Rainbow sample", "fixed", 500, 400)

-- create a Canvas
local canvas = ui.Canvas(win)
canvas.align = "all"

-- create a linear gradient
local gradient = canvas:LinearGradient { [0] = 0xE30940FF, [0.25] = 0xE7D702FF, [0.75] = 0x0FA895FF, [1] = 0x1373E8FF }
gradient.start = { 0, 0 }
gradient.stop = { canvas.width, 0 }

--Get max speed for scheduler idle / set it to 0 to get the best performance for animation
sys.idleThreshold = 0

local pos = 0
local dir = 4

function canvas:onPaint()
  self:begin()
  gradient.start = {pos, 0}
  self:fillrect(0, 0, canvas.width, canvas.height, gradient)
  self:flip()
  if pos > canvas.width-100 then
    dir = -4
  elseif pos < -canvas.width then
    dir = 4
  end
  pos = pos + dir
end

await win:showasync()