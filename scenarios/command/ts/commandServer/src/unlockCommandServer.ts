import { MqttClient } from 'mqtt';
import { CommandServer } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockRequest,
    UnlockResponse,
    UnlockRequestSerializer,
    UnlockResponseSerializer
} from '@mqttapplicationsamples/protomessages';

export const UnlockCommand = 'unlock';
export const RequestTopicPattern = 'vehicles/{clientId}/command/{commandName}/request';

export class UnlockCommandServer extends CommandServer<UnlockRequest, UnlockResponse>
{
    constructor(mqttClient: MqttClient) {
        super(mqttClient, RequestTopicPattern, UnlockCommand, new UnlockRequestSerializer(), new UnlockResponseSerializer());
    }
}
