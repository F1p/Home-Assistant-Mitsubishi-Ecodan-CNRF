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

#ifndef ECODANDECODER_h
#define ECODANDECODER_h

#include <stdint.h>
#include <time.h>
#include <string.h>

#define PACKET_SYNC 0xFC
#define GET_REQUEST 0x48
#define GET_RESPONSE 0x68
#define GET_ABOUT_RESPONSE 0x7B
#define CONNECT_REQUEST 0x5A
#define CONNECT_RESPONSE 0x7A
#define INIT_REQUEST 0x4C
#define INIT_RESPONSE 0x6C


#define COMMANDSIZE 22  // 5 Byte Header + 16 Byte Payload  + 1 Byte Checksum
#define HEADERSIZE 5
#define MAXDATABLOCKSIZE 16

#define PREAMBLESIZE 2


// System Flags

#define SET_ZONE_SETPOINT 0x80
#define UNKNOWN1 0x40
#define SET_HOT_WATER_SETPOINT 0x20
#define UNKNOWN2 0x10
#define SET_HEATING_CONTROL_MODE_Z1 0x08
#define SET_HEATING_CONTROL_MODE_Z2 0x10
#define SET_HOT_WATER_MODE 0x04
#define UNKNOWN3 0x02
#define SET_SYSTEM_POWER 0x01
#define SET_HOT_WATER_BOOST 0x01

#define ZONE1 0x00        // Zone1
#define ZONE1_TSTAT 0x02  // Zone 1 Thermostat
#define ZONE2 0x02        // Zone2
#define ZONE2_TSTAT 0x08  // Zone 2 Thermostat
#define BOTH 0x03         // BOTH



typedef struct _MessgeStruct {
  uint8_t SyncByte;
  uint8_t PacketType;
  uint8_t Preamble[PREAMBLESIZE];
  uint8_t PayloadSize;
  uint8_t Payload[MAXDATABLOCKSIZE];
  uint8_t Checksum;
} MessageStruct;

typedef struct _EcodanStatus {
  uint8_t Zone1ActiveInput, Zone2ActiveInput;
} EcodanStatus;


class ECODANDECODER {
public:
  ECODANDECODER(void);
  uint8_t Process(uint8_t c);

  void CreateBlankTxMessage(uint8_t PacketType, uint8_t PayloadSize);
  void SetPayloadByte(uint8_t Data, uint8_t Location);
  uint8_t PrepareTxCommand(uint8_t *Buffer);
  EcodanStatus Status;
protected:

private:
  MessageStruct RxMessage;
  MessageStruct TxMessage;



  uint8_t Preamble[PREAMBLESIZE];

  uint8_t BuildRxMessage(MessageStruct *Message, uint8_t c);

  void CreateBlankMessageTemplate(MessageStruct *Message, uint8_t PacketType, uint8_t PayloadSize);
  uint8_t PrepareCommand(MessageStruct *Message, uint8_t *Buffer);


  uint16_t ExtractUInt16(uint8_t *Buffer, uint8_t Index);
  float Extract_Float_24(uint8_t *Buffer, uint8_t Index);
  float ExtractUInt16_Signed(uint8_t *Buffer, uint8_t Index);
  float ExtractUInt8_v1(uint8_t *Buffer, uint8_t Index);
  float ExtractUInt8_v2(uint8_t *Buffer, uint8_t Index);
  float ExtractEnergy(uint8_t *Buffer, uint8_t index);

  uint8_t CheckSum(uint8_t *Buffer, uint8_t len);

  void Process0x6c(uint8_t *Payload, EcodanStatus *Status);

  void WriteOK(uint8_t *Payload, EcodanStatus *Status);
};



#endif
