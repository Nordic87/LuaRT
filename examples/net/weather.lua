--! luart-extensions
-- wttr.in Http web service, by Igor Chubin
-- http://www.apache.org/licenses/LICENSE-2.0
-- Execute in Windows Terminal, rather than cmd.exe, for full UTF8 support

import console
import net, json

local url = "http://ip-api.com"
local client = net.Http(url)

-- make a GET request, and after its terminated, call the provided function, to process the response
client:get("/json/").after =  function (self, response)
                                 if not response then
                                    error(net.error)
                                 end
                                 local result = json.decode(response.content)
                                 net.Http("https://wttr.in"):get("/${result.city}?format=3").after = function (client, response)
									console.write(response.content:capitalize())
								end
                              end
waitall()