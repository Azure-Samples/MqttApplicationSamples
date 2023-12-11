import { IMessageSerializer } from ".";

export class Utf8JsonSerializer<T> implements IMessageSerializer {
    public contentType = 'application/json';

    public fromBytes<T>(payload: Buffer): T {
        return JSON.parse(payload.toString('utf8')) as T;
    }

    public toBytes<T>(payload: T): Buffer {
        return Buffer.from(JSON.stringify(payload), 'utf8');
    }
}
