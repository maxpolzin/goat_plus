#!/usr/bin/env python3

import asyncio
from evdev import InputDevice, ecodes

GAMEPAD_DEVICE = '/dev/input/event9'

async def poll_trigger_values(device):
    l2_state = device.absinfo(ecodes.ABS_Z)
    r2_state = device.absinfo(ecodes.ABS_RZ)

    l2_value = l2_state.value / 255.0 if l2_state else 0
    r2_value = -r2_state.value / 255.0 if r2_state else 0

    return l2_value + r2_value

async def check_button_pressed(device, button_code):
    """Check if a specific button is currently pressed."""
    try:
        current = device.active_keys()
    except IOError:  # May occur if the device is temporarily unavailable
        return False
    return button_code in current

async def check_dpad_state(device):
    """Check the state of the D-pad (directional pad)."""
    dpad_x = device.absinfo(ecodes.ABS_HAT0X)
    dpad_y = device.absinfo(ecodes.ABS_HAT0Y)

    if dpad_x and dpad_x.value == -1:
        return 'LeftArrow'
    elif dpad_x and dpad_x.value == 1:
        return 'RightArrow'
    elif dpad_y and dpad_y.value == -1:
        return 'UpArrow'
    elif dpad_y and dpad_y.value == 1:
        return 'DownArrow'
    return None

async def run():
    gamepad = InputDevice(GAMEPAD_DEVICE)

    actuator_values = {
        'Triangle': (-1, -1),
        'Circle': (-1, 1),
        'Square': (1, -1),
        'X': (1, 1),
        'LeftArrow': (1, 0),
        'RightArrow': (-1, 0),
        'UpArrow': (0, -1),
        'DownArrow': (0, 1)
    }

    button_mappings = {
        ecodes.BTN_NORTH: 'Triangle',
        ecodes.BTN_EAST: 'Circle',
        ecodes.BTN_WEST: 'Square',
        ecodes.BTN_SOUTH: 'X'
    }

    while True:
        try:
            value = await poll_trigger_values(gamepad)
            # print(f"Morphing motors set to: {value}")

            # Check for button presses
            pressed = False
            for button, name in button_mappings.items():
                if await check_button_pressed(gamepad, button):
                    actuator_2, actuator_3 = actuator_values[name]
                    print(f"Button {name} pressed - Setting Actuators: 2 to {actuator_2}, 3 to {actuator_3}")
                    pressed = True
                    break

            # Check for D-pad directions if no other button is pressed
            if not pressed:
                dpad_direction = await check_dpad_state(gamepad)
                if dpad_direction:
                    actuator_2, actuator_3 = actuator_values[dpad_direction]
                    print(f"D-pad {dpad_direction} pressed - Setting Actuators: 2 to {actuator_2}, 3 to {actuator_3}")

            await asyncio.sleep(0.1)  # Adjust based on your needs

        except Exception as e:
            print(e)

if __name__ == "__main__":
    asyncio.run(run())
