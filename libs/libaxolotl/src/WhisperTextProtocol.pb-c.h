﻿/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: WhisperTextProtocol.proto */

#ifndef PROTOBUF_C_WhisperTextProtocol_2eproto__INCLUDED
#define PROTOBUF_C_WhisperTextProtocol_2eproto__INCLUDED

#include "protobuf-c/protobuf-c.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1001001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _Textsecure__SignalMessage Textsecure__SignalMessage;
typedef struct _Textsecure__PreKeySignalMessage Textsecure__PreKeySignalMessage;
typedef struct _Textsecure__KeyExchangeMessage Textsecure__KeyExchangeMessage;
typedef struct _Textsecure__SenderKeyMessage Textsecure__SenderKeyMessage;
typedef struct _Textsecure__SenderKeyDistributionMessage Textsecure__SenderKeyDistributionMessage;
typedef struct _Textsecure__DeviceConsistencyCodeMessage Textsecure__DeviceConsistencyCodeMessage;


/* --- enums --- */


/* --- messages --- */

struct  _Textsecure__SignalMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_ratchetkey;
  ProtobufCBinaryData ratchetkey;
  protobuf_c_boolean has_counter;
  uint32_t counter;
  protobuf_c_boolean has_previouscounter;
  uint32_t previouscounter;
  protobuf_c_boolean has_ciphertext;
  ProtobufCBinaryData ciphertext;
};
#define TEXTSECURE__SIGNAL_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__signal_message__descriptor) \
    , 0,{0,NULL}, 0,0, 0,0, 0,{0,NULL} }


struct  _Textsecure__PreKeySignalMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_registrationid;
  uint32_t registrationid;
  protobuf_c_boolean has_prekeyid;
  uint32_t prekeyid;
  protobuf_c_boolean has_signedprekeyid;
  uint32_t signedprekeyid;
  protobuf_c_boolean has_basekey;
  ProtobufCBinaryData basekey;
  protobuf_c_boolean has_identitykey;
  ProtobufCBinaryData identitykey;
  /*
   * SignalMessage
   */
  protobuf_c_boolean has_message;
  ProtobufCBinaryData message;
};
#define TEXTSECURE__PRE_KEY_SIGNAL_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__pre_key_signal_message__descriptor) \
    , 0,0, 0,0, 0,0, 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL} }


struct  _Textsecure__KeyExchangeMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_id;
  uint32_t id;
  protobuf_c_boolean has_basekey;
  ProtobufCBinaryData basekey;
  protobuf_c_boolean has_ratchetkey;
  ProtobufCBinaryData ratchetkey;
  protobuf_c_boolean has_identitykey;
  ProtobufCBinaryData identitykey;
  protobuf_c_boolean has_basekeysignature;
  ProtobufCBinaryData basekeysignature;
};
#define TEXTSECURE__KEY_EXCHANGE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__key_exchange_message__descriptor) \
    , 0,0, 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL}, 0,{0,NULL} }


struct  _Textsecure__SenderKeyMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_id;
  uint32_t id;
  protobuf_c_boolean has_iteration;
  uint32_t iteration;
  protobuf_c_boolean has_ciphertext;
  ProtobufCBinaryData ciphertext;
};
#define TEXTSECURE__SENDER_KEY_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__sender_key_message__descriptor) \
    , 0,0, 0,0, 0,{0,NULL} }


struct  _Textsecure__SenderKeyDistributionMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_id;
  uint32_t id;
  protobuf_c_boolean has_iteration;
  uint32_t iteration;
  protobuf_c_boolean has_chainkey;
  ProtobufCBinaryData chainkey;
  protobuf_c_boolean has_signingkey;
  ProtobufCBinaryData signingkey;
};
#define TEXTSECURE__SENDER_KEY_DISTRIBUTION_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__sender_key_distribution_message__descriptor) \
    , 0,0, 0,0, 0,{0,NULL}, 0,{0,NULL} }


struct  _Textsecure__DeviceConsistencyCodeMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_generation;
  uint32_t generation;
  protobuf_c_boolean has_signature;
  ProtobufCBinaryData signature;
};
#define TEXTSECURE__DEVICE_CONSISTENCY_CODE_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&textsecure__device_consistency_code_message__descriptor) \
    , 0,0, 0,{0,NULL} }


/* Textsecure__SignalMessage methods */
void   textsecure__signal_message__init
                     (Textsecure__SignalMessage         *message);
size_t textsecure__signal_message__get_packed_size
                     (const Textsecure__SignalMessage   *message);
size_t textsecure__signal_message__pack
                     (const Textsecure__SignalMessage   *message,
                      uint8_t             *out);
