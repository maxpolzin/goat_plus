#!/usr/bin/env python3

import asyncio
from evdev import InputDevice, ecodes, categorize

GAMEPAD_DEVICE = '/dev/input/event9'

async def poll_trigger_values(device):
    l2_state = device.absinfo(ecodes.ABS_Z)
    r2_state = device.absinfo(ecodes.ABS_RZ)

    l2_value = l2_state.value / 255.0 if l2_state else 0
    r2_value = -r2_state.value / 255.0 if r2_state else 0

    return l2_value + r2_value

async def check_x_pressed(device):
    """Check if the X button is currently pressed."""
    x_button = ecodes.BTN_SOUTH  # Adjust as needed for your controller
    try:
        # This method returns a list of currently pressed buttons
        current = device.active_keys()
    except IOError:  # May occur if the device is temporarily unavailable
        return False
    return x_button in current

async def run():
    gamepad = InputDevice(GAMEPAD_DEVICE)

    # Simulate connecting to the drone and setting MAVLink timeout
    #await drone.connect(system_address="serial:///dev/ttyACM0:57600")
    #await drone.core.set_mavlink_timeout(0.02)

    while True:
        try:
            value = await poll_trigger_values(gamepad)
            print(f"Morphing motors set to: {value}")
            # Simulate setting the actuator for the morphing motors
            # await drone.action.set_actuator(1, value)

            # Check if the X button is pressed and set actuators 2 and 3 accordingly
            if await check_x_pressed(gamepad):
                print("X is pressed - Actuators 2 and 3 set to 1")
                # await drone.action.set_actuator(2, 1.0)
                # await drone.action.set_actuator(3, 1.0)
            else:
                # await drone.action.set_actuator(2, 0.0)
                # await drone.action.set_actuator(3, 0.0)
                pass

            await asyncio.sleep(0.1)  # Adjust based on your needs

        except Exception as e:
            print(e)

if __name__ == "__main__":
    asyncio.run(run())
