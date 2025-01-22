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
/* As sold Witty ESP8266 based               / Core 3.1.2 / Flash 4MB (1MB FS / 1MB OTA)                        */
/* ESP32 AtomS3 Lite (ESP32S3 Dev Module)    / Core 3.0.7 / Flash 4M with SPIFFS (1.2MB APP / 1.5MB SPIFFS)    */
/* ESP32 Ethernet WT32-ETH01                 / Core 3.0.7 / Flash 4MB (1.2MB APP / 1.5MB SPIFFS)                */


#if defined(ESP8266) || defined(ESP32)  // ESP32 or ESP8266 Compatiability

#include <FS.h>  // Define File System First
#include <LittleFS.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#endif
#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#endif
#ifdef ARDUINO_WT32_ETH01
#include <ETH.h>
#include <Arduino.h>
#endif


extern float RCTemp[8];
extern int ControllerQTY;


#include <DNSServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESPTelnet.h>
#include "Ecodan.h"

String FirmwareVersion = "1.0.0";


#ifdef ESP8266  // Define the Witty ESP8266 Serial Pins
#define HEATPUMP_STREAM SwSerial1
#define SERIAL_CONFIG SWSERIAL_8E1
int LDR = A0;
int Activity_LED = 2;
int Reset_Button = 4;
int Green_RGB_LED = 12;
int Blue_RGB_LED = 13;
#define FTCCable_RxPin 14
int Red_RGB_LED = 15;
#define FTCCable_TxPin 16
#endif

#ifdef ESP32  // Define the M5Stack Serial Pins
#define HEATPUMP_STREAM Serial1
#define SERIAL_CONFIG SERIAL_8E1

#ifdef ARDUINO_M5STACK_ATOMS3
#include <FastLED.h>
#define FASTLED_FORCE_NAMESPACE
#define FASTLED_INTERNAL
#define NUM_LEDS 1
#define DATA_PIN 35
CRGB leds[NUM_LEDS];
int Reset_Button = 41;
#define FTCCable_RxPin 2
#define FTCCable_TxPin 1
#endif

#ifdef ARDUINO_WT32_ETH01
#define FTCCable_RxPin 4
#define FTCCable_TxPin 2
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

bool oneshot_connect = false;

const int deviceId_max_length = 15;
const int clientId_max_length = 25;
const int hostname_max_length = 200;
const int port_max_length = 10;
const int user_max_length = 30;
const int password_max_length = 50;
const int basetopic_max_length = 30;


// The extra parameters to be configured (can be either global or just in the setup)
// After connecting, parameter.getValue() will get you the configured value
// id/name placeholder/prompt default length
// Here you can pre-set the settings for the MQTT connection. The settings can later be changed via Wifi Manager.
struct MqttSettings {
  // These are the placeholder objects for the custom fields
  char deviceId[deviceId_max_length] = "000000000000";
  char wm_device_id_identifier[15] = "device_id";

  // Client 1
  char hostname[hostname_max_length] = "IPorDNS";
  char user[user_max_length] = "Username";
  char password[password_max_length] = "Password";
  char port[port_max_length] = "1883";
  char baseTopic[basetopic_max_length] = "Ecodan/CNRF";
  char clientId[clientId_max_length] = "EcodanBridge";
  char wm_mqtt_hostname_identifier[40] = "mqtt_hostname";
  char wm_mqtt_user_identifier[20] = "mqtt_user";
  char wm_mqtt_password_identifier[30] = "mqtt_password";
  char wm_mqtt_port_identifier[10] = "mqtt_port";
  char wm_mqtt_basetopic_identifier[20] = "mqtt_basetopic";
  char wm_mqtt_client_id_identifier[20] = "mqtt_client_id";

