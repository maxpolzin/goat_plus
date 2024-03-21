#!/usr/bin/env python3

import random
import asyncio
from mavsdk import System
import mavsdk

index = 1
value = 0.0


async def run():

    drone = System()
    # await drone.connect(system_address="tcp://127.0.0.1:5760")
    await drone.connect(system_address="serial:///dev/ttyACM0:57600")

    await drone.core.set_mavlink_timeout(0.02)

    while True:
        try:
            await drone.action.set_actuator(index, random.random())
            # info = await drone.info.get_version()
            # print(info)

        except Exception as e:
            print(e)




if __name__ == "__main__":

    asyncio.run(run())