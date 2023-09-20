// Original file: proto/unlock_command.proto

import type { Timestamp as _google_protobuf_Timestamp, Timestamp__Output as _google_protobuf_Timestamp__Output } from './google/protobuf/Timestamp';

export interface UnlockRequest {
    'when'?: (_google_protobuf_Timestamp | null);
    'requestedFrom'?: (string);
}

export interface UnlockRequest__Output {
    'when': (_google_protobuf_Timestamp__Output | null);
    'requestedFrom': (string);
}
