/*
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

// -- Supported Hardware -- //
/* As sold Witty ESP8266 based               / Core 3.1.2 / Flash 4MB (1MB FS / 1MB OTA - 16KB Cache/48KB IRAM not shared)  */
/* ESP32 AtomS3 Lite (ESP32S3 Dev Module)    / Core 3.2.0 / Flash 8M with SPIFFS (3MB APP / 1.5MB SPIFFS)                   */
/* ESP32 Ethernet WT32-ETH01                 / Core 3.2.0 / Flash 4MB (1.2MB APP / 1.5MB SPIFFS)                            */


#if defined(ESP8266) || defined(ESP32)  // ESP32 or ESP8266 Compatiability

#include <FS.h>  // Define File System First
#include <LittleFS.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>
#include <AsyncTCP.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdate.h>
#endif
#ifdef ARDUINO_WT32_ETH01
#include <ETH.h>
#include <Arduino.h>
#endif

#define WEBSERVER_H "fix confict"
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPTelnet.h>
#include "HttpsOTAUpdate.h"
#include "Ecodan.h"
#include "Melcloud.h"

extern float RCTemp[8];
extern int ControllerQTY;
String FirmwareVersion = "2.0.0-Beta11";


#ifdef ESP8266  // Define the Witty ESP8266 Serial Pins
#define HEATPUMP_STREAM SwSerial1
#define MEL_STREAM SwSerial2
#define SERIAL_CONFIG SWSERIAL_8E1
int LDR = A0;
#define MEL_RxPin 3
int Activity_LED = 2;
int Reset_Button = 4;
#define MEL_TxPin 1
int Green_RGB_LED = 12;
int Blue_RGB_LED = 13;
#define FTCCable_RxPin 14
int Red_RGB_LED = 15;
#define FTCCable_TxPin 16
#endif

#ifdef ESP32  // Define the M5Stack Serial Pins
#define HEATPUMP_STREAM Serial1
#define MEL_STREAM Serial2
#define SERIAL_CONFIG SERIAL_8E1

#ifdef ARDUINO_M5STACK_ATOMS3
#include <LiteLED.h>
#define LED_TYPE LED_STRIP_WS2812
#define LED_TYPE_IS_RGBW 0
//#define LED_GPIO 42
#define LED_GPIO 35
#define LED_BRIGHT 100
static const crgb_t L_RED = 0xff0000;
static const crgb_t L_GREEN = 0x00ff00;
static const crgb_t L_BLUE = 0x0000ff;
static const crgb_t L_ORANGE = 0xffa500;
LiteLED myLED(LED_TYPE, LED_TYPE_IS_RGBW);
int Reset_Button = 41;
#define FTCCable_RxPin 2
#define FTCCable_TxPin 1
#define FTCProxy_RxPin 38
#define FTCProxy_TxPin 39
#define MEL_RxPin 8
#define MEL_TxPin 7
#endif

#ifdef ARDUINO_WT32_ETH01
#define FTCCable_RxPin 4
#define FTCCable_TxPin 2
#define MEL_RxPin 14
#define MEL_TxPin 12
#ifndef ETH_PHY_TYPE
#define ETH_PHY_TYPE ETH_PHY_LAN8720
#define ETH_PHY_ADDR 0
#define ETH_PHY_MDC 23
#define ETH_PHY_MDIO 18
#define ETH_PHY_POWER -1
#define ETH_CLK_MODE ETH_CLOCK_GPIO0_IN
#endif
#endif
#endif


#define Heartbeat_Range 99  // Heatbeat Max value
int Heart_Value = 0;        // Heatbeat ID

unsigned long SERIAL_BAUD = 2400;
bool shouldSaveConfig = false;

const int deviceId_max_length = 15;
const int hostname_max_length = 200;
const int port_max_length = 10;
const int user_max_length = 50;
const int password_max_length = 50;
const int basetopic_max_length = 30;
bool BlockWriteFromMELCloud = false;
bool MELCloud_Adapter_Connected = false;

enum DiscoveryState { IDLE,
                      LISTENING };
DiscoveryState currentDiscoveryState = IDLE;
unsigned long discoveryStartTime = 0;
const unsigned long discoveryTimeout = 15000;  // 15 seconds


const char* witty_house_root_ca =
  "-----BEGIN CERTIFICATE-----\n"
  "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\n"
  "-----END CERTIFICATE-----\n";

static HttpsOTAStatus_t otastatus;

// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
// Here you can pre-set the settings for the MQTT connection. The settings can later be changed via Wifi Manager.
struct MqttSettings {
  // These are the placeholder objects for the custom fields
  char deviceId[deviceId_max_length] = "000000000000";
  char wm_device_id_identifier[10] = "device_id";

  // Client 1
  char hostname[hostname_max_length] = "homeassistant.local";
  char user[user_max_length] = "Username";
  char password[password_max_length] = "Password";
  char port[port_max_length] = "1883";
  char baseTopic[basetopic_max_length] = "Ecodan/CNRF";
  char wm_mqtt_hostname_identifier[14] = "mqtt_hostname";
  char wm_mqtt_user_identifier[10] = "mqtt_user";
  char wm_mqtt_password_identifier[14] = "mqtt_password";
  char wm_mqtt_port_identifier[10] = "mqtt_port";
  char wm_mqtt_basetopic_identifier[15] = "mqtt_basetopic";

