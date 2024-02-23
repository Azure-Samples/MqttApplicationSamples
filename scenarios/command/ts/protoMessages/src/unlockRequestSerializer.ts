import { IMessageSerializer } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockRequest
} from './generated/unlock_command';

export class UnlockRequestSerializer implements IMessageSerializer {
    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return UnlockRequest.fromBinary(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        return Buffer.from(UnlockRequest.toBinary(payload as UnlockRequest));
    }
}