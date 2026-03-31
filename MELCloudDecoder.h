#ifndef MELCLOUDDECODER_h
#define MELCLOUDDECODER_h

#include <stdint.h>
#include <time.h>
#include <string.h>
#include "EcodanDecoder.h"  // To get some of the definitions


typedef struct _MelCloudMessgeStruct {
  uint8_t SyncByte;
  uint8_t PacketType;
  uint8_t Preamble[PREAMBLESIZE];
  uint8_t PayloadSize;
  uint8_t Payload[MAXDATABLOCKSIZE];
  uint8_t Checksum;
} MelCloudMessgeStruct;

typedef struct _MelCloudStatus {
  uint8_t ReplyNow, ActiveMessage, Write_To_Melcloud_OK;
  uint8_t ConnectRequest;
  bool MEL_Online = false;
} MelCloudStatus;


class MELCLOUDDECODER {
public:
  MELCLOUDDECODER(void);
  uint8_t Process(uint8_t c);

  void CreateBlankTxMessage(uint8_t PacketType, uint8_t PayloadSize);
  void SetPayloadByte(uint8_t Data, uint8_t Location);
  uint8_t PrepareTxCommand(uint8_t *Buffer);

  MelCloudStatus Status;
protected:

private:
  MessageStruct RxMessage;
  MessageStruct TxMessage;



  uint8_t Preamble[PREAMBLESIZE];

  uint8_t BuildRxMessage(MessageStruct *Message, uint8_t c);

  void CreateBlankMessageTemplate(MessageStruct *Message, uint8_t PacketType, uint8_t PayloadSize);
  uint8_t PrepareCommand(MessageStruct *Message, uint8_t *Buffer);

  uint8_t CheckSum(uint8_t *Buffer, uint8_t len);

  void Process0x5A(uint8_t *Payload, MelCloudStatus *Status);
  void Process0x4C(uint8_t *Payload, MelCloudStatus *Status);
  void Process0x49(uint8_t *Payload, MelCloudStatus *Status);
  void Process0x4A(uint8_t *Payload, MelCloudStatus *Status);
  void Process0x4B(uint8_t *Payload, MelCloudStatus *Status);
  void Process0x48(uint8_t *Payload, MelCloudStatus *Status);

  void WriteOK(uint8_t *Payload, MelCloudStatus *Status);
};

#endif