  // Heatmiser
  char heatmiser[hostname_max_length] = "neohub.local";
  char z1_name[hostname_max_length] = "Zone_1_Name";
  char z2_name[hostname_max_length] = "Zone_2_Name";
  char wm_hm_hostname_identifier[12] = "hm_hostname";
  char wm_hm_z1_identifier[6] = "hm_z1";
  char wm_hm_z2_identifier[6] = "hm_z2";
};

struct UnitSettings {
  int Quantity = 2;
  char qty_identifier[4] = "qty";
};


MqttSettings mqttSettings;
UnitSettings unitSettings;
ECODAN HeatPump;
MELCLOUD MELCloud;
HTTPClient http;
#ifdef ESP8266
SoftwareSerial SwSerial1;
SoftwareSerial SwSerial2;
#endif
WiFiClient NetworkClient;
WiFiClientSecure NetworkClientS;  // Encryption Support
WiFiUDP udp;

const int discoveryPort = 19790;
char packetBuffer[255];

PubSubClient MQTTClient1(NetworkClient);
ESPTelnet TelnetServer;


// Delcare Global Scope for Non-Blocking, always active Portal with "TEMP" placeholder, real values populated later from filesystem
WiFiManagerParameter custom_hm_server("hm_server", "Heatmiser Neo Hub IP", "TEMP", hostname_max_length);
WiFiManagerParameter custom_hm_z1("z1", "Zone 1 Heatmiser Zone Name", "TEMP", hostname_max_length);
WiFiManagerParameter custom_hm_z2("z2", "Zone 2 Heatmiser Zone Name", "TEMP", hostname_max_length);

WiFiManagerParameter custom_mqtt_server("server", "<hr><br>(Optional) MQTT Server (IP Address or DNS)", "TEMP", hostname_max_length);
WiFiManagerParameter custom_mqtt_user("user", "(Optional) MQTT Username", "TEMP", user_max_length);
WiFiManagerParameter custom_mqtt_pass("pass", "(Optional) MQTT Password", "TEMP", password_max_length);
WiFiManagerParameter custom_mqtt_port("port", "(Optional) MQTT Server Port (Default: 1883)", "TEMP", port_max_length);
WiFiManagerParameter custom_mqtt_basetopic("basetopic", "(Optional) MQTT Base Topic (Default: Ecodan/CNRF)<br><font size='0.8em'>Modify if you have multiple heat pumps connecting to the same MQTT server</font>", "TEMP", basetopic_max_length);
WiFiManagerParameter custom_device_id("device_id", "<hr><br>Device ID", "TEMP", deviceId_max_length, "readonly");

WiFiManager wifiManager;

float RCInput[7];

#include "TimerCallBack.h"
#include "Debug.h"
#include "MQTTDiscovery.h"
#include "MQTTConfig.h"

void HeatPumpQueryStateEngine(void);
void HeatPumpKeepAlive(void);
void Report(void);
void StatusReport(void);
void PublishAllReports(void);
void RequestHeatmiserData(void);
void HeatPumpWriteStateEngine(void);
void CheckForOTAUpdates(void);


TimerCallBack HeatPumpQuery1(1000, HeatPumpQueryStateEngine);  // Set to 400ms (Safe), 320-350ms best time between messages
TimerCallBack HeatPumpQuery2(2000, HeatPumpKeepAlive);         // Set to 20-30s for heat pump query frequency
TimerCallBack HeatPumpQuery3(30000, handleMQTTState);          // Re-connect attempt timer if MQTT is not online
TimerCallBack HeatPumpQuery4(30000, PublishAllReports);        // Set to 20-30s for heat pump query frequency
TimerCallBack HeatPumpQuery5(60000, RequestHeatmiserData);     // Query Heatmiser
TimerCallBack HeatPumpQuery6(1000, HeatPumpWriteStateEngine);  // Set to 500ms (Safe), 320-350ms best time between messages
TimerCallBack HeatPumpQuery7(3600000, CheckForOTAUpdates);     // Set check period to 1hr


unsigned long looppreviousMicros = 0;    // variable for comparing millis counter
unsigned long ftcpreviousMillis = 0;     // variable for comparing millis counter
unsigned long wifipreviousMillis = 0;    // variable for comparing millis counter
unsigned long postwrpreviousMillis = 0;  // variable for comparing millis counter
int FTCLoopSpeed, CPULoopSpeed;          // variable for holding loop time in ms
bool WiFiOneShot = true;
bool FTCOneShot = true;
bool FTCOneShot1 = false;
bool FTCOneShot2 = false;
bool CableConnected = true;
bool WiFiConnectedLastLoop = false;
bool PostWriteTrigger = false;
int connectionFailures = 0;
unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 60000;

extern int cmd_queue_length;
extern int cmd_queue_position;
extern bool WriteInProgress;
extern int CurrentWriteAttempt;

#ifdef ARDUINO_WT32_ETH01
static bool eth_connected = false;
#endif


