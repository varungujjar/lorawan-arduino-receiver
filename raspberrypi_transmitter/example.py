import PyLora
import time
import sys
 
PyLora.init()
PyLora.set_frequency(434000000)
PyLora.set_spreading_factor(12)
PyLora.set_bandwidth(250000)
PyLora.set_preamble_length(8)
PyLora.set_coding_rate(8)
PyLora.enable_crc()

# encode text to hex
# list("hello".encode('ascii'))
# print(bytes(payload).decode("utf-8",'strict'))
# list(map(int,"hello".encode('ascii')))


def getPayload(message, destinationAddress=0XBB, vibrateCount=0, vibrateDuration=0, vibrateInterval=0):
    payload = list(message.encode('ascii'))
    localAddress = 0xFF
    data = []
    data.append(0x20) #This is a spacer important or else arduino wont accept packet.
    data.append(destinationAddress)
    data.append(localAddress)
    data.append(vibrateCount)
    data.append(vibrateDuration/100)
    data.append(vibrateInterval/100)
    data.append(len(payload))
    for i in range(len(payload)):
        data.append(payload[i])
    return data


count = 0

while True:
    count +=1    
    payload = getPayload(message="Hello "+str(count), vibrateCount=2, vibrateDuration=1000, vibrateInterval=200)
    PyLora.send_packet(payload)
    print(payload)
    print('Packet sent...')
    time.sleep(2)
    # PyLora.close()
