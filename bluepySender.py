# Prerequisites: sudo apt-get install libglib2.0-dev; sudo pip install bluepy
# References: https://elinux.org/RPi_Bluetooth_LE, https://ianharvey.github.io/bluepy-doc/
from bluepy import btle
# Prerequisites: sudo pip install RPi.GPIO
# References: https://pypi.org/project/RPi.GPIO/ 
import RPi.GPIO as GPIO

# Initializing Pi GPIO Port 18 (Blue LED)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(18, GPIO.OUT)
GPIO.output(18, 0) # Pin 18 (Blue LED) off

# Connection attempt loop
loopVar = True
atm = 1 # current connection attempt
while loopVar:
    print("Attempt at Connection " + str(atm))
    try:
        # btle.Peripheral throws btle.BTLEDisconnectError if pairing unsuccessful
        dev = btle.Peripheral("62:00:A1:21:6E:67")
        passthroughUuid = btle.UUID("0000ffe0-0000-1000-8000-00805f9b34fb")
        passthroughService = dev.getServiceByUUID(passthroughUuid)

        writeUuid = btle.UUID("0000ffe9-0000-1000-8000-00805f9b34fb")
        writeCharacteristic = passthroughService.getCharacteristics(writeUuid)[0]

        GPIO.output(18, 1) # Pin 18 (Blue LED) on
        print("Pairing Successful")
        loopVar = False
    except btle.BTLEDisconnectError:
        print("Connection Attempt " + str(atm) + " Failed")
    atm += 1
    
# Values to send 0 - 87 + 88 (e)
while True:
    # Sample array data
    num = [88, 88, 88, 88, 88, 87, 43, 34, 50, 55]
    byteNum = bytes(num)
    print(byteNum)
    writeCharacteristic.write(byteNum)