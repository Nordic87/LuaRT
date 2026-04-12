--! luart-extensions
-- Echo server example
-- Using non blocking sockets and asynchronous tasks
--
-- First run this program in a console : luart.exe echoserver.lua
-- Then start the GUI chat client : wluart.exe echoclient.lua


import net

class Server {
    
    -- Server constructor (socket initialization)
    function constructor()
        self.socket = net.Socket("localhost", 5000)
        self.socket.blocking = false
        self.connected = false
        self.clients = {}
    end

    -- Make the server ready to receive incoming connections
    function connect()
        if self.socket:bind() then
            print("Chat server is running\nWaiting for new connections...")
            self.connected = true
        else
            error("Could not create server connection : ${net.error}")
        end
    end
    
    function sendall(msg)
        print(msg)
        for client in each(self.clients) do
            client:send(msg)
        end
    end
    
    -- Task executed each time a new client connects
    async function echo(client)
        print("New client connected from ${client.ip}")
        local id = #self.clients+1
        self.clients[id] = client
    
        -- Asynchronous welcome message sending
        await(client:send("\n----------------------------\nWelcome to the chat server !\n----------------------------"))
    
        -- Wait forever for new messages from the client
        while true do
            -- wait for a message
            print("Waiting for messages from ${client.ip}...")
            local msg = await client:recv()
            print("Received message from ${client.ip}: ${msg or 'nil'}")
            -- An error occured or the client has disconnected
            if not msg then
                self.clients[id] = nil
                self:sendall("Connection with ${client.ip} has been lost")
                return
            else
                -- Send the received message to all connected clients
                self:sendall("[${client.ip}] ${msg}")
            end
        end
    end
    
    -- Server main loop
    async function start()
        self:connect()
        while self.connected do
            -- Wait for a new incoming connection..
            local client = await(self.socket:accept())
            -- ...then call the echo() Task to manage this new client
            self:echo(client)
        end
    end
}

await Server():start()