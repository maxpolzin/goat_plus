#!/usr/bin/env python3

import asyncio
from evdev import InputDevice, ecodes, list_devices
from mavsdk import System


async def find_gamepad_device():
    devices = [InputDevice(path) for path in list_devices()]
    for device in devices:
        if 'Wireless Controller' in device.name:
            return device.path
    return None

async def poll_trigger_values(device):
    l2_state = device.absinfo(ecodes.ABS_Z)
    r2_state = device.absinfo(ecodes.ABS_RZ)

    l2_value = l2_state.value / 255.0 if l2_state else 0
    r2_value = r2_state.value / 255.0 if r2_state else 0

    return l2_value, r2_value

async def check_button_pressed(device, button_codes):
    try:
        current = device.active_keys()
    except IOError:
        return False

    def intersection(lst1, lst2):
        lst3 = [value for value in lst1 if value in lst2]
        return lst3
    return intersection(button_codes,current)

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

    gamepad_device_path = await find_gamepad_device()
    if gamepad_device_path:
        print(f"Found gamepad at: {gamepad_device_path}")
    else:
        print("No gamepad found.")
        return

    gamepad = InputDevice(gamepad_device_path)
    should_tension_tendon_loops = False  

    drone = System()

    print("Connecting...")
    # await drone.connect(system_address="serial:///dev/ttyUSB0:57600")
    # await drone.connect(system_address="tcp://localhost:5760")
    # await drone.connect(system_address="tcp://192.168.2.1:5760")
    # await drone.connect(system_address="udp://192.168.2.1:14550")
    await drone.connect(system_address="udp://:14551")
    print("Connected.")

    await drone.core.set_mavlink_timeout(3.0)


    actuator_values = {
        'Triangle': (1, 1),
        'Circle': (1, -1),
        'Square': (-1, 1),
        'X': (-1, -1),
        'LeftArrow': (-1, 0),
        'RightArrow': (1, 0),
        'UpArrow': (0, 1),
        'DownArrow': (0, -1)
    }

    button_mappings = {
        ecodes.BTN_NORTH: 'Triangle',
        ecodes.BTN_EAST: 'Circle',
        ecodes.BTN_WEST: 'Square',
        ecodes.BTN_SOUTH: 'X',
        ecodes.BTN_START: 'Options',
        ecodes.BTN_SELECT: 'Share'
    }

    msg_count = 0

    while True:

        pressed_buttons = await check_button_pressed(gamepad, button_mappings.keys())

        l2_value, r2_value = await poll_trigger_values(gamepad)

        share_button_state = ecodes.BTN_SELECT in pressed_buttons

        if share_button_state and not prev_share_button_state:
            should_tension_tendon_loops = not should_tension_tendon_loops
        prev_share_button_state = share_button_state


        button_pushed_threshold = 0.1

        if ecodes.BTN_START in pressed_buttons:
            loop_tendon_1 = -0.4
            loop_tendon_2 = -0.4
        elif l2_value > button_pushed_threshold and r2_value > button_pushed_threshold:
            loop_tendon_1 = (l2_value+r2_value)/2
            loop_tendon_2 = (l2_value+r2_value)/2
        elif l2_value > button_pushed_threshold:
            loop_tendon_1 = l2_value
            loop_tendon_2 = -l2_value/10.0
        elif r2_value > button_pushed_threshold:
            loop_tendon_1 = -r2_value/10.0
            loop_tendon_2 = r2_value
        else: 
            if should_tension_tendon_loops:
                loop_tendon_1 = 0.5
                loop_tendon_2 = 0.5
            else:
                loop_tendon_1 = 0.0
                loop_tendon_2 = 0.0




        # Map normalized velocities to actuator values
        no_motion = -1.0
        
        def map_to_actuator_value(value):
            return 2*abs(value) - 1.0

        if loop_tendon_1 >= 0:
            actuator_3 = map_to_actuator_value(loop_tendon_1)
            actuator_4 = no_motion
        elif loop_tendon_1 < 0:
            actuator_3 = no_motion
            actuator_4 = map_to_actuator_value(loop_tendon_1)

        if loop_tendon_2 >= 0:
            actuator_5 = map_to_actuator_value(loop_tendon_2)
            actuator_6 = no_motion
        elif loop_tendon_2 < 0:
            actuator_5 = no_motion
            actuator_6 = map_to_actuator_value(loop_tendon_2)

   

        actuator_1, actuator_2 = 0, 0
        for button in pressed_buttons:
            name = button_mappings[button]
            if name in actuator_values:
                actuator_1, actuator_2 = actuator_values[name]

        dpad_direction = await check_dpad_state(gamepad)
        if dpad_direction:
            actuator_1, actuator_2 = actuator_values[dpad_direction]


        try:
            await drone.action.set_actuator(1, actuator_1)
            await drone.action.set_actuator(2, actuator_2)
            await drone.action.set_actuator(3, actuator_3)
            await drone.action.set_actuator(4, actuator_4)
            await drone.action.set_actuator(5, actuator_5)
            await drone.action.set_actuator(6, actuator_6)
            print(f"Messages sent: {msg_count}")
            print(f"Loop tendons engaged: {should_tension_tendon_loops}")
            print(f"Central winches: {actuator_1}, {actuator_2}, loop winches: {actuator_3}, {actuator_4}, {actuator_5}, {actuator_6}")
            msg_count = msg_count+1

        except Exception as e:
            print(e)
            pass

        # await asyncio.sl1eep(0.1)


if __name__ == "__main__":
    asyncio.run(run())
