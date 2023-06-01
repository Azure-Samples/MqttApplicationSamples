
import sys

if sys.version_info >= (3, 11):
    from typing import Required
else:
    from typing_extensions import Required

if sys.version_info >= (3, 8):
    from typing import TypedDict
else:
    from typing_extensions import TypedDict

from typing import Optional, Final
import dotenv
import os


class ConnectionSettings(TypedDict, total=False):
    MQTT_HOST_NAME: Required[str]
    MQTT_TCP_PORT: Required[int]
    MQTT_USE_TLS: Required[bool]
    MQTT_CLEAN_SESSION: Required[bool]
    MQTT_KEEP_ALIVE_IN_SECONDS: Required[int]
    MQTT_CLIENT_ID: str
    MQTT_USERNAME: str
    MQTT_PASSWORD: str
    MQTT_CA_FILE: str
    MQTT_CA_PATH: str
    MQTT_CERT_FILE: str
    MQTT_KEY_FILE: str
    MQTT_KEY_FILE_PASSWORD: str


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


def _convert_to_int(value: str, name: str) -> int:
    try:
        return int(value)
    except ValueError:
        raise ValueError(f'{name} must be an integer')


def _convert_to_bool(value: str, name: str) -> bool:
    if value == 'true':
        return True
    elif value == 'false':
        return False
    else:
        raise ValueError(f'{name} must be true or false')


# Check if no file path is given how does this function behave?
# Like if u don't provide anything can it find the .env file?
def get_connection_settings(env_filename: Optional[str] = None) -> ConnectionSettings:
    env_file_dict = dotenv.dotenv_values(env_filename)
    # TODO: test envfile finding if filename is None
    envfile_values = {k: v for k, v in env_file_dict.items() if k in mqtt_setting_names}
    envvar_values = {k: v for k, v in os.environ.items() if k in mqtt_setting_names}
    default_values = {
        'MQTT_TCP_PORT': '8883',
        'MQTT_USE_TLS': 'true',
        'MQTT_CLEAN_SESSION': 'true',
        'MQTT_KEEP_ALIVE_IN_SECONDS': '30',
        'MQTT_CLIENT_ID': ''
    }
    
    final_values = {**default_values, **envvar_values, **envfile_values}

    if 'MQTT_HOST_NAME' not in final_values:
        raise ValueError('MQTT_HOST_NAME must be set')

    if 'MQTT_PASSWORD' in final_values and 'MQTT_USERNAME' not in final_values:
        raise ValueError('MQTT_USERNAME must be set if MQTT_PASSWORD is set')

    if 'MQTT_TCP_PORT' in final_values:
        final_values['MQTT_TCP_PORT'] = _convert_to_int(final_values['MQTT_TCP_PORT'], 'MQTT_TCP_PORT')

    if 'MQTT_USE_TLS' in final_values:
        final_values['MQTT_USE_TLS'] = _convert_to_bool(final_values['MQTT_USE_TLS'], 'MQTT_USE_TLS')

    if 'MQTT_CLEAN_SESSION' in final_values:
        final_values['MQTT_CLEAN_SESSION'] = _convert_to_bool(final_values['MQTT_CLEAN_SESSION'], 'MQTT_CLEAN_SESSION')

    if 'MQTT_KEEP_ALIVE_IN_SECONDS' in final_values:
        final_values['MQTT_KEEP_ALIVE_IN_SECONDS'] = _convert_to_int(final_values['MQTT_KEEP_ALIVE_IN_SECONDS'],
                                                                     'MQTT_KEEP_ALIVE_IN_SECONDS')

    return final_values
