# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import sys
from connectionsettings import connection_settings as cs
from argparse import ArgumentParser
import logging
import datetime
import time
import uuid
import ssl
import threading
import paho.mqtt.client as mqtt
from paho.mqtt.properties import Properties
from paho.mqtt.packettypes import PacketTypes
from concurrent.futures import ThreadPoolExecutor

REQUEST_TOPIC_PATTERN = "vehicles/{targetClientId}/command/{commandName}/request"
parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

tpe = ThreadPoolExecutor()

logging.basicConfig(level=logging.DEBUG)

connected_cond = threading.Condition()
connected_prop = False
connection_error = None
subscribed_cond = threading.Condition()
subscribed_prop = False

def on_connect(client, _userdata, _flags, rc, _properties):
    global connected_prop
    global connection_error
    print("Connected to MQTT broker")
    # # In Paho CB thread.
    with connected_cond:
        if rc == mqtt.MQTT_ERR_SUCCESS:
            connected_prop = True
        else:
            connection_error = Exception(mqtt.connack_string(rc))
        connected_cond.notify_all()

def on_subscribe(client, _userdata, mid, _reason_codes, _properties):
    global subscribed_prop
    print(f"Subscribe for message id {mid} acknowledged by MQTT broker")
    # # In Paho CB thread.
    with subscribed_cond:
        subscribed_prop = True
        subscribed_cond.notify_all()

def on_disconnect(_client, _userdata, rc, _properties):
    print("Received disconnect with error='{}'".format(mqtt.error_string(rc)))
    global connected_prop
    # # In Paho CB thread.
    with connected_cond:
        connected_prop = False
        connected_cond.notify_all()

def send_unlock_response(mqtt_client, correlation_id, response_topic):
    # NOTE: Pending investigation of Protobuf in Python, we are using MQTT Properties
    # Revisit this later.
    payload = "placeholder"
    msg_prop = Properties(PacketTypes.PUBLISH)
    msg_prop.UserProperty = ("Succeed", "True")
    msg_prop.CorrelationData = correlation_id
    print("Sending Unlock Response")
    message_info = mqtt_client.publish(response_topic, payload, qos=1, properties=msg_prop)
    message_info.wait_for_publish(timeout=10)


def on_unlock_command(_client, _userdata, message):
    print(f"Received unlock command")
    properties = message.properties
    correlation_id = properties.CorrelationData
    response_topic = properties.ResponseTopic
    # Respond to the Unlock on a different thread so as not to block network loop
    tpe.submit(send_unlock_response, _client, correlation_id, response_topic)

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
        protocol=mqtt.MQTTv5,
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

    # ATTACH CONN/SUB HANDLERS
    mqtt_client.on_connect = on_connect
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.enable_logger()

    # CONNECT
    print("{}: Starting connection".format(client_id))
    hostname = connection_settings['MQTT_HOST_NAME']
    port = connection_settings['MQTT_TCP_PORT']
    keepalive = connection_settings["MQTT_KEEP_ALIVE_IN_SECONDS"]
    mqtt_client.connect(hostname, port, keepalive, clean_start=connection_settings['MQTT_CLEAN_SESSION'])
    print("Starting network loop")
    mqtt_client.loop_start()

    # WAIT FOR CONNECT
    if not wait_for_connected(timeout=10):
        print("{}: failed to connect.  exiting sample".format(client_id))
        raise TimeoutError("Timed out waiting for connect")

    try:
        # SUBSCRIBE TO COMMAND REQUESTS
        request_topic = REQUEST_TOPIC_PATTERN.format(targetClientId="vehicle03", commandName="unlock")
        (_subscribe_result, subscribe_mid) = mqtt_client.subscribe(request_topic, qos=1)
        print(f"Sending subscribe requestor topic \"{request_topic}\" with message id {subscribe_mid}")

        # WAIT FOR SUBSCRIBE
        if not wait_for_subscribed(timeout=10):
            print("{}: failed to subscribe.  exiting sample without publishing".format(client_id))
            raise TimeoutError("Timed out waiting for subscribe")
        
        # ATTACH COMMAND REQUEST HANDLER
        mqtt_client.message_callback_add(request_topic, on_unlock_command)

        
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("User exit")
    except Exception:
        print("Unexpected Exception")
        raise
    finally:
        # DISCONNECT
        print("{}: Disconnecting".format(client_id))
        mqtt_client.disconnect()
        wait_for_disconnected(5)
        # NOTE: lifecycle of TPE isn't quite right - if the conn fails, it won't shut down due to error structure here.
        # Will fix later.
        tpe.shutdown()

if __name__ == '__main__':
    main()