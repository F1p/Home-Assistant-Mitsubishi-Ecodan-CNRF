#include "MELCloudDecoder.h"
#include <cstdio>


MELCLOUDDECODER::MELCLOUDDECODER(void) {
  memset(&RxMessage, 0, sizeof(MessageStruct));
  memset(&Status, 0, sizeof(MelCloudStatus));

  Preamble[0] = 0x04;
  Preamble[1] = 0x03;
}


uint8_t MELCLOUDDECODER::Process(uint8_t c) {
  uint8_t ReturnValue = false;

  if (BuildRxMessage(&RxMessage, c)) {
    ReturnValue = true;
    if (RxMessage.PacketType == CONNECT_REQUEST) {
      Process0x5A(RxMessage.Payload, &Status);
    } else if (RxMessage.PacketType == INIT_REQUEST) {
      Process0x4C(RxMessage.Payload, &Status);
    } else if (RxMessage.PacketType == GET_REQUEST) {
      Process0x48(RxMessage.Payload, &Status);
    } else if (RxMessage.PacketType == SET_TEMP) {
      Process0x49(RxMessage.Payload, &Status);
    } else if (RxMessage.PacketType == SET_DHW) {
      Process0x4A(RxMessage.Payload, &Status);
    } else if (RxMessage.PacketType == SET_HOL) {
      Process0x4B(RxMessage.Payload, &Status);
    }
  }
  return ReturnValue;
}

uint8_t MELCLOUDDECODER::BuildRxMessage(MessageStruct *Message, uint8_t c) {
  static uint8_t Buffer[COMMANDSIZE];
  static uint8_t BufferPos = 0;
  static uint8_t PayloadSize = 0;
  uint8_t i;

  if (BufferPos < HEADERSIZE) {
    switch (BufferPos) {
      case 0:
        if (c != PACKET_SYNC) return false;  // Sync Byte 0xFC
        break;
      case 1:
        switch (c) {
          case INIT_REQUEST:
            break;
          case INIT_RESPONSE:
            break;
          case GET_REQUEST:
            break;
          case GET_RESPONSE:
            break;
          case SET_TEMP:
            break;
          case SET_TEMP_RESPONSE:
            break;
          case SET_DHW:
            break;
          case SET_DHW_RESPONSE:
            break;
          case SET_HOL:
            break;
          case SET_HOL_RESPONSE:
            break;
          case CONNECT_REQUEST:
            break;
          case CONNECT_RESPONSE:
            break;
          default:
            //Serial.println("Unknown PacketType");
            BufferPos = 0;
            return false;  // Unknown Packet Type
        }
        break;

      case 2:
        if (c != Preamble[0]) {
          //Serial.println("Preamble 1 Error");
          BufferPos = 0;
          return false;
        }
        break;

      case 3:
        if (c != Preamble[1]) {
          //Serial.println("Preamble 2 Error");
          BufferPos = 0;
          return false;
        }
        break;

      case 4:
        PayloadSize = c;
        if (c > MAXDATABLOCKSIZE) {
          //Serial.println("Oversize Payload");
          BufferPos = 0;
          return false;
        }
        break;
    }
    Buffer[BufferPos] = c;
    BufferPos++;
    return false;
  } else if (BufferPos < (PayloadSize + HEADERSIZE)) {
    Buffer[BufferPos] = c;
    BufferPos++;
  } else if (BufferPos == (PayloadSize + HEADERSIZE)) {
    Buffer[BufferPos] = c;
    BufferPos = 0;
    if (CheckSum(Buffer, PayloadSize + HEADERSIZE) == c) {
      //Serial.println("CS OK");
      Message->SyncByte = Buffer[0];
      Message->PacketType = Buffer[1];
      Message->Preamble[0] = Buffer[2];
      Message->Preamble[1] = Buffer[3];
      Message->PayloadSize = Buffer[4];
      Message->Checksum = c;
      memcpy(Message->Payload, &Buffer[5], Message->PayloadSize);
      return true;
    } else {
      //Serial.println("Checksum Fail");
      return false;
    }
  }
  return false;
}


