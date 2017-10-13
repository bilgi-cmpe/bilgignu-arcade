import RPi.GPIO as GPIO
from evdev import uinput, ecodes as e


GPIO.setmode(GPIO.BCM)
GPIO.setup(21, GPIO.IN, pull_up_down=GPIO.PUD_UP
def my_callback(channel):
    print "falling edge detected on 21"
    with uinput.UInput() as ui:
         ui.write(e.EV_KEY, e.KEY_A, 1)
         ui.write(e.EV_KEY, e.KEY_A, 0)
         ui.syn()

while True:
    GPIO.add_event_detect(17, GPIO.FALLING, callback=my_callback, bouncetime=300)
#GPIO.cleanup()
