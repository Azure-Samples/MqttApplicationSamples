import {
    MqttClient
} from 'mqtt';
import {
    CommandServer
} from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    IUnlockRequest,
    IUnlockResponse
} from '@mqttapplicationsamples/protomessages';
import {
    ProtobufSerializer
} from './protobufSerializer';

export const UnlockCommand = 'unlock';
export const RequestTopicPattern = 'vehicles/{clientId}/command/{commandName}/request';

export class UnlockCommandServer extends CommandServer<IUnlockRequest, IUnlockResponse>
{
    constructor(mqttClient: MqttClient) {
        super(mqttClient, RequestTopicPattern, UnlockCommand, new ProtobufSerializer());
    }
}
