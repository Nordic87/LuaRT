--! luart-extensions
-- 
-- LuaRT Webview.printPDF() example : Generate a PDF file using Webview content
--

import ui, webview, sysutils

local win = ui.Window("PDF Generation using Webview", "fixed", 640, 540)

local button = ui.Button(win, "Generate PDF", 0, 0)
button:center()
button.y = 10

local wv = ui.Webview(win, 0, button.height+button.y+10, 640, 540)

function button:onClick()
    local success, result = await wv:printPDF {
        file = "test.pdf",
        orientation = "portrait",
        backgrounds = true
    }
    if not success then
        ui.error(result)
     else
        sysutils.shellexec("open", "test.pdf")
     end
end

function wv:onReady()
    wv:loadstring([[
        <!DOCTYPE html>
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <style>
                body {
                    font-family: 'Arial', sans-serif;
                    background-color: #f0f8ff;
                    margin: 20px;
                    line-height: 1.5;
                    color: #333;
                }

                header {
                    text-align: center;
                    background-color: #0078d4;
                    color: white;
                    padding: 10px;
                    border-radius: 10px;
                    margin-bottom: 10px;
                }

                header h1 {
                    margin: 0;
                }

                section {
                    margin-bottom: 10px;
                    padding: 10px;
                    background: #fff;
                    border-radius: 10px;
                    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
                }

                .highlight {
                    color: #0078d4;
                    font-weight: bold;
                }

                footer {
                    text-align: center;
                    margin-top: 30px;
                    font-size: 0.9em;
                    color: #555;
                }

                svg {
                    display: block;
                    margin: 0 auto;
                }
            </style>
        </head>
        <body>
            <header>
                <h1>Welcome to Webview.printPDF() example !</h1>
                <p>Experience seamless, beautiful PDF generation with Webview widget</p>
            </header>
            <main>
                <section>
                    <h2>About This Page</h2>
                    <p>
                        This colorful HTML page demonstrates how you can easily print web content as a PDF file.
                    </p>
                </section>
                <section>
                    <h2>Why Webview:printPDF() method is cool</h2>
                    <ul>
                        <li>🎨 You can use HTML/CSS to create PDF files.</li>
                        <li>📄 Control the layout of your PDF with various options.</li>
                        <li>🌐 Generate a PDF file from any web page.</li>
                    </ul>
                </section>
                <section>
                    <h2>Sample Inline Graphic</h2>
                    <svg xmlns="http://www.w3.org/2000/svg" width="150" height="150" viewBox="0 0 100 100">
                        <circle cx="50" cy="50" r="45" fill="#014080" />                    
                        <circle cx="70" cy="30" r="32" fill="#FFFFFF" />                    
                        <circle cx="70" cy="30" r="25" fill="#EFB42C" />                    
                    </svg>
                </section>
            </main>
            <footer>
                <p>Thank you for using Lua<sup style="color: #EFB42C">rt</sup> 🎉</p>
            </footer>
        </body>
        </html>
    ]])
    wv.zoom = 0.6
end

win:center()

await win:showasync()

