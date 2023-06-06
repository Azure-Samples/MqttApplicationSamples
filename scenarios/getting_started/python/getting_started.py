# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import sys
from connectionsettings import connection_settings as cs
from connectionsettings import mqtt_helpers as mh
from argparse import ArgumentParser
import logging
import time


def on_connect(client, _userdata, _flags, rc):
    if rc != 0:
        print(f"Failed to connect to MQTT broker with return code {rc}")
        return
    
    print("Connected to MQTT broker")
    topic = "sample/+"
    # SUBSCRIBE
    (_subscribe_result, subscribe_mid) = client.subscribe("sample/+")
    print(f"Sending subscribe requestor topic \"{topic}\" with message id {subscribe_mid}")

def on_subscribe(client, _userdata, mid, _granted_qos):    
    print(f"Subscribe for message id {mid} acknowledged by MQTT broker")
    # PUBLISH
    topic = "sample/topic1"
    payload = "hello world!"
    publish_result = client.publish(topic, payload)
    print(f"Sending publish with payload \"{payload}\" on topic \"{topic}\" with message id {publish_result.mid}")

def on_publish(_client, _userdata, mid):
    print(f"Sent publish with message id {mid}") 

    
def on_message(_client, _userdata, message):
    print(f"Received message on topic {message.topic} with payload {message.payload}")


parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

logging.basicConfig(level=logging.DEBUG)

connection_settings = cs.get_connection_settings(args.env_file)
if not connection_settings["MQTT_CLEAN_SESSION"]:
    raise ValueError("This sample does not support connecting with existing sessions")

# INITIALIZE
print("Initializing Paho MQTT client")
client_id = connection_settings["MQTT_CLIENT_ID"]
mqtt_client = mh.create_mqtt_client(client_id, connection_settings)

# ATTACH HANDLERS
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.on_publish = on_publish
mqtt_client.on_subscribe = on_subscribe
mqtt_client.on_message = on_message

# CONNECT
print("{}: Starting connection".format(client_id))
hostname = connection_settings['MQTT_HOST_NAME']
port = connection_settings['MQTT_TCP_PORT']
keepalive = connection_settings["MQTT_KEEP_ALIVE_IN_SECONDS"]
mqtt_client.connect(hostname, port,keepalive)
print("Starting network loop")
mqtt_client.loop_forever()