void setup() {
  WiFi.mode(WIFI_STA);         // explicitly set mode, esp defaults to STA+AP
  DEBUGPORT.begin(DEBUGBAUD);  // Start Debug

  HEATPUMP_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, FTCCable_RxPin, FTCCable_TxPin);  // Rx, Tx
  HeatPump.SetStream(&HEATPUMP_STREAM);
  MEL_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, MEL_RxPin, MEL_TxPin);  // Rx, Tx
  MELCloud.SetStream(&MEL_STREAM);

#ifdef ARDUINO_WT32_ETH01
  Network.onEvent(onEvent);
  ETH.begin();
#endif

#ifndef ARDUINO_WT32_ETH01
  pinMode(Reset_Button, INPUT);  // Pushbutton on other modules
#endif


// -- Lights for ESP8266 and ESP32 -- //
#ifdef ARDUINO_M5STACK_ATOMS3    // Define the M5Stack LED
  myLED.begin(LED_GPIO, 1);      // initialze the myLED object. Here we have 1 LED attached to the LED_GPIO pin
  myLED.brightness(LED_BRIGHT);  // set the LED photon intensity level
#endif
#ifdef ESP8266                     // Define the Witty ESP8266 Ports
  pinMode(Activity_LED, OUTPUT);   // ESP8266 Onboard LED
  pinMode(LDR, INPUT);             // LDR
  pinMode(Red_RGB_LED, OUTPUT);    // Red (RGB) LED
  pinMode(Green_RGB_LED, OUTPUT);  // Green (RGB) LED
  pinMode(Blue_RGB_LED, OUTPUT);   // Blue (RGB) LED

  digitalWrite(Activity_LED, HIGH);  // Set On (Inverted)
  digitalWrite(Red_RGB_LED, LOW);    // Set Off
  digitalWrite(Green_RGB_LED, LOW);  // Set Off
  digitalWrite(Blue_RGB_LED, LOW);   // Set Off
#endif

  readSettingsFromConfig();
  initializeWifiManager();
  RecalculateMQTTTopics();

  //shouldSaveConfig = true;  // Update the config in File System
  if (shouldSaveConfig) {
    saveConfig();
  }

  setupTelnet();
  startTelnet();

  MQTTClient1.setBufferSize(2048);  // Increase MQTT Buffer Size
  initializeMQTTClient1();
  MQTTClient1.setCallback(MQTTonData);
  wifiManager.startWebPortal();

  MDNS.begin("heatpump-cnrf");
  MDNS.addService("http", "tcp", 80);
  HttpsOTA.onHttpEvent(HttpEvent);

  for (int i = 0; i < 8; i++) {
    RCInput[i] = 18.0;
    RCTemp[i] = RCInput[i];
  }


  HeatPumpKeepAlive();
  DEBUG_PRINTLN("Exit Setup");
}


