--! luart-extensions
--
-- LuaRT Canvas example : Image.drawsub()
--

import ui, canvas

-- create a simple Window
local win = ui.Window("Image:drawsub sample", "fixed", 500, 400)

-- create a Canvas
local canvas = ui.Canvas(win)
canvas.align = "all"
canvas.bgcolor = 0x000000FF

local w = canvas.width
local h = canvas.height
local img = canvas:Image(sys.File(arg[0]).path.."LuaRT.png")
win:center()

-- Number of subimages (change to 320 and see !)
local n = 16

-- Create the subimages coordinates in a table
local subimages = {}
local step = img.width/n 

for i=0, img.width, step do
    local n = #subimages+1
    subimages[n] = { x = i, y = 0, width = step, height = img.height, dy = 0}
end

function canvas:onPaint()
  self:begin()
  self:clear()
  
  for subimage in each(subimages) do
    img:drawsub(subimage.x + 10, subimage.y + subimage.dy, subimage.x, subimage.y, subimage.width, subimage.height)
    subimage.dy = subimage.dy + math.random(0, 3)
  end
  
  self:flip()
  
  if subimages[1].dy > 400 then
    win:hide()
  end
  
end

await win:showasync()