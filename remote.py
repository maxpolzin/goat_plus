#!/usr/bin/env python3

import asyncio
import time
import socket
from evdev import InputDevice, ecodes, list_devices
from mavsdk import System
from tqdm import tqdm

class GamepadNotFoundError(Exception):
    pass

class Input:
    TRIANGLE_BUTTON = ecodes.BTN_NORTH
    CIRCLE_BUTTON = ecodes.BTN_EAST
    SQUARE_BUTTON = ecodes.BTN_WEST
    X_BUTTON = ecodes.BTN_SOUTH
    OPTIONS_BUTTON = ecodes.BTN_START
    SHARE_BUTTON = ecodes.BTN_SELECT
    PLAYSTATION_BUTTON = ecodes.BTN_MODE

    def __init__(self):
        self.gamepad = self.initialize_gamepad()
        self.ps_button_pressed_time = None
        self.ps_button_triggered = False        
        self.active_keys = []

    def initialize_gamepad(self):
        devices = [InputDevice(path) for path in list_devices()]
        for device in devices:
            if 'Wireless Controller' in device.name:
                print(f"Found gamepad at: {device.path}")
                return InputDevice(device.path)
        raise GamepadNotFoundError("No gamepad found.")

    def update(self):
        try:
            self.active_keys = self.gamepad.active_keys()
        except IOError:
            self.active_keys = []

    def poll_trigger_values(self):
        l2_state = self.gamepad.absinfo(ecodes.ABS_Z)
        r2_state = self.gamepad.absinfo(ecodes.ABS_RZ)

        l2_value = l2_state.value / 255.0 if l2_state else 0
        r2_value = r2_state.value / 255.0 if r2_state else 0

        return l2_value, r2_value

    def check_button_pressed(self):
        return [button for button in self.active_keys if button in (
            self.TRIANGLE_BUTTON, self.CIRCLE_BUTTON, self.SQUARE_BUTTON, self.X_BUTTON,
            self.OPTIONS_BUTTON, self.SHARE_BUTTON, self.PLAYSTATION_BUTTON)]

    def check_dpad_state(self):
        """Check the state of the D-pad (directional pad)."""
        dpad_x = self.gamepad.absinfo(ecodes.ABS_HAT0X)
        dpad_y = self.gamepad.absinfo(ecodes.ABS_HAT0Y)

        if dpad_x and dpad_x.value == -1:
            return 'LeftArrow'
        elif dpad_x and dpad_x.value == 1:
            return 'RightArrow'
        elif dpad_y and dpad_y.value == -1:
            return 'UpArrow'
        elif dpad_y and dpad_y.value == 1:
            return 'DownArrow'
        return None

    def is_ps_button_pressed(self):
        if self.PLAYSTATION_BUTTON in self.active_keys:
            if self.ps_button_pressed_time is None:
                self.ps_button_pressed_time = time.time()
            elif not self.ps_button_triggered and time.time() - self.ps_button_pressed_time > 2:
                self.ps_button_triggered = True
                return True
        else:
            self.ps_button_pressed_time = None
            self.ps_button_triggered = False
        return False

class FrameActuation:
    def __init__(self, button_pushed_threshold=0.1):
        self.should_tension_tendon_loops = False
        self.prev_share_button_state = False
        self.button_pushed_threshold = button_pushed_threshold

    def update(self, pressed_buttons, l2_value, r2_value):
        share_button_state = Input.SHARE_BUTTON in pressed_buttons
        if share_button_state and not self.prev_share_button_state:
            self.should_tension_tendon_loops = not self.should_tension_tendon_loops
        self.prev_share_button_state = share_button_state

        if Input.OPTIONS_BUTTON in pressed_buttons:
            loop_tendon_1 = -0.4
            loop_tendon_2 = -0.4
        elif l2_value > self.button_pushed_threshold and r2_value > self.button_pushed_threshold:
            loop_tendon_1 = (l2_value + r2_value) / 2
            loop_tendon_2 = (l2_value + r2_value) / 2
        elif l2_value > self.button_pushed_threshold:
            loop_tendon_1 = l2_value
            loop_tendon_2 = -l2_value / 10.0
        elif r2_value > self.button_pushed_threshold:
            loop_tendon_1 = -r2_value / 10.0
            loop_tendon_2 = r2_value
        else:
            if self.should_tension_tendon_loops:
                loop_tendon_1 = 0.5
                loop_tendon_2 = 0.5
            else:
                loop_tendon_1 = 0.0
                loop_tendon_2 = 0.0

        no_motion = -1.0

        def map_to_actuator_value(value):
            return 2 * abs(value) - 1.0

        actuator_commands = [0] * 4

        if loop_tendon_1 >= 0:
            actuator_commands[0] = map_to_actuator_value(loop_tendon_1)
            actuator_commands[1] = no_motion
        else:
            actuator_commands[0] = no_motion
            actuator_commands[1] = map_to_actuator_value(loop_tendon_1)

        if loop_tendon_2 >= 0:
            actuator_commands[2] = map_to_actuator_value(loop_tendon_2)
            actuator_commands[3] = no_motion
        else:
            actuator_commands[2] = no_motion
            actuator_commands[3] = map_to_actuator_value(loop_tendon_2)

        return actuator_commands

