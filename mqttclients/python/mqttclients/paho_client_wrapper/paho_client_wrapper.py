# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License. See License.txt in the project root for
# license information.
import logging
import os
import ssl
from typing import Optional, Final
import dotenv
from paho.mqtt import client as mqtt
from .connection_settings import ConnectionSettings, get_connection_settings
# import connection_settings as cs
# from connection_settings import ConnectionSettings, get_connection_settings
from .mqtt_helpers import IncomingMessageList, IncomingAckList, ConnectionStatus
from typing import Any, Tuple, List, TypeVar, Optional

# from typing import type

logger = logging.getLogger(__name__)

TPahoClientWrapper = TypeVar('TPahoClientWrapper', bound='PahoClientWrapper')

mqtt_setting_names: Final[list[str]] = [
    'MQTT_HOST_NAME',
    'MQTT_TCP_PORT',
    'MQTT_USE_TLS',
    'MQTT_CLEAN_SESSION',
    'MQTT_KEEP_ALIVE_IN_SECONDS',
    'MQTT_CLIENT_ID',
    'MQTT_USERNAME',
    'MQTT_PASSWORD',
    'MQTT_CA_FILE',
    'MQTT_CA_PATH',
    'MQTT_CERT_FILE',
    'MQTT_KEY_FILE',
    'MQTT_KEY_FILE_PASSWORD'
]