void MELCLOUDDECODER::Process0x5A(uint8_t *Buffer, MelCloudStatus *Status) {
  Status->ConnectRequest = true;
}
void MELCLOUDDECODER::Process0x4A(uint8_t *Buffer, MelCloudStatus *Status) {
  for (int i = 1; i < 16; i++) {
    Array0x4A[i] = Buffer[i];
  }
  Status->ReplyNow = true;
  Status->ActiveMessage = 0x4A;
}
void MELCLOUDDECODER::Process0x4B(uint8_t *Buffer, MelCloudStatus *Status) {
  for (int i = 1; i < 16; i++) {
    Array0x4B[i] = Buffer[i];
  }
  Status->ReplyNow = true;
  Status->ActiveMessage = 0x4B;
}
void MELCLOUDDECODER::Process0x4C(uint8_t *Buffer, MelCloudStatus *Status) {
  for (int i = 1; i < 16; i++) {
    Array0x4C[i] = Buffer[i];
  }
  Status->ReplyNow = true;
  Status->ActiveMessage = 0x4C;
}
void MELCLOUDDECODER::Process0x49(uint8_t *Buffer, MelCloudStatus *Status) {
  for (int i = 1; i < 16; i++) {
    Array0x49[i] = Buffer[i];
  }
  Status->ReplyNow = true;
  Status->ActiveMessage = 0x49;
}
void MELCLOUDDECODER::Process0x48(uint8_t *Buffer, MelCloudStatus *Status) {
  for (int i = 1; i < 16; i++) {
    Array0x48[i] = Buffer[i];
  }
  Status->ReplyNow = true;
  Status->ActiveMessage = 0x48;
}


void MELCLOUDDECODER::CreateBlankTxMessage(uint8_t PacketType, uint8_t PayloadSize) {
  CreateBlankMessageTemplate(&TxMessage, PacketType, PayloadSize);
}

void MELCLOUDDECODER::CreateBlankMessageTemplate(MessageStruct *Message, uint8_t PacketType, uint8_t PayloadSize) {
  uint8_t i;

  memset((void *)Message, 0, sizeof(MessageStruct));

  Message->SyncByte = PACKET_SYNC;
  Message->PacketType = PacketType;
  Message->PayloadSize = PayloadSize;
  for (i = 0; i < PREAMBLESIZE; i++) {
    Message->Preamble[i] = Preamble[i];
  }
}

void MELCLOUDDECODER::SetPayloadByte(uint8_t Data, uint8_t Location) {
  TxMessage.Payload[Location] = Data;
}

uint8_t MELCLOUDDECODER::PrepareTxCommand(uint8_t *Buffer) {
  return PrepareCommand(&TxMessage, Buffer);
}

uint8_t MELCLOUDDECODER::PrepareCommand(MessageStruct *Message, uint8_t *Buffer) {
  uint8_t MessageChecksum;
  uint8_t MessageSize;
  uint8_t i;

  Buffer[0] = Message->SyncByte;
  Buffer[1] = Message->PacketType;

  Buffer[2] = Message->Preamble[0];
  Buffer[3] = Message->Preamble[1];

  Buffer[4] = Message->PayloadSize;

  memcpy(&Buffer[5], Message->Payload, Message->PayloadSize);

  MessageSize = HEADERSIZE + Message->PayloadSize;

  MessageChecksum = CheckSum(Buffer, MessageSize);

  Buffer[MessageSize] = MessageChecksum;

  return MessageSize + 1;
}


uint8_t MELCLOUDDECODER::CheckSum(uint8_t *Buffer, uint8_t len) {
  uint8_t sum = 0;
  uint8_t i;

  for (i = 0; i < len; i++) {
    sum += Buffer[i];
  }

  sum = 0xfc - sum;
  sum = sum & 0xff;

  return sum;
}
