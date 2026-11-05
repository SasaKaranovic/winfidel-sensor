import re
import time
import serial

ser = serial.Serial()
ser.port = 'COM5'          # e.g. /dev/ttyACM0 on Linux
ser.baudrate = 115200
ser.timeout = 1
ser.dtr = False            # set before open(): toggling DTR/RTS resets the board
ser.rts = False            # or puts it in bootloader mode
ser.open()

time.sleep(2)              # let boot messages finish if the board did reset
ser.reset_input_buffer()
ser.write(b's')            # start diameter printout

pattern = re.compile(r'^>(\d+\.\d{1,2})mm$')

try:
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        m = pattern.match(line)
        if m:
            print(float(m.group(1)))
except KeyboardInterrupt:
    pass
finally:
    ser.write(b'e')        # stop printout so the device buffer doesn't fill
    ser.close()
