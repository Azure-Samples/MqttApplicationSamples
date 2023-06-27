/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: unlock_command.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "unlock_command.pb-c.h"
void   unlock_request__init
                     (UnlockRequest         *message)
{
  static const UnlockRequest init_value = UNLOCK_REQUEST__INIT;
  *message = init_value;
}
size_t unlock_request__get_packed_size
                     (const UnlockRequest *message)
{
  assert(message->base.descriptor == &unlock_request__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t unlock_request__pack
                     (const UnlockRequest *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &unlock_request__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t unlock_request__pack_to_buffer
                     (const UnlockRequest *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &unlock_request__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
UnlockRequest *
       unlock_request__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (UnlockRequest *)
     protobuf_c_message_unpack (&unlock_request__descriptor,
                                allocator, len, data);
}
void   unlock_request__free_unpacked
                     (UnlockRequest *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &unlock_request__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   unlock_response__init
                     (UnlockResponse         *message)
{
  static const UnlockResponse init_value = UNLOCK_RESPONSE__INIT;
  *message = init_value;
}
size_t unlock_response__get_packed_size
                     (const UnlockResponse *message)
{
  assert(message->base.descriptor == &unlock_response__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t unlock_response__pack
                     (const UnlockResponse *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &unlock_response__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t unlock_response__pack_to_buffer
                     (const UnlockResponse *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &unlock_response__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
UnlockResponse *
       unlock_response__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (UnlockResponse *)
     protobuf_c_message_unpack (&unlock_response__descriptor,
                                allocator, len, data);
}
void   unlock_response__free_unpacked
                     (UnlockResponse *message,
                      ProtobufCAllocator *allocator)
{
  if(!message)
    return;
  assert(message->base.descriptor == &unlock_response__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor unlock_request__field_descriptors[2] =
{
  {
    "when",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_MESSAGE,
    0,   /* quantifier_offset */
    offsetof(UnlockRequest, when),
    &google__protobuf__timestamp__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "requestedFrom",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(UnlockRequest, requestedfrom),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned unlock_request__field_indices_by_name[] = {
  1,   /* field[1] = requestedFrom */
  0,   /* field[0] = when */
};
static const ProtobufCIntRange unlock_request__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor unlock_request__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "unlockRequest",
  "UnlockRequest",
  "UnlockRequest",
  "",
  sizeof(UnlockRequest),
  2,
  unlock_request__field_descriptors,
  unlock_request__field_indices_by_name,
  1,  unlock_request__number_ranges,
  (ProtobufCMessageInit) unlock_request__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCFieldDescriptor unlock_response__field_descriptors[2] =
{
  {
    "succeed",
    1,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_BOOL,
    0,   /* quantifier_offset */
    offsetof(UnlockResponse, succeed),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "errorDetail",
    2,
    PROTOBUF_C_LABEL_NONE,
    PROTOBUF_C_TYPE_STRING,
    0,   /* quantifier_offset */
    offsetof(UnlockResponse, errordetail),
    NULL,
    &protobuf_c_empty_string,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned unlock_response__field_indices_by_name[] = {
  1,   /* field[1] = errorDetail */
  0,   /* field[0] = succeed */
};
static const ProtobufCIntRange unlock_response__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 2 }
};
const ProtobufCMessageDescriptor unlock_response__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "unlockResponse",
  "UnlockResponse",
  "UnlockResponse",
  "",
  sizeof(UnlockResponse),
  2,
  unlock_response__field_descriptors,
  unlock_response__field_indices_by_name,
  1,  unlock_response__number_ranges,
  (ProtobufCMessageInit) unlock_response__init,
  NULL,NULL,NULL    /* reserved[123] */
};
static const ProtobufCMethodDescriptor commands__method_descriptors[1] =
{
  { "unlock", &unlock_request__descriptor, &unlock_response__descriptor },
};
const unsigned commands__method_indices_by_name[] = {
  0         /* unlock */
};
const ProtobufCServiceDescriptor commands__descriptor =
{
  PROTOBUF_C__SERVICE_DESCRIPTOR_MAGIC,
  "Commands",
  "Commands",
  "Commands",
  "",
  1,
  commands__method_descriptors,
  commands__method_indices_by_name
};
void commands__unlock(ProtobufCService *service,
                      const UnlockRequest *input,
                      UnlockResponse_Closure closure,
                      void *closure_data)
{
  assert(service->descriptor == &commands__descriptor);
  service->invoke(service, 0, (const ProtobufCMessage *) input, (ProtobufCClosure) closure, closure_data);
}
void commands__init (Commands_Service *service,
                     Commands_ServiceDestroy destroy)
{
  protobuf_c_service_generated_init (&service->base,
                                     &commands__descriptor,
                                     (ProtobufCServiceDestroy) destroy);
}