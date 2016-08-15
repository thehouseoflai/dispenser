local module = {}
local m = nil

local servo_speed = 1400 -- pulse width in us

local function rfid_enable()
  gpio.write(3, gpio.LOW)
end

local function rfid_disable()
  gpio.write(3, gpio.HIGH)
end

local function rfid_recd(data)
  rfid_disable()
  data=data:match "^%s*(.-)%s*$" -- trim whitespace
  m:publish(config.ENDPOINT .. config.ID .. '/received', data, 0, 0)
  -- delay here to avoid BREAKING THE UNIVERSE
  tmr.stop(5)
end

local function servo_run()
  gpio.serout(4,gpio.HIGH,{servo_speed,20000-servo_speed},50)
  gpio.write(4,gpio.HIGH)
end

local function mqtt_received(conn, topic, data)
  print(topic .. ': ' .. data .. "\n")
  if data == 'rfid_enable' then
    rfid_enable()
    uart.setup(0, 2400, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    uart.alt(1) -- use GPIO13 as RX
    uart.on('data', "\r", rfid_recd, 0)
  end
  if data == 'rfid_disable' then
    rfid_disable()
    uart.setup(0, 9600, 8, uart.PARITY_NONE, uart.STOPBITS_1)
    uart.alt(0) -- use GPIO3 / USB as RX
    uart.on('data')
  end
  if data == 'servo_run' then -- output a pulse
    servo_run()
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
  rfid_enable()
  -- servo control
  gpio.mode(4,gpio.OUTPUT,gpio.FLOAT)
  gpio.write(4,gpio.HIGH)
end

return module