  // Client 2
  char hostname2[hostname_max_length] = "IPorDNS";
  char user2[user_max_length] = "Username";
  char password2[password_max_length] = "Password";
  char port2[port_max_length] = "1883";
  char baseTopic2[basetopic_max_length] = "Ecodan/ASHP";
  char clientId2[clientId_max_length] = "EcodanBridge";
  char wm_mqtt2_hostname_identifier[40] = "mqtt2_hostname";
  char wm_mqtt2_user_identifier[20] = "mqtt2_user";
  char wm_mqtt2_password_identifier[30] = "mqtt2_password";
  char wm_mqtt2_port_identifier[11] = "mqtt2_port";
  char wm_mqtt2_basetopic_identifier[20] = "mqtt2_basetopic";
  char wm_mqtt2_client_id_identifier[20] = "mqtt2_client_id";
};

struct UnitSettings {
  int Quantity = 2;
  char qty_identifier[4] = "qty";
};

MqttSettings mqttSettings;
UnitSettings unitSettings;
ECODAN HeatPump;
#ifdef ESP8266
SoftwareSerial SwSerial1;
#endif
WiFiClient NetworkClient;
PubSubClient MQTTClient1(NetworkClient);
PubSubClient MQTTClient2(NetworkClient);
ESPTelnet TelnetServer;
WiFiManager wifiManager;


// Delcare Global Scope for Non-Blocking, always active Portal with "TEMP" placeholder, real values populated later from filesystem
WiFiManagerParameter custom_mqtt_server("server", "<b>Required</b> Primary MQTT Server", "TEMP", hostname_max_length);
WiFiManagerParameter custom_mqtt_user("user", "Primary MQTT Username", "TEMP", user_max_length);
WiFiManagerParameter custom_mqtt_pass("pass", "Primary MQTT Password", "TEMP", password_max_length);
WiFiManagerParameter custom_mqtt_port("port", "Primary MQTT Server Port (Default: 1883)", "TEMP", port_max_length);
WiFiManagerParameter custom_mqtt_client_id("client_id", "Primary MQTT Client ID (Default: EcodanBridge-deviceID)<br><font size='0.8em'>Unique ID when connecting to MQTT</font>", "TEMP", clientId_max_length);
WiFiManagerParameter custom_mqtt_basetopic("basetopic", "Primary MQTT Base Topic (Default: Ecodan/ASHP)<br><font size='0.8em'>Modify if you have multiple heat pumps connecting to the same MQTT server</font>", "TEMP", basetopic_max_length);
WiFiManagerParameter custom_mqtt2_server("server2", "<hr><b>Optional</b> Secondary MQTT Server<br><font size='0.8em'>You can send data to a second MQTT broker, <b>leave default or blank if not in use</b></font>", "TEMP", hostname_max_length);
WiFiManagerParameter custom_mqtt2_user("user2", "Secondary MQTT Username", "TEMP", user_max_length);
WiFiManagerParameter custom_mqtt2_pass("pass2", "Secondary MQTT Password", "TEMP", password_max_length);
WiFiManagerParameter custom_mqtt2_port("port2", "Secondary MQTT Server Port", "TEMP", port_max_length);
WiFiManagerParameter custom_mqtt2_client_id("client_id2", "Secondary MQTT Client ID", "TEMP", clientId_max_length);
WiFiManagerParameter custom_mqtt2_basetopic("basetopic2", "Secondary MQTT Base Topic", "TEMP", basetopic_max_length);
WiFiManagerParameter custom_device_id("device_id", "<hr>Device ID<br><font size='0.8em'>Only modify if upgrading or changing hardware, copy your previous device ID over</font>", "TEMP", deviceId_max_length);


#include "TimerCallBack.h"
#include "Debug.h"
#include "MQTTDiscovery.h"
#include "MQTTConfig.h"


void HeatPumpQueryStateEngine(void);
void HeatPumpKeepAlive(void);
void Report(void);
void StatusReport(void);
void PublishAllReports(void);


TimerCallBack HeatPumpQuery1(500, HeatPumpQueryStateEngine);  // Set to 400ms (Safe), 320-350ms best time between messages
TimerCallBack HeatPumpQuery2(5000, HeatPumpKeepAlive);        // Set to 20-30s for heat pump query frequency
TimerCallBack HeatPumpQuery3(30000, handleMQTTState);         // Re-connect attempt timer if MQTT is not online
TimerCallBack HeatPumpQuery4(30000, handleMQTT2State);        // Re-connect attempt timer if MQTT Stream 2 is not online
TimerCallBack HeatPumpQuery5(30000, PublishAllReports);       // Set to 20-30s for heat pump query frequency


