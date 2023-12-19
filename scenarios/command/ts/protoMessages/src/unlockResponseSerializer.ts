import { IMessageSerializer } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockResponse
} from './generated/unlock_command';

export class UnlockResponseSerializer implements IMessageSerializer {
    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return UnlockResponse.fromBinary(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        return Buffer.from(UnlockResponse.toBinary(payload as UnlockResponse));
    }
}