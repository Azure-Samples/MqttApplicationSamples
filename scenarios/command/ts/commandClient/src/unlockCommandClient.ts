import { MqttClient } from 'mqtt';
import { CommandClient } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockRequest,
    UnlockResponse,
    UnlockRequestSerializer,
    UnlockResponseSerializer
} from '@mqttapplicationsamples/protomessages';

export const UnlockCommand = 'unlock';
export const RequestTopicPattern = 'vehicles/{clientId}/command/{commandName}/request';
export const ResponseTopicPattern = 'vehicles/{clientId}/command/{commandName}/response';

export class UnlockCommandClient extends CommandClient<UnlockRequest, UnlockResponse>
{
    constructor(mqttClient: MqttClient) {
        super(mqttClient, RequestTopicPattern, ResponseTopicPattern, UnlockCommand, new UnlockRequestSerializer(), new UnlockResponseSerializer());
    }
}
