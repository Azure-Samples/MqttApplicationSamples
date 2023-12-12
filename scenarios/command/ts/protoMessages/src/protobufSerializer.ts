import {
    IMessageSerializer
} from '@mqttapplicationsamples/mqttjsclientextensions';

export class ProtobufSerializer implements IMessageSerializer {
    private protoObject: any;

    constructor(protoObject: any) {
        this.protoObject = protoObject;
    }

    public contentType = "application/protobuf";

    public fromBytes<T>(payload: Buffer): T {
        return this.protoObject.decode(payload) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        return Buffer.from(this.protoObject.encode(payload).finish());
    }
}
