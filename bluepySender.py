# Prerequisites: sudo apt-get install libglib2.0-dev; sudo pip install bluepy
# References: https://elinux.org/RPi_Bluetooth_LE, https://ianharvey.github.io/bluepy-doc/
from bluepy import btle

print "Connecting..."
dev = btle.Peripheral("62:00:A1:21:6E:67")

passthroughUuid = btle.UUID("0000ffe0-0000-1000-8000-00805f9b34fb")
passthroughService = dev.getServiceByUUID(passthroughUuid)

writeUuid = btle.UUID("0000ffe9-0000-1000-8000-00805f9b34fb")
writeCharacteristic = passthroughService.getCharacteristics(writeUuid)[0]

print "Ready to send"

while True:
    userInput = raw_input()
    writeCharacteristic.write(bytes(str(userInput)))
