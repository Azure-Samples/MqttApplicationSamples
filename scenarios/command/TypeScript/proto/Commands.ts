// Original file: proto/unlock_command.proto

import type * as grpc from '@grpc/grpc-js'
import type { MethodDefinition } from '@grpc/proto-loader'
import type { UnlockRequest as _UnlockRequest, UnlockRequest__Output as _UnlockRequest__Output } from './UnlockRequest';
import type { UnlockResponse as _UnlockResponse, UnlockResponse__Output as _UnlockResponse__Output } from './UnlockResponse';

export interface CommandsClient extends grpc.Client {
    Unlock(argument: _UnlockRequest, metadata: grpc.Metadata, options: grpc.CallOptions, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    Unlock(argument: _UnlockRequest, metadata: grpc.Metadata, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    Unlock(argument: _UnlockRequest, options: grpc.CallOptions, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    Unlock(argument: _UnlockRequest, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    unlock(argument: _UnlockRequest, metadata: grpc.Metadata, options: grpc.CallOptions, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    unlock(argument: _UnlockRequest, metadata: grpc.Metadata, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    unlock(argument: _UnlockRequest, options: grpc.CallOptions, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;
    unlock(argument: _UnlockRequest, callback: grpc.requestCallback<_UnlockResponse__Output>): grpc.ClientUnaryCall;

}

export interface CommandsHandlers extends grpc.UntypedServiceImplementation {
    Unlock: grpc.handleUnaryCall<_UnlockRequest__Output, _UnlockResponse>;

}

export interface CommandsDefinition extends grpc.ServiceDefinition {
    Unlock: MethodDefinition<_UnlockRequest, _UnlockResponse, _UnlockRequest__Output, _UnlockResponse__Output>
}
