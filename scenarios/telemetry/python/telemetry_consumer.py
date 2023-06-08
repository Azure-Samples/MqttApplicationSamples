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

connected_cond = threading.Condition()
connected_prop = False
connection_error = None
subscribed_cond = threading.Condition()
subscribed_prop = False
# published_cond = threading.Condition()
# published_prop = False
received_cond = threading.Condition()
received_prop = False

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

# def on_publish(_client, _userdata, mid):
#     print(f"Sent publish with message id {mid}")
#     global published_prop
#     # # In Paho CB thread.
#     with published_cond:
#         published_prop = True
#         published_cond.notify_all()

def on_message(_client, _userdata, message):
    print(f"Received message on topic {message.topic} with payload {message.payload}")
    # global received_prop
    # # In Paho CB thread.
    # with received_cond:
    #     received_prop = True
    #     received_cond.notify_all()

    msg = str(message.payload.decode("utf-8"))
    print('RECV Topic = ', message.topic)
    print('RECV MSG =', msg)

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

# def wait_for_published(timeout: float = None) -> bool:
#     with published_cond:
#         published_cond.wait_for(
#             lambda: published_prop, timeout=timeout,
#         )
#         return published_prop

def wait_for_receive(timeout: float = None) -> bool:
    with received_cond:
        received_cond.wait_for(
            lambda: received_prop, timeout=timeout,
        )
        return received_prop

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

class Point:
    def __init__(self,x_init,y_init):
        self.x = x_init
        self.y = y_init

    def shift(self, x, y):
        self.x += x
        self.y += y

    def __repr__(self):
        return "".join(["Point(", str(self.x), ",", str(self.y), ")"])


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
    # mqtt_client.on_publish = on_publish
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.enable_logger()

    
    # CONNECT
    # print("{}: Starting connection".format(client_id))
    # hostname = connection_settings['MQTT_HOST_NAME']
    # port = connection_settings['MQTT_TCP_PORT']
    # keepalive = connection_settings["MQTT_KEEP_ALIVE_IN_SECONDS"]
    # mqtt_client.connect(hostname, port, keepalive)
    # print("Starting network loop")
    # mqtt_client.loop_start()

    # # WAIT FOR CONNECT
    # if not wait_for_connected(timeout=10):
    #     print("{}: failed to connect.  exiting sample".format(client_id))
    #     sys.exit(1)

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
            sys.exit(1)
        # SUBSCRIBE
        topic = "vehicles/+/position"
        (_subscribe_result, subscribe_mid) = mqtt_client.subscribe("sample/+")
        print(f"Sending subscribe requestor topic \"{topic}\" with message id {subscribe_mid}")

        # WAIT FOR SUBSCRIBE
        if not wait_for_subscribed(timeout=10):
            print("{}: failed to subscribe.  exiting sample without publishing".format(client_id))
            sys.exit(1)

        # # PUBLISH
        # topic = "vehicles/{client_id}/position".format(client_id=client_id)
        # payload = Point(51.899523, -2.124156)
        # publish_result = mqtt_client.publish(topic, payload)
        # print(f"Sending publish with payload \"{payload}\" on topic \"{topic}\" with message id {publish_result.mid}")

        # # WAIT FOR PUBLISH
        # if not wait_for_published(timeout=10):
        #     print("{}: failed to publish.  exiting sample".format(client_id))
        #     sys.exit(1)

        # WAIT FOR MESSAGE RECEIVED
        while True:
        if not wait_for_receive(timeout=10):
            print("{}: failed to receive meessage.  exiting sample".format(client_id))
            sys.exit(1)


    except KeyboardInterrupt:
        print("User initiated exit")
    except Exception:
        print("Unexpected exception!")
        raise
    finally:
        print("Shutting down....")
        # DISCONNECT
        print("{}: Disconnecting".format(client_id))
        mqtt_client.disconnect()
        wait_for_disconnected(5)

if __name__ == "__main__":
    main()