class WinchActuation:
    def __init__(self):
        self.actuator_values = {
            Input.TRIANGLE_BUTTON: (1, 1),
            Input.CIRCLE_BUTTON: (1, -1),
            Input.SQUARE_BUTTON: (-1, 1),
            Input.X_BUTTON: (-1, -1),
            'LeftArrow': (-1, 0),
            'RightArrow': (1, 0),
            'UpArrow': (0, 1),
            'DownArrow': (0, -1)
        }

    def update(self, pressed_buttons, dpad_direction):
        actuator_1, actuator_2 = 0, 0
        for button in pressed_buttons:
            if button in self.actuator_values:
                actuator_1, actuator_2 = self.actuator_values[button]

        if dpad_direction:
            actuator_1, actuator_2 = self.actuator_values[dpad_direction]

        return [actuator_1, actuator_2]


class ParamList:
    def __init__(self, filename):
        self.filename = filename
        self.params = {
            "int": {},
            "float": {},
            "custom": {}
        }
        self.read_params_from_file()

    def read_params_from_file(self):
        """Read parameters from a file and categorize them by type."""
        with open(self.filename, "r") as file:
            for line in file:
                if line.startswith("#") or not line.strip():
                    continue
                vehicle_id, component_id, name, value, type = line.strip().split("\t")
                if type == "6":  # Assuming type 6 is int
                    self.params["int"][name] = int(value)
                elif type == "9":  # Assuming type 9 is float
                    self.params["float"][name] = float(value)
                else:
                    self.params["custom"][name] = value


class ParamHandler:
    def __init__(self):
        self.copter_params = ParamList("px4_flying_goat_v1.15-beta1-short.params")
        self.rover_params = ParamList("px4_driving_goat_v1.15-beta1.params")

        self.differences = self.find_param_differences()

    def find_param_differences(self):
        """Finds names of parameters that are different or unique between copter and rover."""
        differences = {
            "int": set(),
            "float": set(),
            "custom": set()
        }

        # Check all parameter types for differences or uniqueness
        for param_type in ['int', 'float', 'custom']:
            copter_keys = set(self.copter_params.params[param_type].keys())
            rover_keys = set(self.rover_params.params[param_type].keys())

            # Find parameters that are different or unique to one set
            unique_or_different = (copter_keys ^ rover_keys) | {name for name in copter_keys & rover_keys if self.copter_params.params[param_type][name] != self.rover_params.params[param_type][name]}
            differences[param_type].update(unique_or_different)

        return differences

    async def upload_differing_and_missing_copter_params(self, drone):
        await self.upload_params(drone, self.copter_params.params)

    async def upload_differing_and_missing_rover_params(self, drone):
        await self.upload_params(drone, self.rover_params.params)


    async def upload_params(self, drone, param_list):
        print("Uploading parameters to the drone...")
        retry_limit = 3  # Set a retry limit for each parameter upload

        for param_type in ['int', 'float', 'custom']:
            # Create a sorted list of parameter names to upload
            sorted_params = sorted(param_list[param_type].items(), key=lambda x: x[0])
            
            for name, value in sorted_params:
                if name in self.differences[param_type]:
                    retries = 0
                    while retries < retry_limit:
                        try:
                            if param_type == 'int':
                                print(f"Setting {name} to {value}")
                                await drone.param.set_param_int(name, value)
                            elif param_type == 'float':
                                print(f"Setting {name} to {value}")
                                await drone.param.set_param_float(name, value)
                            else:
                                # Assuming custom parameters need specific handling
                                await drone.param.set_param_custom(name, value)
                            break  # Exit retry loop if successful
                        except Exception as e:
                            print(f"Error setting {name}: {e}")
                            retries += 1
                            await asyncio.sleep(1)  # Wait a bit before retrying

                    if retries == retry_limit:
                        print(f"Failed to set {name} after {retry_limit} attempts.")
        print("Parameters uploaded.")

