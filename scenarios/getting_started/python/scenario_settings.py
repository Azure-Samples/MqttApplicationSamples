from dotenv import dotenv_values
from os import getenv
from collections import defaultdict

def _convert_to_bool(value, name):
        if value is None:
            return None
        if value == "true":
            return True
        if value == "false":
            return False
        raise ValueError(f"{name} must be true or false")

def _convert_to_int(value, name):
    if value is None:
        return None
    try:
        return int(value)
    except ValueError:
        raise ValueError(f"{name} must be an integer")

def get_settings(envfile=None):
    default_settings = {
        "MQTT_TCP_PORT": "8883",
        "MQTT_USE_TLS": "true",
        "MQTT_KEEP_ALIVE_IN_SECONDS": "30",
    }
    envvar_settings = {
        "MQTT_HOST_NAME": getenv("MQTT_HOST_NAME"),
        "MQTT_TCP_PORT": getenv("MQTT_TCP_PORT"),
        "MQTT_USE_TLS": getenv("MQTT_USE_TLS"),
        "MQTT_CLEAN_SESSION": getenv("MQTT_CLEAN_SESSION"),
        "MQTT_KEEP_ALIVE_IN_SECONDS": getenv("MQTT_KEEP_ALIVE_IN_SECONDS"),
        "MQTT_CLIENT_ID": getenv("MQTT_CLIENT_ID"), 
        "MQTT_USER_NAME": getenv("MQTT_USER_NAME"),
        "MQTT_PASSWORD": getenv("MQTT_PASSWORD"),
        "MQTT_CA_FILE": getenv("MQTT_CA_FILE"),
        "MQTT_CA_PATH": getenv("MQTT_CA_PATH"),
        "MQTT_CERT_FILE": getenv("MQTT_CERT_FILE"),
        "MQTT_KEY_FILE": getenv("MQTT_KEY_FILE"),
        "MQTT_KEY_FILE_PASSWORD": getenv("MQTT_KEY_FILE_PASSWORD"),
    }
    settings = defaultdict(None, { **default_settings, **envvar_settings, **dotenv_values(envfile) })
    
    settings["MQTT_TCP_PORT"] = _convert_to_int(settings["MQTT_TCP_PORT"], "MQTT_TCP_PORT")
    settings["MQTT_USE_TLS"] = _convert_to_bool(settings["MQTT_USE_TLS"], "MQTT_USE_TLS")
    settings["MQTT_CLEAN_SESSION"] = _convert_to_bool(settings["MQTT_CLEAN_SESSION"], "MQTT_CLEAN_SESSION")
    settings["MQTT_KEEP_ALIVE_IN_SECONDS"] = _convert_to_int(settings["MQTT_KEEP_ALIVE_IN_SECONDS"], "MQTT_KEEP_ALIVE_IN_SECONDS")
    if settings["MQTT_HOST_NAME"] is None:
        raise ValueError("MQTT_HOST_NAME is required")
    if settings["MQTT_CA_PATH"] is not None:
        # TODO: verify what I said here is correct
        # Paho client doesn't directly support CA paths but we could manually create an SSLContext and pass it in if we wanted to support it
        raise ValueError("MQTT_CA_PATH is not supported")
    if settings["MQTT_PASSWORD"] is not None and settings["MQTT_USER_NAME"] is None:
        # MQTT 5 supports password without username, but the Paho client doesn't
        raise ValueError("MQTT_USER_NAME is required when MQTT_PASSWORD is set")

    return settings


    
