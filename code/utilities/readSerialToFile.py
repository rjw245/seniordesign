import serial

import os.path

num = 0;
done = false;
file = ""
while not done:

	file = "./../../data/data" + Str(num)+".csv"
	if not os.path.isfile(file):
		done = true
		break

f = open(file, "w")

portname = ""
ser = serial.Serial(portname)

while true:
	l = ser.readline()
	f.write(l)
	