unsigned long looppreviousMillis = 0;    // variable for comparing millis counter
unsigned long ftcpreviousMillis = 0;     // variable for comparing millis counter
unsigned long wifipreviousMillis = 0;    // variable for comparing millis counter
unsigned long ftcconpreviousMillis = 0;  // variable for comparing millis counter
int FTCLoopSpeed, CPULoopSpeed;          // variable for holding loop time in ms
bool WiFiOneShot = true;
bool FTCOneShot = true;
bool FTCOneShot1 = false;
bool FTCOneShot2 = false;
bool CableConnected = true;
bool WiFiConnectedLastLoop = false;
float RCInput[7];

#ifdef ARDUINO_WT32_ETH01
static bool eth_connected = false;
#endif


void setup() {
  WiFi.mode(WIFI_STA);         // explicitly set mode, esp defaults to STA+AP
  DEBUGPORT.begin(DEBUGBAUD);  // Start Debug

  HEATPUMP_STREAM.begin(SERIAL_BAUD, SERIAL_CONFIG, FTCCable_RxPin, FTCCable_TxPin);  // Rx, Tx
  HeatPump.SetStream(&HEATPUMP_STREAM);


#ifdef ARDUINO_WT32_ETH01
  Network.onEvent(onEvent);
  ETH.begin();
#endif

#ifndef ARDUINO_WT32_ETH01
  pinMode(Reset_Button, INPUT);  // Pushbutton on other modules
#endif


// -- Lights for ESP8266 and ESP32 -- //
#ifdef ARDUINO_M5STACK_ATOMS3                              // Define the M5Stack LED
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // ESP32 M5 Stack Atom S3
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
  if (shouldSaveConfig) {
    saveConfig();
  }
  setupTelnet();
  startTelnet();

  MQTTClient1.setBufferSize(2048);  // Increase MQTT Buffer Size
  MQTTClient2.setBufferSize(2048);  // Increase MQTT Buffer Size

  RecalculateMQTTTopics();
  RecalculateMQTT2Topics();

  initializeMQTTClient1();
  MQTTClient1.setCallback(MQTTonData);
  handleMQTTState();

  initializeMQTT2Client();
  MQTTClient2.setCallback(MQTTonData);
  handleMQTT2State();


  wifiManager.startWebPortal();
  for (int i = 0; i < 8; i++) {
    RCInput[i] = 18.0;
    RCTemp[i] = RCInput[i];
  }
}


void loop() {
  // -- Loop Start -- //
  looppreviousMillis = micros();  // Loop Speed Check

  ControllerQTY = unitSettings.Quantity;

  // -- Process Handlers -- //
  HeatPumpQuery1.Process();
  HeatPumpQuery2.Process();
  HeatPumpQuery3.Process();
  HeatPumpQuery4.Process();
  HeatPumpQuery5.Process();

  MQTTClient1.loop();
  MQTTClient2.loop();
  TelnetServer.loop();
  HeatPump.Process();

  wifiManager.process();

  // -- Config Saver -- //
  if (shouldSaveConfig) { saveConfig(); }  // Handles WiFiManager Settings Changes

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
      leds[0] = CRGB::Red;     // Turn the Red LED On
      FastLED.setBrightness(255);
      FastLED.show();
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
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
      leds[0] = CRGB::Red;     // Flash the Red LED
      FastLED.setBrightness(255);
      FastLED.show();
      delay(500);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(500);
      FastLED.setBrightness(255);
      FastLED.show();
      delay(500);
      FastLED.setBrightness(0);
      FastLED.show();
      delay(500);
      FastLED.setBrightness(255);
      FastLED.show();
      ESP.restart();
#endif
    }  // Wait for 5 mins to try reconnects then force restart
    WiFiConnectedLastLoop = false;
  } else if (WiFi.status() != WL_CONNECTED && wifiManager.getConfigPortalActive()) {
#ifdef ESP8266                         // Define the Witty ESP8266 Ports
    digitalWrite(Blue_RGB_LED, HIGH);  // Turn the Blue LED Off
    analogWrite(Green_RGB_LED, LOW);   // Green LED on, 25% brightness
    digitalWrite(Red_RGB_LED, LOW);    // Turn the Red LED Off
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
    leds[0] = CRGB::Blue;
    FastLED.setBrightness(255);  // LED on, reduced brightness
    FastLED.show();
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
      leds[0] = CRGB::Green;
      FastLED.setBrightness(100);  // LED on, reduced brightness
      FastLED.show();
#endif
    }
    WiFiConnectedLastLoop = true;
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
    ESP.reset();
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
    leds[0] = CRGB::Red;       // Flash the Red LED
    FastLED.show();
    FastLED.setBrightness(255);
    FastLED.show();
    delay(500);
    FastLED.setBrightness(0);
    FastLED.show();
    delay(500);
    FastLED.setBrightness(255);
    FastLED.show();
    delay(500);
    FastLED.setBrightness(0);
    FastLED.show();
    delay(500);
    FastLED.setBrightness(255);
    FastLED.show();
    delay(500);
    ESP.restart();  // No button on ETH