void loop() {
  // -- Loop Start -- //
  looppreviousMicros = micros();  // Loop Speed Check
  ControllerQTY = unitSettings.Quantity;

  // -- Process Handlers -- //
  HeatPumpQuery1.Process();
  HeatPumpQuery2.Process();
  HeatPumpQuery3.Process();
  HeatPumpQuery4.Process();
  HeatPumpQuery5.Process();
  HeatPumpQuery6.Process();
  HeatPumpQuery7.Process();
  runNonBlockingDiscovery();


  MELCloudQueryReplyEngine();
  MQTTClient1.loop();
  TelnetServer.loop();
  HeatPump.Process();
  MELCloud.Process();
  wifiManager.process();


  otastatus = HttpsOTA.status();
  if (otastatus == HTTPS_OTA_SUCCESS) {
    DEBUG_PRINTLN(F("Firmware updated successfully - Rebooting..."));
    ESP.restart();
  } else if (otastatus == HTTPS_OTA_FAIL) {
    DEBUG_PRINTLN(F("Firmware Update Failed"));
  }



  // -- Config Saver -- //
  if (shouldSaveConfig) {
    saveConfig();
    DEBUG_PRINTLN("Returned");
  }  // Handles WiFiManager Settings Changes

  // -- Heat Pump Write Command Handler -- //
  if (HeatPump.Status.Write_To_Ecodan_OK && WriteInProgress) {  // A write command is executing
    DEBUG_PRINTLN(F("Write OK!"));                              // Pause normal processsing until complete
    HeatPump.Status.Write_To_Ecodan_OK = false;                 // Set back to false
    WriteInProgress = false;                                    // Set back to false
    if (cmd_queue_length > cmd_queue_position) {
      cmd_queue_position++;  // Increment the position
      CurrentWriteAttempt = 0;
    } else {
      cmd_queue_position = 1;  // All commands written, reset
      cmd_queue_length = 0;
      CurrentWriteAttempt = 0;
      PostWriteTrigger = true;  // Allows a number of seconds to pass, then restarts read operation
      postwrpreviousMillis = millis();
    }                                              // Dequeue the last message that was written
    if (MQTTReconnect()) { PublishAllReports(); }  // Publish update to the MQTT Topics
  } else if ((WriteInProgress) && (CurrentWriteAttempt > 10)) {
    if (cmd_queue_length > cmd_queue_position) {
      cmd_queue_position++;  // Skip this write + Increment the position
      CurrentWriteAttempt = 0;
    } else {
      cmd_queue_position = 1;  // All commands written, reset
      cmd_queue_length = 0;
      CurrentWriteAttempt = 0;
      PostWriteTrigger = true;  // Allows a number of seconds to pass, then restarts read operation
      postwrpreviousMillis = millis();
    }
  }

  // -- Read Operation Restart -- //
  if ((PostWriteTrigger) && (millis() - postwrpreviousMillis >= 3000)) {
    DEBUG_PRINTLN(F("Restarting Read Operations"));
    PostWriteTrigger = false;
  }

  // -- WiFi Status Handler -- //
  if (WiFi.status() != WL_CONNECTED && !wifiManager.getConfigPortalActive()) {
    if (WiFiOneShot) {
      wifipreviousMillis = millis();
      WiFiOneShot = false;
#ifdef ESP8266                           // Define the Witty ESP8266 Ports
      digitalWrite(Blue_RGB_LED, LOW);   // Turn the Blue LED Off
      digitalWrite(Green_RGB_LED, LOW);  // Turn the Green LED Off
      digitalWrite(Red_RGB_LED, HIGH);   // Turn the Red LED On
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
      myLED.setPixel(0, L_RED, 1);
#endif
    }  // Oneshot to start the timer
    if (millis() - wifipreviousMillis >= 300000) {
#ifdef ESP8266                          // Define the Witty ESP8266 Ports
      digitalWrite(Red_RGB_LED, HIGH);  // Flash the Red LED
      delay(500);
      digitalWrite(Red_RGB_LED, LOW);
      delay(500);
      digitalWrite(Red_RGB_LED, HIGH);
      delay(500);
      digitalWrite(Red_RGB_LED, LOW);
      delay(500);
      digitalWrite(Red_RGB_LED, HIGH);
      ESP.reset();
#endif
#ifdef ARDUINO_M5STACK_ATOMS3       // Define the M5Stack LED
      myLED.setPixel(0, L_RED, 1);  // set the LED colour and show it     // Flash the Red LED
      myLED.brightness(LED_BRIGHT, 1);
      delay(500);
      myLED.brightness(0, 1);
      delay(500);
      myLED.brightness(LED_BRIGHT, 1);
      delay(500);
      myLED.brightness(0, 1);
      delay(500);
      myLED.brightness(LED_BRIGHT, 1);
      ESP.restart();
#endif
    }  // Wait for 5 mins to try reconnects then force restart
    WiFiConnectedLastLoop = false;
  } else if (WiFi.status() != WL_CONNECTED && wifiManager.getConfigPortalActive()) {
#ifdef ESP8266                         // Define the Witty ESP8266 Ports
    digitalWrite(Blue_RGB_LED, HIGH);  // Turn the Blue LED On
    analogWrite(Green_RGB_LED, LOW);   // Green LED on, 25% brightness
    digitalWrite(Red_RGB_LED, LOW);    // Turn the Red LED Off
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
    myLED.setPixel(0, L_BLUE, 1);
#endif
    WiFiConnectedLastLoop = false;
  } else {                              // WiFi is connected
    if (!WiFiConnectedLastLoop) {       // Used to update LEDs only on transition of state
#ifdef ESP8266                          // Define the Witty ESP8266 Ports
      digitalWrite(Blue_RGB_LED, LOW);  // Turn the Blue LED Off
      analogWrite(Green_RGB_LED, 30);   // Green LED on, 25% brightness
      digitalWrite(Red_RGB_LED, LOW);   // Turn the Red LED Off
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
      myLED.setPixel(0, L_GREEN, 1);
#endif
    }
    WiFiOneShot = true;
    WiFiConnectedLastLoop = true;
  }

  if (millis() - lastRequestTime >= requestInterval) {
    lastRequestTime = millis();
    RequestHeatmiserData();
  }

  // -- Push Button Action Handler -- //
#ifndef ARDUINO_WT32_ETH01
  if (digitalRead(Reset_Button) == LOW) {  // Inverted (Button Pushed is LOW)
#ifdef ESP8266                             // Define the Witty ESP8266 Ports
    digitalWrite(Red_RGB_LED, HIGH);       // Flash the Red LED
    delay(500);
    digitalWrite(Red_RGB_LED, LOW);
    delay(500);
    digitalWrite(Red_RGB_LED, HIGH);
    delay(500);
    digitalWrite(Red_RGB_LED, LOW);
    delay(500);
    digitalWrite(Red_RGB_LED, HIGH);
    delay(500);
#endif
#ifdef ARDUINO_M5STACK_ATOMS3     // Define the M5Stack LED
    myLED.setPixel(0, L_RED, 1);  // Flash the Red LED
    delay(500);
    myLED.brightness(0, 1);
    delay(500);
    myLED.brightness(LED_BRIGHT, 1);
    delay(500);
    myLED.brightness(0, 1);
    delay(500);
    myLED.brightness(LED_BRIGHT, 1);
    delay(500);
#endif

    if (digitalRead(Reset_Button) == LOW) {  // If still pressed after flashing seq - reset
#ifdef ESP8266
      digitalWrite(Red_RGB_LED, LOW);
      digitalWrite(Blue_RGB_LED, HIGH);
      delay(500);
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
      myLED.setPixel(0, L_BLUE, 1);
#endif
      delay(500);
      wifiManager.resetSettings();  // Clear settings
      LittleFS.format();            // Wipe Filesystem
    }

#ifdef ESP8266
    ESP.reset();  // Define the Witty ESP8266 Ports
#endif
#ifdef ESP32        // ESP32 Action
    ESP.restart();  // No button on ETH
#endif
  }
#endif

  // -- CPU Loop Time End -- //
  CPULoopSpeed = micros() - looppreviousMicros;  // Loop Speed End Monitor
}

