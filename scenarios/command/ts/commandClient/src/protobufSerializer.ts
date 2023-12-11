
import {
    IMessageSerializer
} from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    IUnlockRequest,
    UnlockRequest
} from '@mqttapplicationsamples/protomessages';

export class ProtobufSerializer implements IMessageSerializer {
    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return UnlockRequest.decode(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        return Buffer.from(UnlockRequest.encode(payload as IUnlockRequest).finish());
    }
}