size_t textsecure__signal_message__pack_to_buffer
                     (const Textsecure__SignalMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__SignalMessage *
       textsecure__signal_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__signal_message__free_unpacked
                     (Textsecure__SignalMessage *message,
                      ProtobufCAllocator *allocator);
/* Textsecure__PreKeySignalMessage methods */
void   textsecure__pre_key_signal_message__init
                     (Textsecure__PreKeySignalMessage         *message);
size_t textsecure__pre_key_signal_message__get_packed_size
                     (const Textsecure__PreKeySignalMessage   *message);
size_t textsecure__pre_key_signal_message__pack
                     (const Textsecure__PreKeySignalMessage   *message,
                      uint8_t             *out);
size_t textsecure__pre_key_signal_message__pack_to_buffer
                     (const Textsecure__PreKeySignalMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__PreKeySignalMessage *
       textsecure__pre_key_signal_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__pre_key_signal_message__free_unpacked
                     (Textsecure__PreKeySignalMessage *message,
                      ProtobufCAllocator *allocator);
/* Textsecure__KeyExchangeMessage methods */
void   textsecure__key_exchange_message__init
                     (Textsecure__KeyExchangeMessage         *message);
size_t textsecure__key_exchange_message__get_packed_size
                     (const Textsecure__KeyExchangeMessage   *message);
size_t textsecure__key_exchange_message__pack
                     (const Textsecure__KeyExchangeMessage   *message,
                      uint8_t             *out);
size_t textsecure__key_exchange_message__pack_to_buffer
                     (const Textsecure__KeyExchangeMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__KeyExchangeMessage *
       textsecure__key_exchange_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__key_exchange_message__free_unpacked
                     (Textsecure__KeyExchangeMessage *message,
                      ProtobufCAllocator *allocator);
/* Textsecure__SenderKeyMessage methods */
void   textsecure__sender_key_message__init
                     (Textsecure__SenderKeyMessage         *message);
size_t textsecure__sender_key_message__get_packed_size
                     (const Textsecure__SenderKeyMessage   *message);
size_t textsecure__sender_key_message__pack
                     (const Textsecure__SenderKeyMessage   *message,
                      uint8_t             *out);
size_t textsecure__sender_key_message__pack_to_buffer
                     (const Textsecure__SenderKeyMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__SenderKeyMessage *
       textsecure__sender_key_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__sender_key_message__free_unpacked
                     (Textsecure__SenderKeyMessage *message,
                      ProtobufCAllocator *allocator);
/* Textsecure__SenderKeyDistributionMessage methods */
void   textsecure__sender_key_distribution_message__init
                     (Textsecure__SenderKeyDistributionMessage         *message);
size_t textsecure__sender_key_distribution_message__get_packed_size
                     (const Textsecure__SenderKeyDistributionMessage   *message);
size_t textsecure__sender_key_distribution_message__pack
                     (const Textsecure__SenderKeyDistributionMessage   *message,
                      uint8_t             *out);
size_t textsecure__sender_key_distribution_message__pack_to_buffer
                     (const Textsecure__SenderKeyDistributionMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__SenderKeyDistributionMessage *
       textsecure__sender_key_distribution_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__sender_key_distribution_message__free_unpacked
                     (Textsecure__SenderKeyDistributionMessage *message,
                      ProtobufCAllocator *allocator);
/* Textsecure__DeviceConsistencyCodeMessage methods */
void   textsecure__device_consistency_code_message__init
                     (Textsecure__DeviceConsistencyCodeMessage         *message);
size_t textsecure__device_consistency_code_message__get_packed_size
                     (const Textsecure__DeviceConsistencyCodeMessage   *message);
size_t textsecure__device_consistency_code_message__pack
                     (const Textsecure__DeviceConsistencyCodeMessage   *message,
                      uint8_t             *out);
size_t textsecure__device_consistency_code_message__pack_to_buffer
                     (const Textsecure__DeviceConsistencyCodeMessage   *message,
                      ProtobufCBuffer     *buffer);
Textsecure__DeviceConsistencyCodeMessage *
       textsecure__device_consistency_code_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   textsecure__device_consistency_code_message__free_unpacked
                     (Textsecure__DeviceConsistencyCodeMessage *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Textsecure__SignalMessage_Closure)
                 (const Textsecure__SignalMessage *message,
                  void *closure_data);
typedef void (*Textsecure__PreKeySignalMessage_Closure)
                 (const Textsecure__PreKeySignalMessage *message,
                  void *closure_data);
typedef void (*Textsecure__KeyExchangeMessage_Closure)
                 (const Textsecure__KeyExchangeMessage *message,
                  void *closure_data);
typedef void (*Textsecure__SenderKeyMessage_Closure)
                 (const Textsecure__SenderKeyMessage *message,
                  void *closure_data);
typedef void (*Textsecure__SenderKeyDistributionMessage_Closure)
                 (const Textsecure__SenderKeyDistributionMessage *message,
                  void *closure_data);
typedef void (*Textsecure__DeviceConsistencyCodeMessage_Closure)
                 (const Textsecure__DeviceConsistencyCodeMessage *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor textsecure__signal_message__descriptor;
extern const ProtobufCMessageDescriptor textsecure__pre_key_signal_message__descriptor;
extern const ProtobufCMessageDescriptor textsecure__key_exchange_message__descriptor;
extern const ProtobufCMessageDescriptor textsecure__sender_key_message__descriptor;
extern const ProtobufCMessageDescriptor textsecure__sender_key_distribution_message__descriptor;
extern const ProtobufCMessageDescriptor textsecure__device_consistency_code_message__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_WhisperTextProtocol_2eproto__INCLUDED */