void HeatPumpKeepAlive(void) {
  if (!HeatPump.HeatPumpConnected()) {
    DEBUG_PRINTLN("Heat Pump Disconnected");
#ifdef ARDUINO_M5STACK_ATOMS3
    // Swap to the other pins and test the connection
    if (CableConnected) {
      DEBUG_PRINTLN("Trying to connect via Proxy Circuit Board");
      HEATPUMP_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, FTCProxy_RxPin, FTCProxy_TxPin);  // Rx, Tx
      HeatPump.SetStream(&HEATPUMP_STREAM);
      CableConnected = false;
    } else {
      DEBUG_PRINTLN("Trying to connect via Cable");
      HEATPUMP_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, FTCCable_RxPin, FTCCable_TxPin);  // Rx, Tx
      HeatPump.SetStream(&HEATPUMP_STREAM);
      CableConnected = true;
    }
#endif
  }
  ftcpreviousMillis = millis();
  HeatPump.TriggerStatusStateMachine();
  if (MQTTReconnect()) { StatusReport(); }
}

void HeatPumpQueryStateEngine(void) {

  if (cmd_queue_length == 0) {      // If there is no commands awaiting written
    HeatPump.StatusStateMachine();  // Full Read trigged by CurrentMessage
  }

  // Call Once Full Update is complete
  if (HeatPump.UpdateComplete()) {
    DEBUG_PRINTLN(F("FTC Update Complete"));
    FTCLoopSpeed = millis() - ftcpreviousMillis;  // Loop Speed End
    Report();
  }
}

void HeatPumpWriteStateEngine(void) {
  HeatPump.WriteStateMachine();  // Full Read trigged by CurrentMessage
}

void MELCloudQueryReplyEngine(void) {
  if (MELCloud.Status.ReplyNow) {
    if (MELCloud.Status.ActiveMessage == SET_DHW || MELCloud.Status.ActiveMessage == SET_HOL || MELCloud.Status.ActiveMessage == SET_TEMP) {  // The write commands
      if (!BlockWriteFromMELCloud) { HeatPump.WriteMELCloudCMD(MELCloud.Status.ActiveMessage); }
    }
    MELCloud.ReplyStatus(MELCloud.Status.ActiveMessage);  // Reply with the OK Message to MELCloud
    MELCloud.Status.ReplyNow = false;
  } else if (MELCloud.Status.ConnectRequest) {
    MELCloud.Connect();  // Reply to the connect request
    MELCloud.Status.ConnectRequest = false;
  }
}

void MQTTonDisconnect(void* response) {
  DEBUG_PRINTLN("MQTT Disconnect");
}


void MQTTonData(char* topic, byte* payload, unsigned int length) {
  payload[length] = 0;
  String Topic = topic;
  String Payload = (char*)payload;

  DEBUG_PRINT("\nReceived MQTT Message on topic ");
  DEBUG_PRINT(Topic.c_str());
  DEBUG_PRINT(" with Payload ");
  DEBUG_PRINTLN(Payload.c_str());

  // QTY in use
  if (Topic == MQTTCommandControllerQTY) {
    unitSettings.Quantity = Payload.toInt();
    shouldSaveConfig = true;  // Write the data to JSON file so if device reboots it is saved
  }

  // RCs 1-8
  if (Topic == MQTTCommandControllerRC) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, Payload);
    if (error) {
      DEBUG_PRINT("Failed to read file: ");
      DEBUG_PRINTLN(error.c_str());
    } else {
      String type = doc["type"];

      // Zone 1
      if (type == "zone1") {
        float value = doc["value"];
      }
      // Zone 2
      if (type == "zone2") {
        float value = doc["value"];
      }
      // RC
      if (type == "rc") {
        int action = doc["action"];
        float value = doc["value"];
        RCInput[action - 1] = value;
        RCTemp[action - 1] = roundToNearestHalf(value);
      }
      // DHW
      if (type == "dhw") {}
      // HOL
      if (type == "hol") {}

      Report();
    }
  }
}