#endif
  }
#endif


  // -- CPU Loop Time End -- //
  CPULoopSpeed = micros() - looppreviousMillis;  // Loop Speed End Monitor
}

void HeatPumpKeepAlive(void) {
  if (!HeatPump.HeatPumpConnected()) {
    DEBUG_PRINTLN("Heat Pump Disconnected");
  }
  ftcpreviousMillis = millis();
  HeatPump.TriggerStatusStateMachine();
}

void HeatPumpQueryStateEngine(void) {
  HeatPump.StatusStateMachine();  // Full Read trigged by CurrentMessage

  // Call Once Full Update is complete
  if (HeatPump.UpdateComplete()) {
    DEBUG_PRINTLN("Update Complete");
    FTCLoopSpeed = millis() - ftcpreviousMillis;  // Loop Speed End
    Report();
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
  if ((Topic == MQTTCommandControllerQTY) || (Topic == MQTTCommand2ControllerQTY)) {
    unitSettings.Quantity = Payload.toInt();
    shouldSaveConfig = true;  // Write the data to JSON file so if device reboots it is saved
  }

  // RCs 1-8
  if ((Topic == MQTTCommandController1Current) || (Topic == MQTTCommand2Controller1Current)) {
    RCInput[0] = Payload.toFloat();
    RCTemp[0] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController2Current) || (Topic == MQTTCommand2Controller2Current)) {
    RCInput[1] = Payload.toFloat();
    RCTemp[1] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController3Current) || (Topic == MQTTCommand2Controller3Current)) {
    RCInput[2] = Payload.toFloat();
    RCTemp[2] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController4Current) || (Topic == MQTTCommand2Controller4Current)) {
    RCInput[3] = Payload.toFloat();
    RCTemp[3] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController5Current) || (Topic == MQTTCommand2Controller5Current)) {
    RCInput[4] = Payload.toFloat();
    RCTemp[4] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController6Current) || (Topic == MQTTCommand2Controller6Current)) {
    RCInput[5] = Payload.toFloat();
    RCTemp[5] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController7Current) || (Topic == MQTTCommand2Controller7Current)) {
    RCInput[6] = Payload.toFloat();
    RCTemp[6] = roundToNearestHalf(Payload.toFloat());
  }
  if ((Topic == MQTTCommandController8Current) || (Topic == MQTTCommand2Controller8Current)) {
    RCInput[7] = Payload.toFloat();
    RCTemp[7] = roundToNearestHalf(Payload.toFloat());
  }

  Report();
}


