local ui = require "ui"
require "capture"


local Camera = Object(ui.Camera)
Camera.inspector = {}

function Camera.inspector.aspect(panel, obj, property)
    local widget = ui.Combobox(panel, false,  {"stretch", "4x3", "16x9"}, 100, panel.pos-3)
    widget.onSelect = function (self, item)
        tracker.widget.aspect = item.text
    end
    widget.update = function(tracked)
        widget.selected = widget.items[tracked.aspect]
    end
    return widget
end

return inspector:register(Camera, { border = inspector.properties.boolean }, { sync = true })