bool getHeatmiserTemperature(const char* host, uint16_t port) {
  WiFiClient client;  // Use a local instance instead of the Class name
  client.setTimeout(10000);

  DEBUG_PRINT("Connecting to: ");
  DEBUG_PRINTLN(host);

  if (!client.connect(host, port)) {
    DEBUG_PRINTLN("!!! Connection to Neo Hub failed !!!");
    return false;
  }

  DEBUG_PRINTLN("Connected! Sending Command...");

  // Send command properly
  client.print("{\"GET_LIVE_DATA\":0}");
  client.write('\0');

  DEBUG_PRINTLN("Command Sent. Waiting for response...");

  unsigned long startWait = millis();
  while (client.available() == 0 && millis() - startWait < 5000) {
    if (!client.connected()) {
      DEBUG_PRINTLN("Hub closed connection early.");
      break;
    }
    delay(10);
  }

  if (client.available() > 0) {
    DEBUG_PRINT("Data detected! Bytes available: ");
    DEBUG_PRINTLN(client.available());

    JsonDocument filter;
    filter["devices"][0]["ZONE_NAME"] = true;
    filter["devices"][0]["ACTUAL_TEMP"] = true;
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, client, DeserializationOption::Filter(filter));

    if (error) {
      DEBUG_PRINT("JSON Parse failed: ");
      DEBUG_PRINTLN(error.f_str());
    } else {
      DEBUG_PRINTLN("JSON Parsed Successfully:");
      JsonArray devices = doc["devices"];
      serializeJson(devices, DEBUGPORT);
      DEBUG_PRINTLN("");
      for (JsonObject device : devices) {
        const char* zoneName = device["ZONE_NAME"] | "Unknown";

        // Note: ACTUAL_TEMP is a string in your JSON ("19.0")
        // .as<float>() usually handles string-to-float conversion
        float temp = device["ACTUAL_TEMP"].as<float>();

        bool matched = false;  // Reset flag for each device in the array

        if (zoneName != nullptr && mqttSettings.z1_name[0] != '\0') {
          if (strcmp(zoneName, mqttSettings.z1_name) == 0) {
            RCInput[0] = temp;
            RCTemp[0] = roundToNearestHalf(temp);
            DEBUG_PRINT("Zone 1 Match: ");
            DEBUG_PRINT(zoneName);
            DEBUG_PRINT(" at: ");
            DEBUG_PRINTLN(temp);
            matched = true;
          }
        }
        if (zoneName != nullptr && mqttSettings.z2_name[0] != '\0') {
          if (strcmp(zoneName, mqttSettings.z2_name) == 0) {
            RCInput[1] = temp;
            RCTemp[1] = roundToNearestHalf(temp);
            DEBUG_PRINT("Zone 2 Match: ");
            DEBUG_PRINT(zoneName);
            DEBUG_PRINT(" at: ");
            DEBUG_PRINTLN(temp);
            matched = true;
          }
        }
        if (!matched) {
          DEBUG_PRINT("Skipping Zone: ");
          DEBUG_PRINTLN(zoneName);
        }
      }
    }
  } else {
    DEBUG_PRINTLN("Timed out waiting for data.");
  }
  client.stop();
  return true;
}


void RequestHeatmiserData(void) {
  String discoveredIP = "";
  if (strcmp(mqttSettings.heatmiser, "neohub.local") != 0 && strcmp(mqttSettings.heatmiser, "") != 0 && WiFi.status() == WL_CONNECTED) {
    DEBUG_PRINTLN("Requesting Heatmiser Data...");
    if (!getHeatmiserTemperature(mqttSettings.heatmiser, 4242)) {
      startDiscovery();
    }
  } else {
    DEBUG_PRINTLN("No Heatmiser Set and/or no WiFi Connection");
  }
}

void Report(void) {
  JsonDocument doc;
  char Buffer[2048];

  uint8_t Power, SystemOpMode, Zone1ControlMode, Zone2ControlMode, TimerProhibit, Zone1ActiveInput, Zone2ActiveInput, ErrorCode, DHWForce, Holiday, ErrorCodeNum;


  doc[F("QTY")] = unitSettings.Quantity;
  doc[F("Power")] = SystemPowerModeString[HeatPump.Status.Power];
  doc[F("SystemOperationMode")] = SystemOperationModeString[HeatPump.Status.SystemOpMode];
  doc[F("Zone1OperationMode")] = HeatingControlModeString[HeatPump.Status.Zone1ControlMode];
  doc[F("Zone2OperationMode")] = HeatingControlModeString[HeatPump.Status.Zone2ControlMode];
  doc[F("TimerProhibit")] = TimerModeString[HeatPump.Status.TimerProhibit];
  doc[F("Z1ActiveInput")] = HeatPump.Status.Zone1ActiveInput;
  doc[F("Z2ActiveInput")] = HeatPump.Status.Zone2ActiveInput;
  doc[F("Z1Setpoint")] = HeatPump.Status.SetpointZ1;
  doc[F("Z2Setpoint")] = HeatPump.Status.SetpointZ2;
  doc[F("ErrorCode")] = HeatPump.Status.ErrorCode;
  doc[F("DHWForce")] = OFF_ON_String[HeatPump.Status.DHWForce];
  doc[F("Holiday")] = OFF_ON_String[HeatPump.Status.Holiday];
  doc[F("HolidayHrs")] = 0;  // Not identified yet

  doc[F("RC1Input")] = RCInput[0];
  doc[F("RC2Input")] = RCInput[1];
  doc[F("RC3Input")] = RCInput[2];
  doc[F("RC4Input")] = RCInput[3];
  doc[F("RC5Input")] = RCInput[4];
  doc[F("RC6Input")] = RCInput[5];
  doc[F("RC7Input")] = RCInput[6];
  doc[F("RC8Input")] = RCInput[7];

  doc[F("HB_ID")] = Heart_Value;

  serializeJson(doc, Buffer);

  MQTTClient1.publish(MQTT_STATUS_TEMP.c_str(), Buffer, false);
  MQTTClient1.publish(MQTT_LWT.c_str(), "online");
}