void Report(void) {
  StaticJsonDocument<1024> doc;
  char Buffer[1024];

  doc[F("QTY")] = unitSettings.Quantity;
  doc[F("Z1ActiveInput")] = HeatPump.Status.Zone1ActiveInput;
  doc[F("Z2ActiveInput")] = HeatPump.Status.Zone2ActiveInput;
  doc[F("Z1Setpoint")] = HeatPump.Status.SetpointZ1;
  doc[F("Z2Setpoint")] = HeatPump.Status.SetpointZ2;
  doc[F("ErrorCode")] = HeatPump.Status.ErrorCode;

  doc[F("RC1Input")] = RCInput[0];
  doc[F("RC1Rounded")] = RCTemp[0];
  doc[F("RC2Input")] = RCInput[1];
  doc[F("RC2Rounded")] = RCTemp[1];
  doc[F("RC3Input")] = RCInput[2];
  doc[F("RC3Rounded")] = RCTemp[2];
  doc[F("RC4Input")] = RCInput[3];
  doc[F("RC4Rounded")] = RCTemp[3];
  doc[F("RC5Input")] = RCInput[4];
  doc[F("RC5Rounded")] = RCTemp[4];
  doc[F("RC6Input")] = RCInput[5];
  doc[F("RC6Rounded")] = RCTemp[5];
  doc[F("RC7Input")] = RCInput[6];
  doc[F("RC7Rounded")] = RCTemp[6];
  doc[F("RC8Input")] = RCInput[7];
  doc[F("RC8Rounded")] = RCTemp[7];

  doc[F("HB_ID")] = Heart_Value;

  serializeJson(doc, Buffer);

  MQTTClient1.publish(MQTT_STATUS_TEMP.c_str(), Buffer, false);
  MQTTClient2.publish(MQTT_2_STATUS_TEMP.c_str(), Buffer, false);
}

void StatusReport(void) {
  StaticJsonDocument<512> doc;
  char Buffer[512];

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
  MQTTClient2.publish(MQTT_2_STATUS_WIFISTATUS.c_str(), Buffer, false);
  MQTTClient1.publish(MQTT_LWT.c_str(), "online");
  MQTTClient2.publish(MQTT_2_LWT.c_str(), "online");
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
  FastLED.setBrightness(255);
  FastLED.show();
#endif
#ifdef ESP8266                        // Define the Witty ESP8266 Ports
  digitalWrite(Green_RGB_LED, HIGH);  // Flash the Green LED full brightness
#endif
  delay(10);                       // Hold for 10ms then WiFi brightness will return it to 25%
#ifdef ESP8266                     // Define the Witty ESP8266 Ports
  analogWrite(Green_RGB_LED, 30);  // Green LED on, 25% brightness
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
  FastLED.setBrightness(100);
  FastLED.show();
#endif
}

void setupTelnet() {
  TelnetServer.onConnect(onTelnetConnect);
  TelnetServer.onConnectionAttempt(onTelnetConnectionAttempt);
  TelnetServer.onReconnect(onTelnetReconnect);
  TelnetServer.onDisconnect(onTelnetDisconnect);
}

void startTelnet() {
  DEBUG_PRINT("Telnet: ");
  if (TelnetServer.begin()) {
    DEBUG_PRINTLN("Running");
  } else {
    DEBUG_PRINTLN("error.");
  }
}

void stopTelnet() {
  DEBUG_PRINTLN("Stopping Telnet");
  TelnetServer.stop();
}

void onTelnetConnect(String ip) {
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" connected");
  TelnetServer.println("\nWelcome " + TelnetServer.getIP());
  TelnetServer.println("(Use ^] + q  to disconnect.)");
}

void onTelnetDisconnect(String ip) {
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" disconnected");
}

void onTelnetReconnect(String ip) {
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" reconnected");
}

void onTelnetConnectionAttempt(String ip) {
  DEBUG_PRINT("Telnet: ");
  DEBUG_PRINT(ip);
  DEBUG_PRINTLN(" tried to connected");
}

double round2(double value) {
  return (int)(value * 100 + 0.5) / 100.0;
}

float roundToNearestHalf(float value) {
  // Multiply by 2 to get whole numbers for 0.5 increments
  float temp = value * 2.0;

  // Use round() function to round to nearest integer
  int rounded = round(temp);

  // Divide by 2 to get back to original scale
  return static_cast<float>(rounded) / 2.0;
}

#ifdef ARDUINO_WT32_ETH01
// WARNING: onEvent is called from a separate FreeRTOS task (thread)!
void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      DEBUG_PRINTLN("ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname("Ecodan-Bridge");
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