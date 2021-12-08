def finger_status_loader_2way(xyz, handedness):
    # [0, 0, 0] 순서대로 엄지, 검지, 중지 & 0은 펴져 있는 상태, 1은 접은 상태
    status = [0, 0, 0]

    if handedness == 'right':
        status[0] = 1 if xyz[2][0] < xyz[4][0] else 0
    else:
        status[0] = 1 if xyz[2][0] > xyz[4][0] else 0

    status[1] = 1 if xyz[6][1] < xyz[7][1] and xyz[6][1] < xyz[8][1] else 0
    
    status[2] = 1 if xyz[10][1] < xyz[11][1] and xyz[10][1] < xyz[12][1] else 0

    output = status[0]*4 + status[1]*2 + status[2]*1

    return output

import serial



def finger_status_loader_5way(xyz, handedness):
    # [0, 0, 0] 순서대로 엄지, 검지, 중지 & 0은 펴져 있는 상태, 4은 접은 상태 + 0~4 normalize
    status = [0, 0, 0]
    len58 = cal_len_2d(xyz[8][0], xyz[8][1], xyz[5][0], xyz[5][1])
    len50 = cal_len_2d(xyz[0][0], xyz[0][1], xyz[5][0], xyz[5][1])
    len912 = cal_len_2d(xyz[12][0], xyz[12][1], xyz[9][0], xyz[9][1])
    len90 = cal_len_2d(xyz[0][0], xyz[0][1], xyz[9][0], xyz[9][1])

    if handedness == 'left':   
        if xyz[4][0] > xyz[9][0]:
            status[0] = 0x05
        elif xyz[4][0] > xyz[5][0]:
            status[0] = 0x04
        elif xyz[4][0] > (xyz[1][0]+xyz[2][0])/2:
            status[0] = 0x03
        elif xyz[4][0] > xyz[3][0]:
            status[0] = 0x02
        elif xyz[4][0] < xyz[3][0]:
            status[0] = 0x01

    else:  
        if xyz[4][0] < xyz[9][0]:
            status[0] = 0x05
        elif xyz[4][0] < xyz[5][0]:
            status[0] = 0x04
        elif xyz[4][0] < (xyz[1][0]+xyz[2][0])/2:
            status[0] = 0x03
        elif xyz[4][0] < xyz[3][0]:
            status[0] = 0x02
        elif xyz[4][0] > xyz[3][0]:
            status[0] = 0x01

    
    # Index Finger
    if xyz[7][1] > xyz[5][1]:
        status[1] = 0x05
    elif xyz[8][1] > xyz[5][1]:
        status[1] = 0x04
    elif xyz[8][1] > xyz[6][1]:
        status[1] = 0x03
    # This ratio is fitted by hand size of jeongts. 
    # Change this ratio to fit your hand's size
    elif len58/len50 < 0.7:
        status[1] = 0x02
    elif len58/len50 >= 0.7:
        status[1] = 0x01

    # Middle Finger
    if xyz[11][1] > xyz[9][1]:
        status[2] = 0x05
    elif xyz[12][1] > xyz[9][1]:
        status[2] = 0x04
    elif xyz[12][1] > xyz[10][1]:
        status[2] = 0x03
    # This ratio is fitted by hand size of jeongts. 
    # Change this ratio to fit your hand's size
    elif len912/len90 < 0.75:
        status[2] = 0x02
    elif len912/len90 >= 0.75:
        status[2] = 0x01

    return status


def cal_len_2d(x1, y1, x2, y2):
    import math
    return math.sqrt(math.pow(x2-x1, 2) + math.pow(y2-y1, 2))


# convert hex number to byte data for serial communication
def h2b(hex):
    return bytes(bytearray([hex]))


# Serial Communication to Arduino
def openSerial(port, baudrate=115200, bytesize=serial.EIGHTBITS, 
            parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, 
            timeout=None, xonxoff=False, rtscts=False, dsrdtr=False):

    ser = serial.Serial()

    ser.port = port
    ser.baudrate = baudrate
    ser.bytesize = bytesize
    ser.parity = parity
    ser.stopbits = stopbits
    ser.timeout = timeout
    ser.xonxoff = xonxoff
    ser.rtscts = rtscts
    ser.dsrdtr = dsrdtr

    ser.open()
    return ser