void StatusReport(void) {
  JsonDocument doc;
  char Buffer[1024];

  doc[F("SSID")] = WiFi.SSID();
  doc[F("RSSI")] = WiFi.RSSI();
#ifdef ARDUINO_WT32_ETH01
  doc[F("IP")] = ETH.localIP().toString();
#else
  doc[F("IP")] = WiFi.localIP().toString();
#endif
  doc[F("Firmware")] = FirmwareVersion;
#ifdef ESP32  // Define the M5Stack LED
  doc[F("CPUTemp")] = round2(temperatureRead());
#endif
#ifdef ESP8266  // Define the M5Stack LED
  doc[F("CPUTemp")] = "None";
#endif
  doc[F("CPULoopTime")] = CPULoopSpeed;
  doc[F("FTCLoopTime")] = FTCLoopSpeed;
  doc[F("FTCReplyTime")] = HeatPump.Lastmsbetweenmsg();

  doc[F("HB_ID")] = Heart_Value;

  serializeJson(doc, Buffer);
  MQTTClient1.publish(MQTT_STATUS_WIFISTATUS.c_str(), Buffer, false);
  MQTTClient1.publish(MQTT_LWT.c_str(), "online");
}

void PublishAllReports(void) {
  // Increment the Heatbeat ID Counter
  ++Heart_Value;
  if (Heart_Value > Heartbeat_Range) {
    Heart_Value = 1;
  }

  Report();
  StatusReport();

  FlashGreenLED();
  DEBUG_PRINTLN("MQTT Published!");
}



void FlashGreenLED(void) {
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
  myLED.setPixel(0, L_GREEN, 1);
  myLED.brightness(255, 1);
#endif
#ifdef ESP8266                        // Define the Witty ESP8266 Ports
  digitalWrite(Green_RGB_LED, HIGH);  // Flash the Green LED full brightness
#endif
  delay(10);                       // Hold for 10ms then WiFi brightness will return it to 25%
#ifdef ESP8266                     // Define the Witty ESP8266 Ports
  analogWrite(Green_RGB_LED, 30);  // Green LED on, 25% brightness
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
  myLED.brightness(LED_BRIGHT, 1);
#endif
}

void setupTelnet() {
  TelnetServer.onConnect(onTelnetConnect);
  TelnetServer.onConnectionAttempt(onTelnetConnectionAttempt);
  TelnetServer.onReconnect(onTelnetReconnect);
  TelnetServer.onDisconnect(onTelnetDisconnect);
}

void startTelnet() {
  DEBUG_PRINT(F("Telnet: "));
#ifdef ARDUINO_WT32_ETH01
  if (TelnetServer.begin(23, false)) {
#else
  if (TelnetServer.begin()) {
#endif
    DEBUG_PRINTLN(F("Running"));
  } else {
    DEBUG_PRINTLN(F("error."));
  }
}

void stopTelnet() {
  DEBUG_PRINTLN(F("Stopping Telnet"));
  TelnetServer.stop();
}

void onTelnetConnect(String ip) {
  DEBUG_PRINT(F("Telnet: "));
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(F(" connected"));
  TelnetServer.println("\nWelcome " + TelnetServer.getIP());
  TelnetServer.println(F("(Use ^] + q  to disconnect.)"));
}

void onTelnetDisconnect(String ip) {
  DEBUG_PRINT(F("Telnet: "));
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(F(" disconnected"));
}

void onTelnetReconnect(String ip) {
  DEBUG_PRINT(F("Telnet: "));
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(F(" reconnected"));
}

void onTelnetConnectionAttempt(String ip) {
  DEBUG_PRINT(F("Telnet: "));
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(F(" tried to connected"));
}



void CheckForOTAUpdates(void) {
  // Perform HTTP request to get Target Version
  bool updateAvailable = false;
  String targetversion, betaversion;

  DEBUG_PRINT(F("Checking for Firmware Updates..."));

  http.begin(F("https://witty.house/CNRF/update.json"), witty_house_root_ca);  //HTTPS

  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been sent and Server response header has been handled
    if (httpCode == HTTP_CODE_OK) {
      DEBUG_PRINTLN(F(" OK"));
      String payload = http.getString();

      JsonDocument doc;
      deserializeJson(doc, payload);
      String tv = doc["target"];
      String bv = doc["beta"];
      bool beta_active = doc["beta_active"];
      targetversion = tv;
      betaversion = bv;

      DEBUG_PRINT(F("Actual Firmware Version: "));
      DEBUG_PRINT(FirmwareVersion);
      DEBUG_PRINT(F(", Target Firmware Version: "));
      DEBUG_PRINT(targetversion);
      DEBUG_PRINT(F(", Beta Firmware Version: "));
      DEBUG_PRINT(betaversion);
      DEBUG_PRINT(F(", Beta Active: "));
      DEBUG_PRINTLN(beta_active);

      if (targetversion != FirmwareVersion) {
        updateAvailable = true;
      } else {
        DEBUG_PRINTLN(F("No new firmware updates"));
      }

      if (betaversion == FirmwareVersion && beta_active) {
        DEBUG_PRINTLN(F("Beta Version - Skipping Updates"));
        updateAvailable = false;
      }
    }
  } else {
    DEBUG_PRINT(F(" Failed - Error: "));
    DEBUG_PRINT(httpCode);
    DEBUG_PRINTLN(http.errorToString(httpCode));
  }

  http.end();

  if (updateAvailable) {  // If different, download the target file
    DEBUG_PRINTLN(F("Firmware Update Available! Starting..."));
#ifdef ARDUINO_WT32_ETH01
    String TargetURL = "https://witty.house/CNRF/" + targetversion + "e.bin";
#endif
#ifdef ARDUINO_M5STACK_ATOMS3
    String TargetURL = "https://witty.house/CNRF/" + targetversion + "w.bin";
#endif
    HttpsOTA.begin(TargetURL.c_str(), witty_house_root_ca);
  }
}


