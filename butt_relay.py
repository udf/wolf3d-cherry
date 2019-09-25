from gpiozero import Button
from signal import pause
from time import sleep
from pynput.keyboard import Key, Controller

keyboard = Controller()

pins = {
        'A': {"pin": 2, "key": Key.shift},
        'B': {"pin": 4, "key": 'x'},
        'Start': {"pin": 3, "key": Key.esc},
        'R': {"pin": 6, "key": Key.right},
        'L': {"pin": 0, "key": Key.left},
        'U': {"pin": 13, "key": Key.up},
        'D': {"pin": 5, "key": Key.down}
}

buttons = {}

for key, val in pins.items():
    b = Button(val["pin"])
    buttons[key] = {}
    buttons[key]["button"] = b
    buttons[key]["value"] = False
    buttons[key]["key"] = val["key"] 

while True:
    for name, content in buttons.items():
        if content["button"].is_pressed == True:
            if content["value"] == False:
                content["value"] = True
                keyboard.press(content["key"])
        else:
            if content["value"] == True:
                content["value"] = False
                keyboard.release(content["key"])

        
    sleep(0.001)
