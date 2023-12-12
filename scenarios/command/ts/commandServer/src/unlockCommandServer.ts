import {
    MqttClient
} from 'mqtt';
import {
    CommandServer
} from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    pb,
    ProtobufSerializer
} from '@mqttapplicationsamples/protomessages';

export const UnlockCommand = 'unlock';
export const RequestTopicPattern = 'vehicles/{clientId}/command/{commandName}/request';

export class UnlockCommandServer extends CommandServer<pb.UnlockRequest, pb.UnlockResponse>
{
    constructor(mqttClient: MqttClient) {
        super(mqttClient, RequestTopicPattern, UnlockCommand, new ProtobufSerializer(pb.UnlockRequest));
    }
}