class Goat():

    COPTER = 0
    ROVER = 6

    def __init__(self):
        self.drone = None
        self.ip = "192.168.2.1"
        self.param_handler = ParamHandler()
        differences = self.param_handler.find_param_differences()


    async def initialize(self):
        print("Connecting...")

        self.drone = System()
        self.wait_for_ip(self.ip, 5760)
        await self.drone.connect(system_address=f"tcp://{self.ip}")

        await self.drone.core.set_mavlink_timeout(5.0)
        
        self.mode = await self.drone.param.get_param_int("CA_AIRFRAME")

        print(f"Connected. GOAT is copter(0) or rover(6): {self.mode}")


    async def reboot(self):
        await self.drone.action.reboot()


    def wait_for_ip(self, ip_address, port, timeout=60):
        start_time = time.time()
        while time.time() - start_time < timeout:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.settimeout(1)  # Set a timeout on the blocking socket connect call
                try:
                    s.connect((ip_address, port))
                    print(f"{ip_address} is now reachable on port {port}.")
                    return True
                except socket.timeout:
                    print(f"Timeout connecting to {ip_address} on port {port}.")
                except socket.error as err:
                    print(f"Failed to connect to {ip_address} on port {port}: {err}")
                time.sleep(1)  # Wait a brief period before trying again

        print(f"Timeout reached. {ip_address} is not reachable after {timeout} seconds.")
        return False


    async def change_mode(self):
        print("Changing mode...")

        # to rover
        if self.mode == self.COPTER:
            await self.drone.param.set_param_int("CA_AIRFRAME", self.ROVER)
            await self.drone.param.set_param_int("SYS_AUTOSTART", 50003)
            params = self.param_handler.rover_params.params

        # to copter
        else:
            await self.drone.param.set_param_int("CA_AIRFRAME", self.COPTER)
            await self.drone.param.set_param_int("SYS_AUTOSTART", 4001)
            params = self.param_handler.copter_params.params

        await self.reboot()
        time.sleep(3)
        await self.initialize()

        await self.param_handler.upload_params(self.drone, params)

        self.mode = await self.drone.param.get_param_int("CA_AIRFRAME")

        print(f"Changed mode to {self.mode}")


async def run():
    input_handler = Input()

    goat = Goat()
    await goat.initialize()

    frame_actuation = FrameActuation()
    winch_actuation = WinchActuation()
    

    msg_count = 0

    while True:

        input_handler.update()
        pressed_buttons = input_handler.check_button_pressed()
        l2_value, r2_value = input_handler.poll_trigger_values()
        dpad_direction = input_handler.check_dpad_state()
        should_change_mode = input_handler.is_ps_button_pressed()

        if should_change_mode:            
            await goat.change_mode()

        frame_actuators = frame_actuation.update(pressed_buttons, l2_value, r2_value)
        winch_actuators = winch_actuation.update(pressed_buttons, dpad_direction)
        
        try:
            timeout_duration = 0.2

            await asyncio.wait_for(goat.drone.action.set_actuator(1, winch_actuators[0]), timeout_duration)
            await asyncio.wait_for(goat.drone.action.set_actuator(2, winch_actuators[1]), timeout_duration)
            await asyncio.wait_for(goat.drone.action.set_actuator(3, frame_actuators[0]), timeout_duration)
            await asyncio.wait_for(goat.drone.action.set_actuator(4, frame_actuators[1]), timeout_duration)
            await asyncio.wait_for(goat.drone.action.set_actuator(5, frame_actuators[2]), timeout_duration)
            await asyncio.wait_for(goat.drone.action.set_actuator(6, frame_actuators[3]), timeout_duration)
            
            # print(f"Messages sent: {msg_count}")
            # print(f"Loop tendons engaged: {frame_actuation.should_tension_tendon_loops}")
            # print(f"Central winches: {winch_actuators[0]}, {winch_actuators[1]}, loop winches: {frame_actuators[0]}, {frame_actuators[1]}, {frame_actuators[2]}, {frame_actuators[3]}")
            
            msg_count += 1

        except asyncio.TimeoutError:
            # print("A command timed out")
            pass

        except Exception as e:
            # print(e)
            pass

        # await asyncio.sleep(0.1)

if __name__ == "__main__":
    asyncio.run(run())
