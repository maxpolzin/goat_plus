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

    # Simulate connecting to the drone and setting MAVLink timeout
    await drone.connect(system_address="serial:///dev/ttyACM0:57600")
    await drone.core.set_mavlink_timeout(0.02)

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
        actuator_1 = await poll_trigger_values(gamepad)

        actuator_2, actuator_3 = 0, 0
        pressed = False
        for button, name in button_mappings.items():
            if await check_button_pressed(gamepad, button):
                actuator_2, actuator_3 = actuator_values[name]
                pressed = True
                break

        if not pressed:
            dpad_direction = await check_dpad_state(gamepad)
            if dpad_direction:
                actuator_2, actuator_3 = actuator_values[dpad_direction]


        print(f"Setting Actuators: 1 to {actuator_1}, 2 to {actuator_2}, 3 to {actuator_3}")
        try:
            await drone.action.set_actuator(1, actuator_1)
        except Exception as e:
            pass
        try:
            await drone.action.set_actuator(2, actuator_2)
        except Exception as e:
            pass
        try:
            await drone.action.set_actuator(3, actuator_3)
        except Exception as e:
            pass

        await asyncio.sleep(0.1)


if __name__ == "__main__":
    asyncio.run(run())
