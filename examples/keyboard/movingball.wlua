--! luart-extensions
-- 
-- LuaRT moving ball example  : Move a red ball using arrow keys
--

import ui, keyboard, canvas

local win = ui.Window("keyboard example - Move the ball using arrow keys", "fixed", 410, 464)

local c = ui.Canvas(win)
c.align = "all"

-- load the background Image
local bg = c:Image(sys.File(arg[0]).path.."back.png")

-- set initial ball position to the center
x = c.width/2
y = c.height/2

-- start the keyboard handler Task
async function kbhandler()
    while win.visible do
        if keyboard.isdown("LEFT") and x > 11 then
            x = x - 6
        end
        if keyboard.isdown("RIGHT") and x < c.width-11 then
            x = x + 6
        end
        if keyboard.isdown("UP") and y > 10 then
            y = y - 5
        end
        if keyboard.isdown("DOWN") and y < c.height-10 then
            y = y + 5
        end
        sleep()
    end
end

-- Canvas onPaint() event
function c:onPaint()
    self:begin()
    -- draw the image on the whole Window
    bg:drawrect(0, 0, 410, 464)
    -- draw the ball shadow
    self:fillcircle(x+3, y+2, 7, 0x00000080)
    -- draw the red ball
    self:fillcircle(x, y, 7, 0xFF0000FF)
    self:flip()
end

win:showasync()

--launch the kbhandler Task
kbhandler()

-- wait for all tasks to finish
waitall()
