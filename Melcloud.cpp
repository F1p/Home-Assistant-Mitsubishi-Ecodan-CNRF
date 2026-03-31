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
#include "Melcloud.h"

#include <ESPTelnet.h>
extern ESPTelnet TelnetServer;
#include "Debug.h"

uint8_t CNRFInit[] = { 0xfc, 0x7a, 0x04, 0x03, 0x01, 0x00, 0x7e };

bool PrintMELStart = false;
bool FirstReadAfterConnect = false;

MELCLOUD::MELCLOUD(void)
  : MELCLOUDDECODER() {
  UpdateFlag = 0;
  Connected = false;
  msbetweenmsg = 0;
}


void MELCLOUD::Process(void) {
  uint8_t c;

  while (DeviceStream->available()) {
    if (!PrintMELStart) {
      DEBUG_PRINT("[Base > Bridge] ");
      PrintMELStart = true;
    }
    c = DeviceStream->read();

    if (c == 0)
      DEBUG_PRINT("00, ");
    else {
      if (c < 0x10) DEBUG_PRINT("0");
      DEBUG_PRINT(String(c, HEX));
      DEBUG_PRINT(", ");
    }

    if (MELCLOUDDECODER::Process(c)) {
      DEBUG_PRINTLN();
      PrintMELStart = false;
      Connected = true;
    }
  }
}

void MELCLOUD::SetStream(Stream *MELCloudStream) {
  DeviceStream = MELCloudStream;
}



void MELCLOUD::ReplyStatus(uint8_t TargetMessage) {
  uint8_t Buffer[COMMANDSIZE];
  uint8_t CommandSize;
  uint8_t i;

  DEBUG_PRINT("[Bridge > Base] ");

  if (TargetMessage == INIT_REQUEST) {
    MELCLOUDDECODER::CreateBlankTxMessage(INIT_RESPONSE, 0x10);
  } else if (TargetMessage == GET_REQUEST) {
    MELCLOUDDECODER::CreateBlankTxMessage(GET_RESPONSE, 0x10);
  } else if (TargetMessage == SET_TEMP) {
    MELCLOUDDECODER::CreateBlankTxMessage(SET_TEMP_RESPONSE, 0x10);
  } else if (TargetMessage == SET_DHW) {
    MELCLOUDDECODER::CreateBlankTxMessage(SET_DHW_RESPONSE, 0x10);
  } else if (TargetMessage == SET_HOL) {
    MELCLOUDDECODER::CreateBlankTxMessage(SET_HOL_RESPONSE, 0x10);
  }

  for (int i = 1; i < 16; i++) {
    MELCLOUDDECODER::SetPayloadByte(Array0x68[i], i);
  }
  MELCLOUDDECODER::SetPayloadByte(TargetMessage, 0);

  CommandSize = MELCLOUDDECODER::PrepareTxCommand(Buffer);
  DeviceStream->write(Buffer, CommandSize);
  DeviceStream->flush();


  for (i = 0; i < CommandSize; i++) {
    if (Buffer[i] < 0x10) DEBUG_PRINT("0");
    DEBUG_PRINT(String(Buffer[i], HEX));
    DEBUG_PRINT(", ");
    Buffer[i] = 0x00;
  }
  DEBUG_PRINTLN();
}


void MELCLOUD::Connect(void) {
  DEBUG_PRINTLN("[Bridge > Base] Replying to connect request from Wireless Basestation...");
  FirstReadAfterConnect = true;
  DeviceStream->write(CNRFInit, 7);
  DeviceStream->flush();
  Process();
}



uint8_t MELCLOUD::UpdateComplete(void) {
  if (UpdateFlag) {
    UpdateFlag = 0;
    return 1;
  } else {
    return 0;
  }
}

uint8_t MELCLOUD::Lastmsbetweenmsg(void) {
  return msbetweenmsg;
}
