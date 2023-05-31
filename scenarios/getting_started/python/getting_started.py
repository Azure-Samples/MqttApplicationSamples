# import paho.mqtt.client as mqtt
# import scenario_settings
# from argparse import ArgumentParser

# def on_connect(client, _userdata, _flags, rc):
#     if rc != 0:
#         print(f"Failed to connect to MQTT broker with return code {rc}")
#         return
    
#     print("Connected to MQTT broker")
#     topic = "sample/+"
#     (_subscribe_result, subscribe_mid) = client.subscribe("sample/+")
#     print(f"Sending subscribe requestor topic \"{topic}\" with message id {subscribe_mid}")

# def on_subscribe(client, _userdata, mid, _granted_qos):    
#     print(f"Subscribe for message id {mid} acknowledged by MQTT broker")
#     topic = "sample/topic1"
#     payload = "hello world!"
#     publish_result = client.publish(topic, payload)
#     print(f"Sending publish with payload \"{payload}\" on topic \"{topic}\" with message id {publish_result.mid}")

# def on_publish(_client, _userdata, mid):
#     print(f"Sent publish with message id {mid}") 

    
# def on_message(_client, _userdata, message):
#     print(f"Received message on topic {message.topic} with payload {message.payload}")

# parser = ArgumentParser()
# parser.add_argument("--env-file", help="path to the .env file to use")
# args = parser.parse_args()

# settings = scenario_settings.get_settings(args.env_file)

# if settings["MQTT_CLEAN_SESSION"] == False:
#     raise ValueError("This sample does not support connecting with existing sessions")

# print("Initializing Paho MQTT client")
# client = mqtt.Client(settings["MQTT_CLIENT_ID"])

# if settings["MQTT_USERNAME"] is not None:
#     client.username_pw_set(settings["MQTT_USERNAME"], settings["MQTT_PASSWORD"])

# if settings["MQTT_USE_TLS"]:
#     client.tls_set(
#         ca_certs=settings["MQTT_CA_FILE"],
#         certfile=settings["MQTT_CERT_FILE"],
#         keyfile=settings["MQTT_KEY_FILE"],
#         keyfile_password=settings["MQTT_KEY_FILE_PASSWORD"],
#     )

# client.on_connect = on_connect
# client.on_message = on_message
# client.on_publish = on_publish
# client.on_subscribe = on_subscribe

# client.connect(
#     host=settings["MQTT_HOST_NAME"],
#     port=settings["MQTT_TCP_PORT"],
#     keepalive=settings["MQTT_KEEP_ALIVE_IN_SECONDS"]
# )

# print("Starting network loop")
# client.loop_forever()

import sys
# import mqttclients
# # import mqttclients.python
from mqttclients import paho_client_wrapper as pc
# # import mqttclients
# from mqttclients.paho_client_wrapper.connection_settings as conn_sett
# import mqttclients.python.paho_client_wrapper as pc
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("--env-file", help="path to the .env file to use")
args = parser.parse_args()

connection_settings = pc.connection_settings.get_connection_settings(args.env_file)
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

# PUBLISH
(rc, mid) = paho_client.publish("sample/topic1", "hello world")
print("{}: Publish returned rc={}: {}".format(paho_client.device_id, rc, paho_client.error_string(rc)))

# DISCONNECT
print("{}: Disconnecting".format(paho_client.auth.device_id))
paho_client.disconnect()
paho_client.connection_status.wait_for_disconnected()

