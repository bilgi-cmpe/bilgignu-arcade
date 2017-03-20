import time
import uinput

device = uinput.Device([uinput.KEY_A])

time.sleep(15)

device.emit_click(uinput.KEY_A)
time.sleep(2)
device.emit_click(uinput.KEY_A)
