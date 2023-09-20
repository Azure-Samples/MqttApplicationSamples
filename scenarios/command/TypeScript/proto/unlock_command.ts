import type * as grpc from '@grpc/grpc-js';
import type { MessageTypeDefinition } from '@grpc/proto-loader';

import type { CommandsClient as _CommandsClient, CommandsDefinition as _CommandsDefinition } from './Commands';

type SubtypeConstructor<Constructor extends new (...args: any) => any, Subtype> = {
    new(...args: ConstructorParameters<Constructor>): Subtype;
};

export interface ProtoGrpcType {
    Commands: SubtypeConstructor<typeof grpc.Client, _CommandsClient> & { service: _CommandsDefinition }
    UnlockRequest: MessageTypeDefinition
    UnlockResponse: MessageTypeDefinition
    google: {
        protobuf: {
            Timestamp: MessageTypeDefinition
        }
    }
}