float roundToNearestHalf(float value) {
  // Multiply by 2 to get whole numbers for 0.5 increments
  float temp = value * 2.0;

  // Use round() function to round to nearest integer
  int rounded = round(temp);

  // Divide by 2 to get back to original scale
  return static_cast<float>(rounded) / 2.0;
}

float roundToOneDecimal(float value) {
  return (round(value * 10.0) / 10.0);
}

double round0(double value) {
  return (int)(value);
}

double round1(double value) {
  return (int)(value * 10 + 0.5) / 10.0;
}

double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
}

String decimalToBinary(int decimal) {
  String binary = "";

  for (int i = 0; i < 8; i++) {  // 8 bits for a byte
    binary += (decimal >> i) & 1 ? '1' : '0';
  }

  return binary;
}


void MQTTWriteReceived(String message, int MsgNumber) {
  DEBUG_PRINTLN(message);
  WriteInProgress = true;  // Wait For OK
}

void HttpEvent(HttpEvent_t* event) {
  switch (event->event_id) {
    case HTTP_EVENT_ERROR: DEBUG_PRINTLN("Http Event Error"); break;
    case HTTP_EVENT_ON_CONNECTED: DEBUG_PRINTLN("Http Event On Connected"); break;
    case HTTP_EVENT_HEADER_SENT: DEBUG_PRINTLN("Http Event Header Sent"); break;
    case HTTP_EVENT_ON_HEADER:
      DEBUG_PRINT("Http Event On Header, key=");
      DEBUG_PRINT(event->header_key);
      DEBUG_PRINT(" value=");
      DEBUG_PRINTLN(event->header_value);
      break;
    case HTTP_EVENT_ON_DATA: break;
    case HTTP_EVENT_ON_FINISH: DEBUG_PRINTLN("Http Event On Finish"); break;
    case HTTP_EVENT_DISCONNECTED: DEBUG_PRINTLN("Http Event Disconnected"); break;
    case HTTP_EVENT_REDIRECT: DEBUG_PRINTLN("Http Event Redirect"); break;
  }
}

void runNonBlockingDiscovery() {
  // If we aren't supposed to be looking, just exit immediately
  if (currentDiscoveryState == IDLE) return;

  // Check if we have timed out
  if (millis() - discoveryStartTime > discoveryTimeout) {
    DEBUG_PRINTLN("Discovery timed out.");
    udp.stop();
    currentDiscoveryState = IDLE;
    return;
  }

  // Check if a UDP packet has arrived
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, packetBuffer);

    if (!error && doc.containsKey("ip")) {
      String hubIP = doc["ip"].as<String>();
      DEBUG_PRINT("Found NeoHub at: ");
      DEBUG_PRINTLN(hubIP);

      // Copy to your char array storage
      hubIP.toCharArray(mqttSettings.heatmiser, sizeof(mqttSettings.heatmiser));

      // Update your live tracking variable
      custom_hm_server.setValue(hubIP.c_str(), hostname_max_length);
      shouldSaveConfig = true;

      udp.stop();
      currentDiscoveryState = IDLE;  // We are done!
      return;
    }
  }
}

// Helper to start the process
void startDiscovery() {
  if (currentDiscoveryState == IDLE) {
    udp.begin(discoveryPort);
    discoveryStartTime = millis();
    currentDiscoveryState = LISTENING;
    DEBUG_PRINTLN("Starting non-blocking discovery...");
  }
}

#ifdef ARDUINO_WT32_ETH01
// WARNING: onEvent is called from a separate FreeRTOS task (thread)!
void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      DEBUG_PRINTLN("ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname("Ecodan-CNRF-Bridge");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED:
      DEBUG_PRINTLN("ETH Connected");
      break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      DEBUG_PRINTLN("ETH Got IP");
      DEBUG_PRINTLN(ETH);
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      DEBUG_PRINTLN("ETH Lost IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      DEBUG_PRINTLN("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      DEBUG_PRINTLN("ETH Stopped");
      eth_connected = false;
      break;
    default: break;
  }
}
#endif

#endif