import serial
import os.path
import sys

if __name__=="__main__":

    argc = len(sys.argv);
    assert(argc==4);
    arduino = serial.Serial(sys.argv[1],int(sys.argv[2]))
    f = open(sys.argv[3],'w');
    while True:
        amp = ord(arduino.read())
	ampStr = str(amp)+"\n"
        f.write(ampStr)
