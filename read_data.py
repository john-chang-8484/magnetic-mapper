#!/usr/bin/python2.7
import serial # for serial port
import numpy as np # for arrays, numerical processing
import csv

TEST = True # testing mode

#needs: python2, pyserial, numpy, 
#0) flash the serial temperature measurement program into the msp430
#1) start this program
#2) press the button the Launchpad to enter 'applcation mode'
#3) warm the chip (eg with a light bulb or your fingers)
#4) when you've seen enough, press the reset button on the launchpad
#5) exit the program by pressing 'q' or clicking on the x

#define the serial port. Pick one:
port = "/dev/ttyACM0"  #for Linux
#port = "COM5" #For Windows?
#port = "/dev/tty.uart-XXXX" #For Mac?


# compute the sign for the integer
def compute_sign(n):
    if n < 32768:
        return n
    else:
        return n - 65536


if TEST:
    integers = [1, 1, 3, 699, 698, 697, 3, 697, 698, 699, 3, 697, 697, 697, 0, 550, 1000, 700, 701, 702, 0, 600, 1000, 700, 701, 702, 0, 650, 1000, 700, 701, 702, 2]
else:
    integers = [] # the integers that will be read into the csv file
    #start our program proper:
    #open the serial port
    try:
        ser = serial.Serial(port,2400,timeout = 0.050) 
        ser.baudrate=9600
    # with timeout=0, read returns immediately, even if no data
    except:
        print ("Opening serial port",port,"failed")
        print ("Edit program to point to the correct port.")
        print ("Hit enter to exit")
        raw_input()
        quit()

    ser.flushInput()

    bytes = []
    while(1): #loop forever
        data = ser.read(1) # look for a character from serial port - will wait for up to 50ms (specified above in timeout)
        if len(data) > 0: #was there a byte to read?
            bytes.append(data)
            if len(bytes) > 1:
                num = compute_sign(256*ord(bytes[0]) + ord(bytes[1]))
                integers.append(num)
                print num
                bytes = []
        # end code is 2, 1
        if len(integers) >= 2 and integers[-1] == 1 and integers[-2] == 2:
            break

print "Parsing data and writing to file..."

# parse the data stream:
rows = [["azm", "alt", "B_r", "B_phi", "B_theta"]]
# calibration readings:
avgs = [0., 0., 0.]
count = 0

i = 0
while i < len(integers):
    if integers[i] == 1:
        i += 1
    elif integers[i] == 0: # read data
        rows.append(integers[i + 1 : i + 6])
        i += 6
    elif integers[i] == 2:
        break # done here
    elif integers[i] == 3:
        for j in range(0, 3):
            avgs[j] += integers[i + 1 + j]
        count += 1
        i += 4
    else:
        print "hmmm, invalid integer block header of %d" % integers[i]
        i += 1

# take averages:
for i in range(0, 3):
    avgs[i] = avgs[i] / float(count)

for i in range(1, len(rows)):
    for j in range(0, 3):
        rows[i][2 + j] -= avgs[j]

with open("out.csv", "w") as f:
    writer = csv.writer(f)
    writer.writerows(rows)

f.close()

