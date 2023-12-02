export class TelemetryMessage<T>
{
    public clientIdFromTopic: string;
    public payload: T;

    constructor(clientIdFromTopic: string, payload: T) {
        this.clientIdFromTopic = clientIdFromTopic;
        this.payload = payload;
    }
}
