local module = {}
module.SSID = {}
--module.SSID['L'] = '5102379569'
module.SSID['AMT'] = 'huskybarleytwist'

-- broker address
--module.HOST = '192.168.1.82'
module.HOST = '192.168.162.177'
module.PORT = 1883

-- who am I
module.ID = node.chipid()
module.ENDPOINT = 'dispenser/'


return module
