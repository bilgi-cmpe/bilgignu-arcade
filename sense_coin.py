#!/usr/bin/python
'''
Credit: Bilgi Open Source Comunity - Orkun Asa, Enis B. Tuysuz
Charity Arcade Project
'''
import uinput
import time
import RPi.GPIO as GPIO

#Use GPIO numbers not pin numbers
#http://www.raspberrypi-spy.co.uk/2012/06/simple-guide-to-the-rpi-gpio-header-and-pins/
GPIO.setmode(GPIO.BCM)

#Setup port 26 as input
GPIO.setup(26, GPIO.IN)

device = uinput.Device([
	uinput.KEY_5
])

while True:
	#Get input value from port 26
	input_val = GPIO.input(26)
	if input_val:
		time.sleep(1)
		# Without this program ends too fast, uinput can't generate keypress
		device.emit_click(uinput.KEY_5)
