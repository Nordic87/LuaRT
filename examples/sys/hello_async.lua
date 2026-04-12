--! luart-extensions
--
-- LuaRT Hello World example using asynchronous tasks
--


-- print a message after a delay (in seconds)
async function print_after(delay, message)
    sleep(delay*1000)
    print(message)
end

-- print "Hello" after 1 second
print_after(1, "Hello")

-- print "Hello" after 2 seconds
print_after(2, "World")

-- wait for all tasks to finish before exit
waitall()