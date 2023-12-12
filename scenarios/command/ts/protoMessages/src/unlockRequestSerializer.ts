import { IMessageSerializer } from '@mqttapplicationsamples/mqttjsclientextensions';
import {
    UnlockRequest
} from './generated/src/proto/unlock_command';

export class UnlockRequestSerializer implements IMessageSerializer {
    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return UnlockRequest.decode(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        const unlockRequest = UnlockRequest.fromJSON(payload);
        const unlockRequestBytes = UnlockRequest.encode(unlockRequest).finish();

        return Buffer.from(unlockRequestBytes);
    }
}
