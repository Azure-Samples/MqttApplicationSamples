import {
    MqttClient
} from 'mqtt';
import {
    CommandClient
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
export const ResponseTopicPattern = 'vehicles/{clientId}/command/{commandName}/response';

export class UnlockCommandClient extends CommandClient<IUnlockRequest, IUnlockResponse>
{
    constructor(mqttClient: MqttClient) {
        super(mqttClient, RequestTopicPattern, ResponseTopicPattern, UnlockCommand, new ProtobufSerializer());
    }
}
