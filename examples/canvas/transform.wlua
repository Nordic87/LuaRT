--! luart-extensions

import ui, canvas

local win = ui.Window("Canvas - Use mousewheel to zoom in/out", 400, 360)

local c = ui.Canvas(win)
c.align = "all"
c.color = 0xffcc00ff

local degree = 0
local scale = 1

local img = c:Image(sys.File(arg[0]).path.."LuaRT.png")

function win:onKey(key)
	if key == "VK_RETURN" then
		win.fullscreen = not win.fullscreen
	end
end

function c:onMouseWheel(delta)
	scale = scale + 0.05*delta/120
end

function c:onPaint()
	c:begin()
	c:clear()
	c:identity()
	c:rotate(degree)
	c:scale(scale, scale)
	img:draw((c.width-img.width)/2, (c.height-img.height)/2, 1, 1, "linear")
	degree = degree + 0.5
	c:flip()
end

await win:showasync()

