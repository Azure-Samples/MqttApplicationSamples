# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import sys
from pahoclientwrapper import connection_settings as cs
from pahoclientwrapper import paho_client_wrapper as pc
from argparse import ArgumentParser
import logging
import time

parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

logging.basicConfig(level=logging.DEBUG)

connection_settings = cs.get_connection_settings(args.env_file)
if not connection_settings["MQTT_CLEAN_SESSION"]:
    raise ValueError("This sample does not support connecting with existing sessions")

# INITIALIZE
print("Initializing Paho MQTT client")
paho_client = pc.PahoClientWrapper.create_from_connection_settings(connection_settings)

# CONNECT ##
print("{}: Starting connection".format(paho_client.device_id))
paho_client.start_connect()
if not paho_client.connection_status.wait_for_connected(timeout=20):
    print("{}: failed to connect.  exiting sample".format(paho_client.device_id))
    sys.exit(1)
print("{}: Connected".format(paho_client.device_id))

# SUBSCRIBE
(rc, mid) = paho_client.subscribe("sample/+")
print("{}: Subscribe returned rc={}: {}".format(paho_client.device_id, rc, paho_client.error_string(rc)))

# PUBLISH
(rc, mid) = paho_client.publish("sample/topic", "hello world")
print("{}: Publish returned rc={}: {}".format(paho_client.device_id, rc, paho_client.error_string(rc)))

# SLEEP FOR SOME TIME TO RECEIVE THE MESSAGE
time.sleep(4)

# DISCONNECT
print("{}: Disconnecting".format(paho_client.device_id))
paho_client.disconnect()
paho_client.connection_status.wait_for_disconnected()

