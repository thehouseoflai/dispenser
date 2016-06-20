local module = {}

local function wifi_wait_ip()
  if (wifi.sta.getip() == nil) then
    print("IP unavailable, waiting...")
  else
    tmr.stop(1)
    print("ESP8266 mode: " .. wifi.getmode())
    print("MAC address:  " .. wifi.ap.getmac())
    print("IP address:   " .. wifi.sta.getip())
    module.server = net.createServer(net.TCP,180) 
-- remote control
    module.server:listen(2323,
      function(c) 
        local function s_output(str) 
          if(c~=nil) 
            then c:send(str) 
          end 
        end 
        node.output(s_output, 1)   
        -- re-direct output to function s_ouput.
        c:on("receive",function(c,l) 
          node.input(l)           
          --like pcall(loadstring(l)), support multiple separate lines
        end) 
        c:on("disconnection",function(c) 
          node.output(nil)        
          --unregist redirect output function, output goes to serial
        end) 
        print("Welcome to NodeMCU.")
      end)
    app.start()
  end
end

local function wifi_start()
  -- bullshit version that doesn't actually check any
  -- past the first one
  if config.SSID then
    for k,v in pairs(config.SSID) do
      print("Connecting to " .. k .. " with " .. v .. "...")
      wifi.sta.config(k,v)
      wifi.sta.connect()
      tmr.alarm(1, 2500, tmr.ALARM_AUTO, wifi_wait_ip)
    end
  else
    print("config.SSID not set")
  end
end

function module.start()
  print "Configuring Wifi..."
  wifi.setmode(wifi.STATION)
  wifi.sta.getap(wifi_start)
end

return module
