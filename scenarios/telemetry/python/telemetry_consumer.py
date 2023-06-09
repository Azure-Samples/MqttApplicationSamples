# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import sys
from connectionsettings import connection_settings as cs
from argparse import ArgumentParser
import logging
import time
import paho.mqtt.client as mqtt
import ssl
import threading

parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)
logging.getLogger("paho").setLevel(level=logging.DEBUG)
messages=[]

connected_cond = threading.Condition()
connected_prop = False
connection_error = None
subscribed_cond = threading.Condition()
subscribed_prop = False

def on_connect(client, _userdata, _flags, rc):
    global connected_prop
    print("Connected to MQTT broker")
    # # In Paho CB thread.
    with connected_cond:
        if rc == mqtt.MQTT_ERR_SUCCESS:
            connected_prop = True
        else:
            connection_error = Exception(mqtt.connack_string(rc))
        connected_cond.notify_all()

def on_subscribe(client, _userdata, mid, _granted_qos):
    global subscribed_prop
    print(f"Subscribe for message id {mid} acknowledged by MQTT broker")
    # # In Paho CB thread.
    with subscribed_cond:
        subscribed_prop = True
        subscribed_cond.notify_all()

def on_message(_client, _userdata, message):
    # # In Paho CB thread.
    print(f"Received message on topic {message.topic} with payload {message.payload}")
    msg = str(message.payload.decode("utf-8"))
    messages.append(msg)

def on_disconnect(_client, _userdata, rc):
    print("Received disconnect with error='{}'".format(mqtt.error_string(rc)))
    global connected_prop
    # # In Paho CB thread.
    with connected_cond:
        connected_prop = False
        connected_cond.notify_all()

def wait_for_connected(timeout: float = None) -> bool:
    with connected_cond:
        connected_cond.wait_for(lambda: connected_prop or connection_error, timeout=timeout, )
        if connection_error:
            raise connection_error
        return connected_prop

def wait_for_subscribed(timeout: float = None) -> bool:
    with subscribed_cond:
        subscribed_cond.wait_for(
            lambda: subscribed_prop, timeout=timeout,
        )
        return subscribed_prop

def wait_for_disconnected(timeout: float = None):
    with connected_cond:
        connected_cond.wait_for(lambda: not connected_prop, timeout=timeout, )

def create_mqtt_client(client_id, connection_settings):
    mqtt_client = mqtt.Client(
        client_id=client_id,
        clean_session=connection_settings['MQTT_CLEAN_SESSION'],
        protocol=mqtt.MQTTv311,
        transport="tcp",
    )
    if 'MQTT_USERNAME' in connection_settings:
        mqtt_client.username_pw_set(
            username=connection_settings['MQTT_USERNAME'],
            password=connection_settings['MQTT_PASSWORD'] if 'MQTT_PASSWORD' in connection_settings else None
        )
    if connection_settings['MQTT_USE_TLS']:
        context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)

        if connection_settings['MQTT_CERT_FILE']:
            context.load_cert_chain(
                certfile=connection_settings['MQTT_CERT_FILE'],
                keyfile=connection_settings['MQTT_KEY_FILE'],
                password=connection_settings['MQTT_KEY_FILE_PASSWORD']
                )
        if "MQTT_CA_FILE" in connection_settings:
            context.load_verify_locations(
                cafile=connection_settings['MQTT_CA_FILE'],
            )
        else:
            context.load_default_certs()

        mqtt_client.tls_set_context(context)
    return mqtt_client

def main():
    connection_settings = cs.get_connection_settings(args.env_file)
    if not connection_settings["MQTT_CLEAN_SESSION"]:
        raise ValueError("This sample does not support connecting with existing sessions")

    # INITIALIZE
    print("Initializing Paho MQTT client")
    client_id = connection_settings["MQTT_CLIENT_ID"]
    mqtt_client = create_mqtt_client(client_id, connection_settings)

    # ATTACH HANDLERS
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.enable_logger()

    try:
        print("{}: Starting connection".format(client_id))
        hostname = connection_settings['MQTT_HOST_NAME']
        port = connection_settings['MQTT_TCP_PORT']
        keepalive = connection_settings["MQTT_KEEP_ALIVE_IN_SECONDS"]
        mqtt_client.connect(hostname, port, keepalive)
        print("Starting network loop")
        mqtt_client.loop_start()

        # WAIT FOR CONNECT
        if not wait_for_connected(timeout=10):
            print("{}: failed to connect.  exiting sample".format(client_id))
            raise TimeoutError("Timeout out trying to connect")
        # SUBSCRIBE
        topic = "vehicles/+/position"
        (_subscribe_result, subscribe_mid) = mqtt_client.subscribe(topic)
        print(f"Sending subscribe requestor topic \"{topic}\" with message id {subscribe_mid}")

        # WAIT FOR SUBSCRIBE
        if not wait_for_subscribed(timeout=10):
            print("{}: failed to subscribe.  exiting sample without publishing".format(client_id))
            raise TimeoutError("Timeout out trying to subscribe")

        # WAIT FOR MESSAGE RECEIVED
        print("Waiting to receive message indefinitely....")
        while True:
            time.sleep(60)
    except KeyboardInterrupt:
        print("User initiated exit")
    except Exception:
        print("Unexpected exception!")
        raise
    finally:
        num = len(messages)
        print("Number of message received is {}".format(num))
        # DISCONNECT
        print("{}: Disconnecting....".format(client_id))
        mqtt_client.disconnect()
        wait_for_disconnected(5)

if __name__ == "__main__":
    main()
