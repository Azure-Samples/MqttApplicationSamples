import paho.mqtt.client as mqtt
import ssl

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
        elif "MQTT_CA_PATH" in connection_settings:
                context.load_verify_locations(
                    capath=connection_settings['MQTT_CA_PATH']
            )
        else:
            context.load_default_certs()

        mqtt_client.tls_set_context(context)
    return mqtt_client