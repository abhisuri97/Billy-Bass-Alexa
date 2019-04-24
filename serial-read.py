#!/usr/bin/env python
import sys
import glob
import serial
import time
import requests

last_publish = 0
URL = "https://ping-tester-ese190.herokuapp.com/"
def publish_data(ln):
    global last_publish
    if abs(last_publish - time.time()) > 1.0:
        if (len(ln) == 5 and float(ln[3]) > 0.5 and float(ln[4]) > 0.5):
            r = requests.get("{}update?light={}&humidity={}&temperature={}".format(URL, ln[2], ln[3], ln[4]))
            print(r.status_code, r.text)
            print("published: light", ln[2], ", hum: ", ln[3], ", temp: ", ln[4])
        last_publish = time.time()

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/cu.usb[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/cu.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def get_port():
    while True:
        candidate_ports = []
        s_port = None
        while s_port == None:
            candidate_ports = serial_ports()
            for s in candidate_ports:
                if s.find('cu.usbmodem') > -1:
                    s_port = s
                    break
            print("port not found: {}".format(candidate_ports))
            time.sleep(1)
        print("Found port: {}".format(s_port))


        with serial.Serial(s_port, baudrate=9600, parity=serial.PARITY_NONE, 
                       stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS, timeout=0) as ser:
            print("connected to: " + ser.portstr)

            #this will store the line
            line = []
            connected = True
            while connected==True:
                try:
                    for c in ser.read():

                        line.append(c)
                        if c == 10:
                            ln = ''.join(chr(i) for i in line)
                            ln = ln.replace("\r\n", "")
                            clean_ln = ln.split(" ")
                            publish_data(clean_ln)
                            line = []
                except Exception as e:
                    connected=False
                    print("Lost connection! Plug back in! {}".format(e))

            ser.close()
get_port()
