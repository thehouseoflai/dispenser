local module = {}
module.SSID = {}
module.SSID['L'] = '5102379569'

-- broker address
module.HOST = '192.168.1.82'
module.PORT = 1883

-- who am I
module.ID = node.chipid()
module.ENDPOINT = 'dispenser/'..node.chipid()


return module
