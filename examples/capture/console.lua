--! luart-extensions
--
-- LuaRT capture module example with Camera object in console app
--

import capture

-- 1. List available devices
print("--- Video Devices ---")
for i, dev in ipairs(capture.devices.video) do
    print(i, dev)
end

print("\n--- Audio Devices ---")
for i, dev in ipairs(capture.devices.audio) do
    print(i, dev)
end

-- 2. Initialize the Camera
-- You can pass specific device names in a table: {video="Name", audio="Name"}
-- Or leave empty for defaults.
print("\nInitializing camera...")
local cam = capture.Camera()

-- 3. Take a Snapshot
local file = "console_snapshot.jpg"
if cam:snapshot(file) then
    print("Snapshot saved to: " .. file)
else
    print("Failed to take snapshot.")
end

-- 4. Recording Example
print("Recording 5 seconds of video...")
cam.record.start({
    file = "test_video.mp4",
    format = "HD720p",   -- Options: "Auto", "VGA", "HD720p", "HD1080p", "UHD2k", "UHD4k"
    audio = "medium",    -- Bitrate: "low", "medium", "high", "ultra"
    video = "low"        -- Bitrate: "low", "medium", "high", "ultra"
})

sleep(5000)  -- Record for 5 seconds
cam.record.stop()
print("Recording finished.")