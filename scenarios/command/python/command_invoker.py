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
import request_ledger

REQUEST_TOPIC_PATTERN = "vehicles/{targetClientId}/command/{commandName}/request"
RESPONSE_TOPIC_PATTERN = "vehicles/{targetClientId}/command/{commandName}/response"

parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

logging.basicConfig(level=logging.DEBUG)

connected_cond = threading.Condition()
connected_prop = False
connection_error = None
subscribed_cond = threading.Condition()
subscribed_prop = False

request_ledger = request_ledger.RequestLedger()

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

def on_unlock_response(_client, _userdata, message):
    print(f"Received message on topic {message.topic} with payload {message.payload}")
    properties = message.properties
    # # In Paho CB thread.
    # NOTE: probably should marshall this to another thread for safety, but it should be fine
    # because the lock used in the ledger shouldn't ever block in practice of this sample
    request_ledger.respond_to_request(properties.CorrelationData, message)

def on_disconnect(_client, _userdata, rc, _properties):
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
        protocol=mqtt.MQTTv5,
        transport="tcp",
    )
    if 'MQTT_USERNAME' in connection_settings:
        mqtt_client.username_pw_set(
            username=connection_settings['MQTT_USERNAME'],
            password=connection_settings['MQTT_PASSWORD'] if 'MQTT_PASSWORD' in connection_settings else None
        )
    if connection_settings['MQTT_USE_TLS']:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_CLIENT)
        context.minimum_version = ssl.TLSVersion.TLSv1_2
        context.maximum_version = ssl.TLSVersion.TLSv1_3

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

def send_unlock_command(mqtt_client, client_id, response_topic):
    request_topic = REQUEST_TOPIC_PATTERN.format(targetClientId="vehicle03", commandName="unlock")
    payload = "placeholder"
    correlation_id = str(uuid.uuid4()).encode()
    msg_prop = Properties(PacketTypes.PUBLISH)
    # NOTE: Pending investigation of Protobuf in Python, we are using MQTT Properties
    # Revisit this later.
    msg_prop.UserProperty = ("When", str(datetime.datetime.utcnow()))
    msg_prop.UserProperty = ("RequestedFrom", client_id)
    msg_prop.CorrelationData = correlation_id
    msg_prop.ResponseTopic = response_topic
    print("Sending Unlock Request")
    message_info = mqtt_client.publish(request_topic, payload, qos=1, properties=msg_prop)
    message_info.wait_for_publish(timeout=10)
    response_future = request_ledger.get_response_future(correlation_id)
    print("Waiting for Unlock Response")
    response_msg = response_future.result(timeout=10)
    print("Response: {}".format(response_msg.properties.UserProperty[0]))


def main():
    connection_settings = cs.get_connection_settings(args.env_file)
    if not connection_settings["MQTT_CLEAN_SESSION"]:
        raise ValueError("This sample does not support connecting with existing sessions")

    # INITIALIZE
    print("Initializing Paho MQTT client")
    response_topic = RESPONSE_TOPIC_PATTERN.format(targetClientId="vehicle03", commandName="unlock")
    client_id = connection_settings["MQTT_CLIENT_ID"]
    mqtt_client = create_mqtt_client(client_id, connection_settings)

    # ATTACH CONN/SUB HANDLERS
    mqtt_client.on_connect = on_connect
    mqtt_client.on_subscribe = on_subscribe
    mqtt_client.on_disconnect = on_disconnect
    mqtt_client.enable_logger()

    # CONNECT
    print("{}: Starting connection".format(client_id))
    mqtt_client.connect(
        host=connection_settings['MQTT_HOST_NAME'],
        port=connection_settings['MQTT_TCP_PORT'],
        keepalive=connection_settings["MQTT_KEEP_ALIVE_IN_SECONDS"],
        clean_start=connection_settings['MQTT_CLEAN_SESSION'],
    )
    print("Starting network loop")
    mqtt_client.loop_start()

    # WAIT FOR CONNECT
    if not wait_for_connected(timeout=10):
        print("{}: failed to connect.  exiting sample".format(client_id))
        raise TimeoutError("Timed out waiting for connect")

    try:
        # SUBSCRIBE TO COMMAND RESPONSES
        (_subscribe_result, subscribe_mid) = mqtt_client.subscribe(response_topic, qos=1)
        print(f"Sending subscribe requestor topic \"{response_topic}\" with message id {subscribe_mid}")

        # WAIT FOR SUBSCRIBE
        if not wait_for_subscribed(timeout=10):
            print("{}: failed to subscribe.  exiting sample without publishing".format(client_id))
            raise TimeoutError("Timed out waiting for subscribe")

        # # ATTACH COMMAND RESPONSE HANDLER
        mqtt_client.message_callback_add(response_topic, on_unlock_response)

        # PUBLISH COMMAND REQUESTS
        print("Beginning commands")
        while True:
            send_unlock_command(mqtt_client, client_id, response_topic)
            time.sleep(2)
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

if __name__ == '__main__':
    main()
