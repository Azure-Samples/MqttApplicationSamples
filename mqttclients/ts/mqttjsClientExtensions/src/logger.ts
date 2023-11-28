import Pino from 'pino';

export const logger = Pino({
    transport: {
        target: 'pino-pretty',
        options: {
            colorize: true,
            messageFormat: '[{tags}] {msg}',
            translateTime: 'SYS:yyyy-mm-dd"T"HH:MM:sso',
            ignore: 'pid,hostname,tags,msg'
        }
    },
    serializers: {
        tags: (tags: string[]) => {
            return `${tags}`;
        }
    }
});
