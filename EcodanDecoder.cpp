/*
    Copyright (C) <2020>  <Mike Roberts>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "EcodanDecoder.h"
#include <cstdio>

uint8_t Array0x4C[] = {};
uint8_t Array0x6C[] = {};
uint8_t Array0x48[] = {};
uint8_t Array0x68[] = {};
uint8_t Array0x49[] = {};
uint8_t Array0x69[] = {};
uint8_t Array0x4A[] = {};
uint8_t Array0x6A[] = {};
uint8_t Array0x4B[] = {};
uint8_t Array0x6B[] = {};

uint8_t BufferArray[][17] = { {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {} };


ECODANDECODER::ECODANDECODER(void) {
  memset(&RxMessage, 0, sizeof(MessageStruct));
  memset(&Status, 0, sizeof(EcodanStatus));

  Preamble[0] = 0x04;
  Preamble[1] = 0x03;
}


uint8_t ECODANDECODER::Process(uint8_t c) {
  uint8_t ReturnValue = false;

  if (BuildRxMessage(&RxMessage, c)) {
    ReturnValue = true;
    if (RxMessage.PacketType == INIT_RESPONSE) {
      Process0x6C(RxMessage.Payload, &Status);
    }
    else if (RxMessage.PacketType == GET_RESPONSE) {
      Process0x68(RxMessage.Payload, &Status);
    }
    else if (RxMessage.PacketType == SET_TEMP_RESPONSE) {
      Process0x69(RxMessage.Payload, &Status);
    }
    else if (RxMessage.PacketType == SET_DHW_RESPONSE) {
      Process0x6A(RxMessage.Payload, &Status);
    }
    else if (RxMessage.PacketType == SET_HOL_RESPONSE) {
      Process0x6B(RxMessage.Payload, &Status);
    }
  }
  return ReturnValue;
}

uint8_t ECODANDECODER::BuildRxMessage(MessageStruct *Message, uint8_t c) {
  static uint8_t Buffer[COMMANDSIZE];
  static uint8_t BufferPos = 0;
  static uint8_t PayloadSize = 0;
  uint8_t i;

  if (BufferPos < HEADERSIZE) {
    switch (BufferPos) {
      case 0:
        if (c != PACKET_SYNC) return false;
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
          //Serial.println("Preamble 1 Error");
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
  }

  else if (BufferPos == (PayloadSize + HEADERSIZE)) {
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

void ECODANDECODER::Process0x6C(uint8_t *Buffer, EcodanStatus *Status) {
  bool Write_To_Ecodan_OK;
  Write_To_Ecodan_OK = true;             // For de-queue
  Status->Write_To_Ecodan_OK = Write_To_Ecodan_OK;

  for (int i = 1; i < 16; i++) {
    Array0x6C[i] = Buffer[i];
  }
}


void ECODANDECODER::Process0x68(uint8_t *Buffer, EcodanStatus *Status) {
  uint8_t Power, SystemOpMode, Zone1ControlMode, Zone2ControlMode, TimerProhibit, Zone1ActiveInput, Zone2ActiveInput, ErrorCode, DHWForce, Holiday, ErrorCodeNum;
  uint8_t SetpointZ1Temp, SetpointZ2Temp;
  bool Write_To_Ecodan_OK;
  Write_To_Ecodan_OK = true;             // For de-queue
  Status->Write_To_Ecodan_OK = Write_To_Ecodan_OK;

  
  for (int i = 1; i < 16; i++) {
    Array0x68[i] = Buffer[i];
  }

  Power = Buffer[1];
  SystemOpMode = Buffer[2];
  SetpointZ1Temp = Buffer[3];
  Zone1ControlMode = Buffer[4];
  DHWForce = Buffer[5];
  Holiday = Buffer[6];
  TimerProhibit = Buffer[7];
  //Unknown = Buffer[8];
  //Unknown = Buffer[9];
  ErrorCodeNum = Buffer[10];
  //Unknown = Buffer[11];
  SetpointZ2Temp = Buffer[12];
  Zone2ControlMode = Buffer[4];

  Status->Power = Power;
  Status->SystemOpMode = SystemOpMode;
  Status->Zone1ControlMode = Zone1ControlMode;
  Status->SetpointZ1 = ((SetpointZ1Temp - 128.0f) / 2);  
  Status->TimerProhibit = TimerProhibit;
  Status->SetpointZ2 = ((SetpointZ2Temp - 128.0f) / 2);
  Status->Zone2ControlMode = Zone2ControlMode;
  Status->ErrorCode = ErrorCode;
  Status->ErrorCodeNum = ErrorCodeNum;
  Status->DHWForce = DHWForce;
  Status->Holiday = Holiday;
}

void ECODANDECODER::Process0x6A(uint8_t *Buffer, EcodanStatus *Status) {
  bool Write_To_Ecodan_OK;
  Write_To_Ecodan_OK = true;             // For de-queue
  Status->Write_To_Ecodan_OK = Write_To_Ecodan_OK;

  for (int i = 1; i < 16; i++) {
    Array0x6B[i] = Buffer[i];
  }
}

void ECODANDECODER::Process0x69(uint8_t *Buffer, EcodanStatus *Status) {
  bool Write_To_Ecodan_OK;
  Write_To_Ecodan_OK = true;             // For de-queue
  Status->Write_To_Ecodan_OK = Write_To_Ecodan_OK;

  for (int i = 1; i < 16; i++) {
    Array0x69[i] = Buffer[i];
  }
}

void ECODANDECODER::Process0x6B(uint8_t *Buffer, EcodanStatus *Status) {
  bool Write_To_Ecodan_OK;
  Write_To_Ecodan_OK = true;             // For de-queue
  Status->Write_To_Ecodan_OK = Write_To_Ecodan_OK;

  for (int i = 1; i < 16; i++) {
    Array0x6B[i] = Buffer[i];
  }
}


void ECODANDECODER::CreateBlankTxMessage(uint8_t PacketType, uint8_t PayloadSize) {
  CreateBlankMessageTemplate(&TxMessage, PacketType, PayloadSize);
}

void ECODANDECODER::CreateBlankMessageTemplate(MessageStruct *Message, uint8_t PacketType, uint8_t PayloadSize) {
  uint8_t i;

  memset((void *)Message, 0, sizeof(MessageStruct));

  Message->SyncByte = PACKET_SYNC;
  Message->PacketType = PacketType;
  Message->PayloadSize = PayloadSize;
  for (i = 0; i < PREAMBLESIZE; i++) {
    Message->Preamble[i] = Preamble[i];
  }
}

void ECODANDECODER::SetPayloadByte(uint8_t Data, uint8_t Location) {
  TxMessage.Payload[Location] = Data;
}

uint8_t ECODANDECODER::PrepareTxCommand(uint8_t *Buffer) {
  return PrepareCommand(&TxMessage, Buffer);
}


void ECODANDECODER::EncodeMELCloud(uint8_t cmd) {
  TxMessage.Payload[0] = cmd;
  for (int i = 1; i < 16; i++) {
    if (cmd == INIT_REQUEST) {
      TxMessage.Payload[i] = Array0x4C[i];
    } else if (cmd == GET_REQUEST) {
      TxMessage.Payload[i] = Array0x48[i];
    } else if (cmd == SET_TEMP) {
      TxMessage.Payload[i] = Array0x49[i];
    } else if (cmd == SET_DHW) {
      TxMessage.Payload[i] = Array0x4A[i];
    } else if (cmd == SET_HOL) {
      TxMessage.Payload[i] = Array0x4B[i];
    }
  }
}


void ECODANDECODER::TransfertoBuffer(uint8_t msgtype, uint8_t bufferposition) {
  BufferArray[bufferposition][0] = msgtype;
  for (int i = 1; i < 17; i++) {
    BufferArray[bufferposition][i] = TxMessage.Payload[i - 1];
  }
}
uint8_t ECODANDECODER::ReturnNextCommandType(uint8_t bufferposition) {
  return BufferArray[bufferposition][0];
}

void ECODANDECODER::EncodeNextCommand(uint8_t bufferposition) {
  for (int i = 1; i < 17; i++) {
    TxMessage.Payload[i - 1] = BufferArray[bufferposition][i];
  }
}

uint8_t ECODANDECODER::PrepareCommand(MessageStruct *Message, uint8_t *Buffer) {
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



uint8_t ECODANDECODER::CheckSum(uint8_t *Buffer, uint8_t len) {
  uint8_t sum = 0;
  uint8_t i;

  for (i = 0; i < len; i++) {
    sum += Buffer[i];
  }

  sum = 0xfc - sum;
  sum = sum & 0xff;

  return sum;
}