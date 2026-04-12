--! luart-extensions
--
-- LuaRT asynchronous zip extraction example
--

import net, compression

-- Download LuaRT 1.8.0 x64 from GitHub
print("Downloading LuaRT 1.8.0...")
local client, response = await net.Http("https://github.com"):download("/samyeyo/LuaRT/releases/download/v1.8.0/LuaRT-1.8.0-x64.zip")
if not response or response.status ~= 200 then
    error(net.error)
end

print("Downloaded "..response.file.name..", "..response.file.size.." bytes")

-- Extract installer from the downloaded ZIP file
local zip = compression.Zip(response.file.name)
local task = zip.async.extract("LuaRT-1.8.0-x64.exe") or error("Zip archive error: "..zip.error)

-- Set task.after property to check if archive extraction succeeded
task.after = function(result)
    if result == false then
        error(sys.error)
    else
        print("\nSuccessfully extracted "..result.name)
    end
end

-- Launch a new Task during archive extraction (that prints several "." characters)
local async function extract()
    io.write("Extracting "..response.file.name)
    while not task.terminated do
        io.write(".")
        sleep()
    end
end

extract()

waitall()
