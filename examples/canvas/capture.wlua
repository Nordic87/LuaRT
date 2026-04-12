--! luart-extensions
import ui, canvas

local win = ui.Window("Canvas - Image capture example", "fixed", 480, 360)

local c = ui.Canvas(win, 0, 0, 0, 300)
c.align = "top"

local btn = ui.Button(win, "Capture !")
btn:center()
btn.y = 308
btn.cursor = "hand"

local scale = 0.5
local delta = 0.03
local img = c:Image(sys.File(arg[0]).path.."LuaRT.png")

function btn:onClick()
	c:capture("test.jpg")
	ui.info ("Image captured as test.jpg")
end

-- Scale the Canvas content and draw the image
function c:onPaint()
	self:begin()
	self:clear()
	img:draw((c.width-img.width*scale)/2, (c.height-img.height*scale)/2, scale, 1)
	scale = scale + delta
	if scale < 0.5 or scale > 2 then
		delta = -delta
	end
	self:flip()
end

await win:showasync()

