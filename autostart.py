"""Script for launching a game on RetroPie after startup.

This script launches the latest emulated device and first game on the list. It
is useful for arcade machines with one game. In event of reboot script launches
the game without human interaction.

This script is developed by students of Istanbul Bilgi University. You can use
this program under the terms of GNU General Public License. See LICENSE for
details.
"""
import time
import uinput

device = uinput.Device([uinput.KEY_A])
time.sleep(15) # Waiting for EmulationStation to launch.
device.emit_click(uinput.KEY_A)
time.sleep(2) # Just to make sure.
device.emit_click(uinput.KEY_A)
