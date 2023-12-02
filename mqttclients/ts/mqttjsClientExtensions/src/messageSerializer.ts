export interface IMessageSerializer {
    contentType: string;
    toBytes<T>(payload: T): Buffer;
    fromBytes<T>(bytes: Buffer): T;
}