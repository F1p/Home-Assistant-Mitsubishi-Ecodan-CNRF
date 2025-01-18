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


float Controller1Temperature, Controller2Temperature;

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
}


void ECODAN::TriggerStatusStateMachine(void) {
  if (!Connected) {
    Connect();
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
    ECODANDECODER::SetPayloadByte(((Controller1Temperature - 128.0f) / 2), 1);  // Controller 1 Current Temperature
    ECODANDECODER::SetPayloadByte(((Controller2Temperature - 128.0f) / 2), 2);  // Controller 2 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 3);         // Controller 3 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 4);         // Controller 4 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 5);         // Controller 5 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 6);         // Controller 6 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 7);         // Controller 7 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 8);         // Controller 8 Current Temperature
    ECODANDECODER::SetPayloadByte(0x03, 9);         // Controller 2 Current Temperature
    ECODANDECODER::SetPayloadByte(0xff, 10);        // Controller 2 Current Temperature
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