class PahoClientWrapper(object):
    def __init__(self, connection_settings: ConnectionSettings) -> None:
        self.device_id = connection_settings['MQTT_CLIENT_ID']
        self.mqtt_client = mqtt.Client(
            client_id=self.client_id,
            clean_session=connection_settings['MQTT_CLEAN_SESSION']
        )
        self.mqtt_client.enable_logger()
        if 'MQTT_USERNAME' in connection_settings:
            self.mqtt_client.username_pw_set(
                username=connection_settings['MQTT_USERNAME'],
                password=connection_settings['MQTT_PASSWORD'] if 'MQTT_PASSWORD' in connection_settings else None
            )

        self.hostname = connection_settings['MQTT_HOST_NAME']
        self.port = connection_settings['MQTT_TCP_PORT']

        if connection_settings['MQTT_USE_TLS']:
            # pass # TODO: implement TLS
            context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)

            # # load client cert 
            # # TODO: call only if provided the setting
            if connection_settings['MQTT_CERT_FILE']:
                context.load_cert_chain(
                    certfile=connection_settings['MQTT_CERT_FILE'],
                    keyfile=connection_settings['MQTT_KEY_FILE'],
                    password=connection_settings['MQTT_KEY_FILE_PASSWORD']
                )

            # # load trusted certs
            # # TODO: call only if user provided the setting
            # TODO Does the ca file or capath work? We have always used cadata
            # try:
            #     with open("<trusted_and_inter_cert>", mode="r") as ca_cert_file:
            #         self.server_verification_cert = ca_cert_file.read()
            #         logger.info(self.server_verification_cert)
            # except FileNotFoundError:
            #     raise
            # except OSError as e:
            #     raise ValueError("Invalid CA certificate file") from e
            if connection_settings['MQTT_CA_FILE']:
                context.load_verify_locations(
                    cafile=connection_settings['MQTT_CA_FILE'],
                )
            elif connection_settings['MQTT_CA_PATH']:
                context.load_verify_locations(
                    capath=connection_settings['MQTT_CA_PATH']
                )
            else:
                context.load_default_certs()

            self.mqtt_client.tls_set_context(context)

        self.mqtt_client.on_connect = self._handle_on_connect
        self.mqtt_client.on_disconnect = self._handle_on_disconnect
        self.mqtt_client.on_subscribe = self._handle_on_subscribe
        self.mqtt_client.on_publish = self._handle_on_publish
        self.mqtt_client.on_message = self._handle_on_message

        self.connection_status = ConnectionStatus()
        # A list of the results for each subscription in the request - either the granted qos, or -1 on failure
        self.incoming_subacks = IncomingAckList[List[int]]()
        # All other acks just return the packet id
        self.incoming_unsubacks = IncomingAckList[int]()
        self.incoming_pubacks = IncomingAckList[int]()
        self.incoming_messages = IncomingMessageList()

    @classmethod
    def create_from_connection_settings(cls: type[TPahoClientWrapper], conn_sett:ConnectionSettings) -> TPahoClientWrapper:
        # TODO: test envfile finding if filename is None
        return cls(conn_sett)

    @staticmethod
    def error_string(mqtt_error: int) -> str:
        return mqtt.error_string(mqtt_error)

    def _handle_on_connect(
            self, mqtt_client: mqtt.Client, userdata: Any, flags: Any, rc: int
    ) -> None:
        """
        event handler for Paho on_connect events.  Do not call directly.
        """
        logger.info(
            "_handle_on_connect called with status='{}'".format(mqtt.connack_string(rc))
        )

        # In Paho thread.  Save what we need and return.
        if rc == mqtt.MQTT_ERR_SUCCESS:
            # causes code waiting in `self.connection_status.wait_for_connected` to return
            self.connection_status.connected = True
        else:
            # causes code waiting in `self.connection_status.wait_for_connected` to raise this exception
            # causes code waiting in `self.connection_status.wait_for_disconnected` to return
            self.connection_status.connection_error = Exception(mqtt.connack_string(rc))

    def _handle_on_disconnect(
            self, client: mqtt.Client, userdata: Any, rc: int
    ) -> None:
        """
        event handler for Paho on_disconnect events.  Do not call directly.
        """
        # In Paho thread.  Save what we need and return.
        logger.info(
            "_handle_on_disconnect called with error='{}'".format(mqtt.error_string(rc))
        )
        # causes code waiting in `self.connection_status.wait_for_disconnected` to raise this exception
        self.connection_status.connected = False

    def _handle_on_subscribe(
            self,
            client: mqtt.Client,
            userdata: Any,
            mid: int,
            granted_qos: Tuple[int, ...],  # tuple of ints
            properties: Any = None,
    ) -> None:
        """
        event handler for Paho on_subscribe events.  Do not call directly.
        """
        # In Paho thread.  Save what we need and return.
        logger.info(
            "Received SUBACK for mid {}, granted_qos {}".format(mid, granted_qos)
        )
        granted_qos_list = list(granted_qos)
        # causes code waiting for this mid via `self.incoming_subacks.wait_for_ack` to return
        for i in range(len(granted_qos_list)):
            if granted_qos_list[i] == 128:
                # Use -1 to make it easier for clients
                granted_qos_list[i] = -1

        self.incoming_subacks.add_ack(mid, granted_qos_list)

    def _handle_on_unsubscribe(
            self, client: mqtt.Client, userdata: Any, mid: int
    ) -> None:
        """
        event handler for Paho on_unsubscribe events.  Do not call directly.
        """
        # In Paho thread.  Save what we need and return.
        logger.info("Received UNSUBACK for mid {}".format(mid))
        # causes code waiting for this mid via `self.incoming_unsubacks.wait_for_ack` to return
        self.incoming_unsubacks.add_ack(mid, mid)

    def _handle_on_publish(self, client: mqtt.Client, userdata: Any, mid: int) -> None:
        """
        event handler for Paho on_publish events.  Do not call directly.
        """
        # In Paho thread.  Save what we need and return.
        logger.info("Received PUBACK for mid {}".format(mid))
        # causes code waiting for this mid via `self.incoming_pubacks.wait_for_ack` to return
        self.incoming_pubacks.add_ack(mid, mid)

    def _handle_on_message(
            self, mqtt_client: mqtt.Client, userdata: Any, message: mqtt.MQTTMessage
    ) -> None:
        """
        event handler for Paho on_message events.  Do not call directly.
        """
        # In Paho thread.  Save what we need and return.
        logger.info("received message on {}".format(message.topic))
        # causes code waiting for messages via `self.incoming_messages.wait_for_message` and
        # `self.incoming_messages.pop_next_message` to return.
        self.incoming_messages.add_message(message)

    def start_connect(self) -> None:
        """
        Start connecting to the server.  Returns after the CONNECT packet has been sent.
        Connection isn't established until `self._handle_on_connect` has been called and
        `self.connection_status.connected` is `True`.
        """
        self.mqtt_client.connect(self.hostname, self.port)
        self.mqtt_client.loop_start()

    def disconnect(self) -> None:
        """
        Disconnect from the server.  Disconnection is likely complete after this function
        returns, but it is more reliable to wait for `self.connection_status.connected` to be
        set to `False`.
        """
        self.mqtt_client.disconnect()

    def publish(
            self, topic: str, payload: Any = None, qos: int = 0, retain: bool = False
    ) -> Tuple[int, int]:
        return self.mqtt_client.publish(topic, payload, qos, retain)  # type: ignore

    def subscribe(self, topic: str, qos: int = 0) -> Tuple[int, int]:
        return self.mqtt_client.subscribe(topic, qos)  # type: ignore

    def unsubscribe(self, topic: str) -> Tuple[int, int]:
        return self.mqtt_client.unsubscribe(topic)

    def client_id(self) -> str:
        return self.auth.client_id
