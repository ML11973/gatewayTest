from time import sleep
import logging
import sys
import signal
from datetime import datetime
import os

from moduleRm1s3 import ModuleRm1s3
from ism_stats import decode_stats

# Test utility for RM1S3 module.
# This script runs a gateway on a RM1S3 module connected by USB

COM_SERVER = '/dev/ttyUSB0'#'COM5'

POWER = 0x10 # Use lower RF power for powering by USB

PHY = 0x00 # 0=ETSI, 3=FCC_64, 4=FCC_64_LO, 5=FCC_64_HI, 6=FCC_128

SERVER_ADR = 0
CLIENT_ADR = 8
GROUP = 1
DATA_SLOT_COUNT = 9
FRAME_SIZE = 239#140

config_server = [
    [0x02, 0x60, POWER], # RF power
    [0x05, 0x40, 0x19, 0x17, 0x10, 0x25], # pattern
    [0x09, 0xA8, 0xD3, 0x22, 0xFE, 0x7D, 0x02, 0xD3, 0xD1, 0x17], # Gateway ID
    [0x02, 0xA0, 0x00], # sync mode tx
    [0x02, 0x62, PHY],  # Phy
    [0x02, 0x5C, 0x04], # Tx retry count = 2
    [0x02, 0xC8, 0x00], # Tx retry restriction = next cycle
    [0x02, 0x20, DATA_SLOT_COUNT], # data slot count
    [0x02, 0xE0, 0x01], # SIG_SYNC_OUT to GPIO0
    [0x02, 0xE2, 0x02], # SIG_TIMESLOT_OUT to GPIO1
    [0x02, 0x44, SERVER_ADR], # address
    [0x02, 0x70, 0x01], # radio mode active
]

terminate = False

def signal_handling(signum, frame):
    global terminate
    terminate = True

signal.signal(signal.SIGINT, signal_handling)

def wait_sync():
    while not terminate:
        if client.is_synchronized():
            break
        sleep(1)

def create_frame(destination, dataslot, size):
    frame = (size + 3).to_bytes(1, 'big') # size
    frame += b'\xD2'
    frame += destination.to_bytes(1, 'big') # destination
    frame += dataslot.to_bytes(1, 'big') # data slot
    frame += os.urandom(size)
    return frame


def wakeup_group_frame(group):
    frame = (5).to_bytes(1, 'big') # size
    frame += b'\xA6'
    frame += group.to_bytes(4, 'big') # group
    return frame

def unwakeup_groups_frame():
    frame = (5).to_bytes(1, 'big') # size
    frame += b'\xA6'
    frame += (0).to_bytes(4, 'big') # group
    return frame

def beacon_change_frame(size):
    frame = (size + 1).to_bytes(1, 'big')
    frame += b'\xAA'
    frame += os.urandom(size)
    return frame

logFormatter = logging.Formatter('%(asctime)s [%(name)-12.12s] [%(levelname)-5.5s]  %(message)s')
rootLogger = logging.getLogger()

fileHandler = logging.FileHandler('test_communication.log')
fileHandler.setFormatter(logFormatter)
rootLogger.addHandler(fileHandler)

consoleHandler = logging.StreamHandler()
consoleHandler.setFormatter(logFormatter)
rootLogger.addHandler(consoleHandler)
rootLogger.setLevel(logging.DEBUG)


server = ModuleRm1s3(COM_SERVER)
server.open()
server.set_baudrate(115200)
server_fingerprint = server.get_fw_hw_id()
print(server_fingerprint)
server.send_config(config_server)


#wait_sync()

rssi_server = {'cnt': 0, 'min': 0, 'max': -128, 'sum': 0}

def rssi_stat(rssi, frame):
    if frame is not None:
        val = frame[4] - 256
        rssi['cnt'] += 1
        rssi['sum'] += val
        rssi['avg'] = rssi['sum']/rssi['cnt']
        if val < rssi['min']: rssi['min'] = val
        if val > rssi['max']: rssi['max'] = val

menuOptions = {
    1: 'Wake up group',
    2: 'Unwake group',
    3: 'TX to client',
    4: 'RX dump',
    5: 'Change beacon data',
    6: 'Reconfigure gateway',
    0: 'Exit'
}

def printMenu():
    for key in menuOptions.keys():
        print(key,'--',menuOptions[key])
    choice = int(input('Choice: '))
    return choice

while not terminate:
    choice=printMenu()

    match choice:
        case 0:
            terminate=True
            #quit()
        case 1:
            frame = wakeup_group_frame(GROUP)
            server.write_frame(frame)
        case 2:
            frame = unwakeup_groups_frame()
            server.write_frame(frame)
        case 3:
            frame = create_frame(CLIENT_ADR, SERVER_ADR, FRAME_SIZE)
            server.write_frame(frame)
        case 4:
            rx=0;
            while(rx!=None):
                rx = server.read_frame(0.2)
                print(rx)
        case 5:
            frame = beacon_change_frame(2)
            server.write_frame(frame)

        case 6:
            server.send_config(config_server)

        case _:
            print('No selection')
    sleep(0.5)
    rx_frame = server.read_frame(0.2)
    print(rx_frame)


server_stats = decode_stats(server.get_stat())

server.close()

f = open('test_choice_results.txt', 'a')
f.write(f'{datetime.now()}:\n')
print('%22s %12s' % ('stats', 'server'))
f.write('%22s %12s\n' % ('stats', 'server'))
for key in server_fingerprint:
    print('%22s %12s' % (key, server_fingerprint[key]))
    f.write('%22s %12s\n' % (key, server_fingerprint[key]))
for key in server_stats:
    print('%22s %12d' % (key, server_stats[key]))
    f.write('%22s %12d\n' % (key, server_stats[key]))

print(f'rssi server: {rssi_server}')
f.write(f'rssi server: {rssi_server}\n')

f.close()
