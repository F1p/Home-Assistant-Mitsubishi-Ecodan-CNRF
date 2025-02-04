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
#include "Ecodan.h"

#include <ESPTelnet.h>
extern ESPTelnet TelnetServer;
#include "Debug.h"

float RCTemp[8];
int ControllerQTY;

// Initialisation Commands
uint8_t Init3[] = { 0xfc, 0x5a, 0x04, 0x03, 0x02, 0xca, 0x01, 0xd2 };  // CNRF Connect


unsigned long lastmsgdispatchedMillis = 0;  // variable for comparing millis counter
int cmd_queue_length = 0;
int cmd_queue_position = 1;
bool WriteInProgress = false;

ECODAN::ECODAN(void)
  : ECODANDECODER() {
  CurrentMessage = 0;
  CurrentCommand = 0;
  UpdateFlag = 0;
  ProcessFlag = false;
  Connected = false;
  msbetweenmsg = 0;
}


void ECODAN::Process(void) {
  uint8_t c;

  while (DeviceStream->available()) {
    if (!ProcessFlag) {
      DEBUG_PRINT("[CNRF > Bridge] ");
      ProcessFlag = true;
    }
    c = DeviceStream->read();

    if (c == 0)
      DEBUG_PRINT("__, ");
    else {
      if (c < 0x10) DEBUG_PRINT("0");
      DEBUG_PRINT(String(c, HEX));
      DEBUG_PRINT(", ");
    }

    if (ECODANDECODER::Process(c)) {
      ProcessFlag = false;
      msbetweenmsg = millis() - lastmsgdispatchedMillis;
      DEBUG_PRINTLN();
      Connected = true;
    }
  }
}

void ECODAN::SetStream(Stream *HeatPumpStream) {
  DeviceStream = HeatPumpStream;
  Connect();
  delay(250);
  ConfigConnect();
}


void ECODAN::TriggerStatusStateMachine(void) {
  if (!Connected) {
    Connect();
    delay(250);
    ConfigConnect();
  }
  CurrentMessage = 1;  // This triggers the run
  Connected = false;
}



void ECODAN::StatusStateMachine(void) {
  uint8_t Buffer[COMMANDSIZE];
  uint8_t CommandSize;
  uint8_t i;

  if (CurrentMessage != 0) {
    DEBUG_PRINT("[Bridge > FTC] ");
    ECODANDECODER::CreateBlankTxMessage(GET_REQUEST, 0x10);
    ECODANDECODER::SetPayloadByte(0x27, 0);

    // Update based on quantity in use
    for (int i = 0; i < 8; i++) {
      if ((i + 1) <= ControllerQTY) {
        ECODANDECODER::SetPayloadByte(((RCTemp[i] * 2) + 128), (i + 1));
      } else {
        ECODANDECODER::SetPayloadByte(0xff, i + 1);
      }
    }

    // Controller Quantity Bitmask
    switch (ControllerQTY) {
      case (1):
        ECODANDECODER::SetPayloadByte(0x01, 9);
        break;
      case (2):
        ECODANDECODER::SetPayloadByte(0x03, 9);
        break;
      case (3):
        ECODANDECODER::SetPayloadByte(0x07, 9);
        break;
      case (4):
        ECODANDECODER::SetPayloadByte(0x0F, 9);
        break;
      case (5):
        ECODANDECODER::SetPayloadByte(0x1F, 9);
        break;
      case (6):
        ECODANDECODER::SetPayloadByte(0x3F, 9);
        break;
      case (7):
        ECODANDECODER::SetPayloadByte(0x7F, 9);
        break;
      case (8):
        ECODANDECODER::SetPayloadByte(0xFF, 9);
        break;
    }

    ECODANDECODER::SetPayloadByte(0xff, 10);  // Unknown exactly what this is...

    CommandSize = ECODANDECODER::PrepareTxCommand(Buffer);
    DeviceStream->write(Buffer, CommandSize);
    lastmsgdispatchedMillis = millis();
    DeviceStream->flush();

    for (i = 0; i < CommandSize; i++) {
      if (Buffer[i] < 0x10) DEBUG_PRINT("0");
      DEBUG_PRINT(String(Buffer[i], HEX));
      DEBUG_PRINT(", ");
    }
    DEBUG_PRINTLN();

    CurrentMessage = 0;

    // Straight to end
    if (CurrentMessage == 0) {
      UpdateFlag = 1;
    }
  }
}


void ECODAN::Connect(void) {
  DEBUG_PRINTLN("Connecting to Heat Pump...");
  DeviceStream->write(Init3, 8);
  DeviceStream->flush();
  Process();
}

void ECODAN::ConfigConnect(void) {
  uint8_t Buffer[COMMANDSIZE];
  uint8_t CommandSize;
  uint8_t i;

  DEBUG_PRINTLN("Sending Config Request to Heat Pump...");
  // Config Builder
  ECODANDECODER::CreateBlankTxMessage(GET_INIT, 0x10);

  // Controller Quantity Bitmask
  switch (ControllerQTY) {
    case (1):
      ECODANDECODER::SetPayloadByte(0x01, 1);
      break;
    case (2):
      ECODANDECODER::SetPayloadByte(0x03, 1);
      break;
    case (3):
      ECODANDECODER::SetPayloadByte(0x07, 1);
      break;
    case (4):
      ECODANDECODER::SetPayloadByte(0x0F, 1);
      break;
    case (5):
      ECODANDECODER::SetPayloadByte(0x1F, 1);
      break;
    case (6):
      ECODANDECODER::SetPayloadByte(0x3F, 1);
      break;
    case (7):
      ECODANDECODER::SetPayloadByte(0x7F, 1);
      break;
    case (8):
      ECODANDECODER::SetPayloadByte(0xFF, 1);
      break;
  }

  ECODANDECODER::SetPayloadByte(0xff, 2);  // Unknown exactly what this is...

  CommandSize = ECODANDECODER::PrepareTxCommand(Buffer);
  DeviceStream->write(Buffer, CommandSize);
  lastmsgdispatchedMillis = millis();

  DeviceStream->flush();

  for (i = 0; i < CommandSize; i++) {
    if (Buffer[i] < 0x10) DEBUG_PRINT(F("0"));
    DEBUG_PRINT(String(Buffer[i], HEX));
    DEBUG_PRINT(F(", "));
  }
  DEBUG_PRINTLN();


  Process();
}

uint8_t ECODAN::HeatPumpConnected(void) {
  return Connected;
}

uint8_t ECODAN::UpdateComplete(void) {
  if (UpdateFlag) {
    UpdateFlag = 0;
    return 1;
  } else {
    return 0;
  }
}

uint8_t ECODAN::Lastmsbetweenmsg(void) {
  return msbetweenmsg;
}