import { IMessageSerializer } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockResponse
} from './generated/src/proto/unlock_command';

export class UnlockResponseSerializer implements IMessageSerializer {
    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return UnlockResponse.decode(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        const unlockResponse = UnlockResponse.fromJSON(payload);
        const unlockResponseBytes = UnlockResponse.encode(unlockResponse).finish();

        return Buffer.from(unlockResponseBytes);
    }
}
