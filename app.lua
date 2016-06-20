local module = {}
local m = nil

local function rfid_recd(data)
  gpio.write(3, gpio.HIGH)
  m:publish(config.ENDPOINT .. config.ID .. '/received', data, 0, 1)
  gpio.write(3, gpio.LOW)
end

local function mqtt_received(conn, topic, data)
  print(topic .. ': ' .. data)
  if data == 'rfid_enable' then
    gpio.write(3, gpio.LOW)
    --uart.setup(0, 2400, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    uart.alt(1) -- use GPIO13 as RX
    uart.on('data', "\r", rfid_recd, 0)
  else if data == 'rfid_disable' then
    gpio.write(3, gpio.HIGH)
    --uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    uart.alt(0) -- use GPIO3 / USB as RX
    uart.on('data')
  end
end

end

-- send a ping to the broker
local function send_ping()
  m:publish(config.ENDPOINT .. 'ping', 'id=' .. config.ID, 0, 0)
end

-- send my id to the broker for registration
local function register_myself()
  m:subscribe(config.ENDPOINT .. config.ID, 0, function(conn)
    print("Registered to data endpoint")
  end)
end

local function mqtt_start()
  m = mqtt.Client(config.ID, 120)
  m:on('message', mqtt_received)
  print("Connecting to MQTT broker at " .. config.HOST)
  m:connect(config.HOST, config.PORT, 0, 1, function(conn)
    register_myself()
    tmr.stop(6)
    tmr.alarm(6, 5000, 1, send_ping)
  end)
end

function module.start()
  mqtt_start()

  -- rfid control
  gpio.mode(3, gpio.OUTPUT, gpio.FLOAT)
end

return module
