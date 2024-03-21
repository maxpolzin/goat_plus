#!/usr/bin/env python3

import asyncio
import os
import struct
import time
from mavsdk import System

# Adjust this path to your gamepad's device file
GAMEPAD_DEVICE = '/dev/input/js0'
# Define the timeout duration in seconds
TRIGGER_TIMEOUT = 1.0  # 1 second for example, adjust as needed

async def get_trigger_values():
    """
    Read the gamepad's device file to get the R2 and L2 trigger values.
    Returns a value between -1 and +1, where -1 is L2 fully pressed, and +1 is R2 fully pressed.
    Implements a timeout to return the value to 0 if neither trigger is pressed for a defined duration.
    """
    last_press_time = time.time()  # Track the last time a trigger was pressed

    # Open the gamepad device file in non-blocking mode
    with open(GAMEPAD_DEVICE, 'rb', buffering=0) as f:
        while True:
            # Check for timeout
            if time.time() - last_press_time > TRIGGER_TIMEOUT:
                return 0  # Return to neutral position after timeout

            # Attempt to read an event from the gamepad
            event = f.read(8)
            if not event:
                await asyncio.sleep(0.01)  # Small sleep to prevent a busy loop
                continue

            # Unpack the event
            _, value, type, number = struct.unpack('IhBB', event)

            # Adjust the logic for L2 and R2 triggers
            if type == 2:  # Analog type (triggers)
                if number == 2:  # L2 trigger
                    last_press_time = time.time()  # Update last press time
                    # Normalize the L2 value
                    l2_value = (value - (-32000)) / (32000 - (-32000))
                    return l2_value
                elif number == 5:  # R2 trigger
                    last_press_time = time.time()  # Update last press time
                    # Normalize and invert the R2 value
                    r2_value = (value - (-32000)) / (32000 - (-32000))
                    r2_value = r2_value * -1  # Invert to make 0 to -1
                    return r2_value

async def run():
    drone = System()
    await drone.connect(system_address="serial:///dev/ttyACM0:57600")
    await drone.core.set_mavlink_timeout(0.02)

    while True:
        try:
            value = await get_trigger_values()

            # Control actuator with the trigger value
            index = 1  # Example index, adjust as needed
            await drone.action.set_actuator(index, value)

        except Exception as e:
            print(e)

if __name__ == "__main__":
    asyncio.run(run())
