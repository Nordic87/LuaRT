--! luart-extensions
import ui

ui.theme = "dark"

-- create a simple Window
local win = ui.Window(" File list sample", 320, 300)

-- Define a FileList widget
local FileList = Object(ui.List)

function FileList:constructor(parent, path, ...)
  local list = ui.List.constructor(self, parent, {}, ...)
  list.style = "icons"
  list.border = false
  local dir = {}
  local files = {}
  local directory
  
  function list:get_directory()
    return directory and directory.fullpath or path
  end
  
  function list:set_directory(folder)
    directory = sys.Directory(folder)
    if directory.parent ~= nil then
      dir = { {name = "..", fullpath = directory.parent.fullpath} }
    else
      dir = {}
    end
    files = {}
    for entry in directory:list("*.*") do
      local item = {name = entry.name, fullpath = entry.fullpath}
      if type(entry) == "Directory" then
        dir[#dir+1] = item
      else
        files[#files+1] = item
      end
    end
    list:clear()
    self:append(dir)
    self:append(files)
  end
  
  function list:append(dir)
    local i = 1
    for entry in each(dir) do
      i = i + 1
      if i == 32 then -- sleep() only each 32 entry to speedup
        i = 1
      end
      local item = self:add(entry.name)
      item:loadicon(entry.fullpath)
    end
  end
  
  function list:onDoubleClick(item)
    if item.index <= #dir then
      self.directory = dir[item.index].fullpath
    end
  end
  
  list.directory = path or sys.currentdir
  return list
end

-- Now use it !
local fl = FileList(win)
fl.align = "all"

await win:showasync()