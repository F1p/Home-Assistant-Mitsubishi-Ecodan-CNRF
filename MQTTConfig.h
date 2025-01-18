String MQTT_BASETOPIC = "Ecodan/CNRF";

String MQTT_LWT = MQTT_BASETOPIC + "/LWT";
String MQTT_STATUS = MQTT_BASETOPIC + "/Status";
String MQTT_COMMAND = MQTT_BASETOPIC + "/Command";

String MQTT_STATUS_TEMP = MQTT_STATUS + "/Temperature";
String MQTT_STATUS_WIFISTATUS = MQTT_STATUS + "/WiFiStatus";

String MQTT_COMMAND_C1 = MQTT_COMMAND + "/Controller1";
String MQTT_COMMAND_C2 = MQTT_COMMAND + "/Controller2";

String MQTTCommandController1Current = MQTT_COMMAND_C1;
String MQTTCommandController2Current = MQTT_COMMAND_C2;

String MQTT_2_BASETOPIC = "Ecodan/CNRF";

String MQTT_2_LWT = MQTT_2_BASETOPIC + "/LWT";
String MQTT_2_STATUS = MQTT_2_BASETOPIC + "/Status";
String MQTT_2_COMMAND = MQTT_2_BASETOPIC + "/Command";

String MQTT_2_STATUS_TEMP = MQTT_2_STATUS + "/Temperature";
String MQTT_2_STATUS_WIFISTATUS = MQTT_2_STATUS + "/WiFiStatus";

String MQTT_2_COMMAND_C1 = MQTT_2_COMMAND + "/Controller1";
String MQTT_2_COMMAND_C2 = MQTT_2_COMMAND + "/Controller2";

String MQTTCommand2Controller1Current = MQTT_2_COMMAND_C1;
String MQTTCommand2Controller2Current = MQTT_2_COMMAND_C2;



char snprintbuffer[41] = "";
char DeviceID[15] = "";
const char ClientPrefix[14] = "CNRFBridge-";
char WiFiHostname[26] = "";



// Programs

#if defined(ESP8266) || defined(ESP32)  // ESP32 or ESP8266 Compatiability
void readSettingsFromConfig() {
  // Clean LittleFS for testing
  //LittleFS.format();

  // Read configuration from LittleFS JSON
  DEBUG_PRINTLN("Mounting File System...");
#ifdef ESP8266
  if (LittleFS.begin()) {
#endif
#ifdef ESP32
    if (LittleFS.begin("/storage")) {
#endif
      DEBUG_PRINTLN("Mounted File System");
      if (LittleFS.exists("/config.json")) {
        //file exists, reading and loading
        DEBUG_PRINTLN("Reading config file");
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          DEBUG_PRINTLN("Opened config file");
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, configFile);
          if (error) {
            DEBUG_PRINT("Failed to read file: ");
            DEBUG_PRINTLN(error.c_str());
          } else {
            DEBUG_PRINTLN("Parsed JSON: ");
            serializeJson(doc, Serial);
            DEBUG_PRINTLN();

            // Build in safety check, otherwise ESP will crash out and you can't get back in
            if (doc.containsKey(mqttSettings.wm_device_id_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_device_id_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_device_id_identifier]) + 1) <= deviceId_max_length)) {
                strcpy(mqttSettings.deviceId, doc[mqttSettings.wm_device_id_identifier]);
              }
            } else {  // For upgrading from <5.3.1, create the entry
#ifdef ESP8266
              snprintf(snprintbuffer, deviceId_max_length, (String(ESP.getChipId(), HEX)).c_str());
#endif
#ifdef ESP32
              snprintf(snprintbuffer, deviceId_max_length, (String(ESP.getEfuseMac(), HEX)).c_str());
#endif
              strcpy(mqttSettings.deviceId, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_client_id_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_client_id_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_client_id_identifier]) + 1) <= clientId_max_length)) {
                strcpy(mqttSettings.clientId, doc[mqttSettings.wm_mqtt_client_id_identifier]);
              }
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_hostname_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_hostname_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_hostname_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.hostname, doc[mqttSettings.wm_mqtt_hostname_identifier]);
              }
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_port_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_port_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_port_identifier]) + 1) <= port_max_length)) {
                strcpy(mqttSettings.port, doc[mqttSettings.wm_mqtt_port_identifier]);
              }
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_user_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_user_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_user_identifier]) + 1) <= user_max_length)) {
                strcpy(mqttSettings.user, doc[mqttSettings.wm_mqtt_user_identifier]);
              }
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_password_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_password_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_password_identifier]) + 1) <= password_max_length)) {
                strcpy(mqttSettings.password, doc[mqttSettings.wm_mqtt_password_identifier]);
              }
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_basetopic_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt_basetopic_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt_basetopic_identifier]) + 1) <= basetopic_max_length)) {
                strcpy(mqttSettings.baseTopic, doc[mqttSettings.wm_mqtt_basetopic_identifier]);
                MQTT_BASETOPIC = mqttSettings.baseTopic;
              }
            }
            // MQTT Stream 2
            if (doc.containsKey(mqttSettings.wm_mqtt2_client_id_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_client_id_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_client_id_identifier]) + 1) <= clientId_max_length)) {
                strcpy(mqttSettings.clientId2, doc[mqttSettings.wm_mqtt2_client_id_identifier]);
              }
            } else {  // For upgrading from <6.0.0, create the entry
              snprintf(snprintbuffer, clientId_max_length, mqttSettings.clientId2);
              strcpy(mqttSettings.clientId2, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt2_hostname_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_hostname_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_hostname_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.hostname2, doc[mqttSettings.wm_mqtt2_hostname_identifier]);
              }
            } else {  // For upgrading from <6.0.0, create the entry
              snprintf(snprintbuffer, hostname_max_length, mqttSettings.hostname2);
              strcpy(mqttSettings.hostname2, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt2_port_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_port_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_port_identifier]) + 1) <= port_max_length)) {
                strcpy(mqttSettings.port2, doc[mqttSettings.wm_mqtt2_port_identifier]);
              }
            } else {  // For upgrading from <6.0.0, create the entry
              snprintf(snprintbuffer, port_max_length, mqttSettings.port2);
              strcpy(mqttSettings.port2, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt2_user_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_user_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_user_identifier]) + 1) <= user_max_length)) {
                strcpy(mqttSettings.user2, doc[mqttSettings.wm_mqtt2_user_identifier]);
              }
            } else {  // For upgrading from <6.0.0, create the entry
              snprintf(snprintbuffer, user_max_length, mqttSettings.user2);
              strcpy(mqttSettings.user2, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt2_password_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_password_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_password_identifier]) + 1) <= password_max_length)) {
                strcpy(mqttSettings.password2, doc[mqttSettings.wm_mqtt2_password_identifier]);
              }
            } else {  // For upgrading from <6.0.0, create the entry
              snprintf(snprintbuffer, password_max_length, mqttSettings.password2);
              strcpy(mqttSettings.password2, snprintbuffer);
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
            if (doc.containsKey(mqttSettings.wm_mqtt2_basetopic_identifier)) {
              if ((sizeof(doc[mqttSettings.wm_mqtt2_basetopic_identifier]) > 0) && ((sizeof(doc[mqttSettings.wm_mqtt2_basetopic_identifier]) + 1) <= basetopic_max_length)) {
                strcpy(mqttSettings.baseTopic2, doc[mqttSettings.wm_mqtt2_basetopic_identifier]);
                MQTT_2_BASETOPIC = mqttSettings.baseTopic2;
              }
            } else {  // For upgrading from <6.0.0, create the entry
              strcpy(mqttSettings.baseTopic2, mqttSettings.deviceId);
              MQTT_2_BASETOPIC = mqttSettings.baseTopic2;
              shouldSaveConfig = true;  // Save config after exit to update the file
            }
          }
        }
        configFile.close();
      } else {
        DEBUG_PRINTLN("No config file exists, using placeholder values");
        // Populate the Dynamic Variables (Device ID)
#ifdef ESP8266
        snprintf(DeviceID, deviceId_max_length, (String(ESP.getChipId(), HEX)).c_str());
#endif
#ifdef ESP32
        snprintf(DeviceID, deviceId_max_length, (String(ESP.getEfuseMac(), HEX)).c_str());
#endif
        strcpy(mqttSettings.deviceId, DeviceID);
        strcpy(mqttSettings.baseTopic2, DeviceID);  // Base topic 2 defaults to deviceID
        snprintf(snprintbuffer, 30, "%s%s", ClientPrefix, DeviceID);
        strcpy(mqttSettings.clientId, snprintbuffer);
        strcpy(mqttSettings.clientId2, snprintbuffer);
      }
    } else {
      DEBUG_PRINTLN("Failed to mount File System");
    }
  }



  void RecalculateMQTTTopics() {
    // The base topic may change via WiFi Manager
    MQTT_LWT = MQTT_BASETOPIC + "/LWT";
    MQTT_STATUS = MQTT_BASETOPIC + "/Status";
    MQTT_COMMAND = MQTT_BASETOPIC + "/Command";

    MQTT_STATUS_TEMP = MQTT_STATUS + "/Temperature";
    MQTT_STATUS_WIFISTATUS = MQTT_STATUS + "/WiFiStatus";

    MQTT_COMMAND_C1 = MQTT_COMMAND + "/Controller1";
    MQTT_COMMAND_C2 = MQTT_COMMAND + "/Controller2";

    MQTTCommandController1Current = MQTT_COMMAND_C1;
    MQTTCommandController2Current = MQTT_COMMAND_C2;
  }




  void saveConfig() {
    // Read MQTT Portal Values for save to file system
    DEBUG_PRINTLN("Copying Portal Values...");
    strcpy(mqttSettings.deviceId, custom_device_id.getValue());
    strcpy(mqttSettings.clientId, custom_mqtt_client_id.getValue());
    strcpy(mqttSettings.hostname, custom_mqtt_server.getValue());
    strcpy(mqttSettings.port, custom_mqtt_port.getValue());
    strcpy(mqttSettings.user, custom_mqtt_user.getValue());
    strcpy(mqttSettings.password, custom_mqtt_pass.getValue());
    strcpy(mqttSettings.baseTopic, custom_mqtt_basetopic.getValue());
    strcpy(mqttSettings.clientId2, custom_mqtt2_client_id.getValue());
    strcpy(mqttSettings.hostname2, custom_mqtt2_server.getValue());
    strcpy(mqttSettings.port2, custom_mqtt2_port.getValue());
    strcpy(mqttSettings.user2, custom_mqtt2_user.getValue());
    strcpy(mqttSettings.password2, custom_mqtt2_pass.getValue());
    strcpy(mqttSettings.baseTopic2, custom_mqtt2_basetopic.getValue());

    DEBUG_PRINT("Saving config... ");
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      DEBUG_PRINTLN("[FAILED] Unable to open config file for writing");
    } else {
      JsonDocument doc;
      doc[mqttSettings.wm_device_id_identifier] = mqttSettings.deviceId;
      doc[mqttSettings.wm_mqtt_client_id_identifier] = mqttSettings.clientId;
      doc[mqttSettings.wm_mqtt_hostname_identifier] = mqttSettings.hostname;
      doc[mqttSettings.wm_mqtt_port_identifier] = mqttSettings.port;
      doc[mqttSettings.wm_mqtt_user_identifier] = mqttSettings.user;
      doc[mqttSettings.wm_mqtt_password_identifier] = mqttSettings.password;
      doc[mqttSettings.wm_mqtt_basetopic_identifier] = mqttSettings.baseTopic;
      doc[mqttSettings.wm_mqtt2_client_id_identifier] = mqttSettings.clientId2;
      doc[mqttSettings.wm_mqtt2_hostname_identifier] = mqttSettings.hostname2;
      doc[mqttSettings.wm_mqtt2_port_identifier] = mqttSettings.port2;
      doc[mqttSettings.wm_mqtt2_user_identifier] = mqttSettings.user2;
      doc[mqttSettings.wm_mqtt2_password_identifier] = mqttSettings.password2;
      doc[mqttSettings.wm_mqtt2_basetopic_identifier] = mqttSettings.baseTopic2;

      if (serializeJson(doc, configFile) == 0) {
        DEBUG_PRINTLN("[FAILED]");
      } else {
        DEBUG_PRINTLN("[DONE]");
        serializeJson(doc, Serial);
        DEBUG_PRINTLN();
      }
    }
    configFile.close();
    shouldSaveConfig = false;
  }

  //callback notifying us of the need to save config
  void saveConfigCallback() {
    saveConfig();
  }

  void initializeWifiManager() {
    DEBUG_PRINTLN("Starting WiFi Manager");
    // Reset Wifi settings for testing
    //wifiManager.resetSettings();
    //wifiManager.setDebugOutput(true);
    wifiManager.setTitle("Ecodan Bridge CNRF");

    // Set or Update the values
    custom_device_id.setValue(mqttSettings.deviceId, deviceId_max_length);
    custom_mqtt_client_id.setValue(mqttSettings.clientId, clientId_max_length);
    custom_mqtt_server.setValue(mqttSettings.hostname, hostname_max_length);
    custom_mqtt_port.setValue(mqttSettings.port, port_max_length);
    custom_mqtt_user.setValue(mqttSettings.user, user_max_length);
    custom_mqtt_pass.setValue(mqttSettings.password, password_max_length);
    custom_mqtt_basetopic.setValue(mqttSettings.baseTopic, basetopic_max_length);
    custom_mqtt2_client_id.setValue(mqttSettings.clientId2, clientId_max_length);
    custom_mqtt2_server.setValue(mqttSettings.hostname2, hostname_max_length);
    custom_mqtt2_port.setValue(mqttSettings.port2, port_max_length);
    custom_mqtt2_user.setValue(mqttSettings.user2, user_max_length);
    custom_mqtt2_pass.setValue(mqttSettings.password2, password_max_length);
    custom_mqtt2_basetopic.setValue(mqttSettings.baseTopic2, basetopic_max_length);

    // Add the custom MQTT parameters here
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_basetopic);
    wifiManager.addParameter(&custom_mqtt_client_id);
    wifiManager.addParameter(&custom_mqtt2_server);
    wifiManager.addParameter(&custom_mqtt2_user);
    wifiManager.addParameter(&custom_mqtt2_pass);
    wifiManager.addParameter(&custom_mqtt2_port);
    wifiManager.addParameter(&custom_mqtt2_basetopic);
    wifiManager.addParameter(&custom_mqtt2_client_id);
    wifiManager.addParameter(&custom_device_id);

    //set minimum quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();

    snprintf(WiFiHostname, 26, "%s%s", ClientPrefix, mqttSettings.deviceId);
    WiFi.hostname(WiFiHostname);
#ifdef ESP8266                         // Define the Witty ESP8266 Ports
    digitalWrite(Blue_RGB_LED, HIGH);  // Turn the Blue LED Off
#endif
    wifiManager.setConfigPortalBlocking(false);             // Non-Blocking portal for ESP32, ESP8266 had rendering issues during testing
    wifiManager.setBreakAfterConfig(true);                  // Saves settings, even if WiFi Fails
    wifiManager.setSaveConfigCallback(saveConfigCallback);  // Set config save callback
    wifiManager.setSaveParamsCallback(saveConfigCallback);  // Set param save callback
    wifiManager.setAPClientCheck(true);                     // avoid timeout if client connected to softap

#ifndef ARDUINO_WT32_ETH01
    wifiManager.setConfigPortalTimeout(120);  // Timeout before launching the config portal (WiFi Only)
    if (!wifiManager.autoConnect("Ecodan Bridge AP")) {
      DEBUG_PRINTLN("Failed to connect and hit timeout");
    } else {
      DEBUG_PRINTLN("WiFi Connected!");
    }
#endif
  }



  void PublishDiscoveryTopics(uint8_t MQTTStream, String BASETOPIC) {

    // Compile Topics
    String MQTT_DISCOVERY_TOPIC, Buffer_Topic;

// -- Entities Configuration JSON -- //
#ifdef ESP8266
    String ChipModel = "ESP8266";
#endif
#ifdef ESP32
    String ChipModel = ESP.getChipModel();
#endif

    String ChipID = mqttSettings.deviceId;

    // JSON Formation
    StaticJsonDocument<2048> Config;
    char Buffer_Payload[2048];

    // Publish all the discovery topics
    for (int i = 0; i < discovery_topics; i++) {

      if (i == 0) {  // If the first topic
        Config["device"]["identifiers"] = WiFiHostname;
        Config["device"]["manufacturer"] = "F1p";
        Config["device"]["model"] = ChipModel;
        Config["device"]["serial_number"] = ChipID;
        Config["device"]["name"] = "Ecodan CNRF";
        Config["device"]["configuration_url"] = "http://" + WiFi.localIP().toString() + ":80";
        Config["device"]["sw_version"] = FirmwareVersion;
      } else {  // Otherwise post just identifier
        Config["device"]["identifiers"] = WiFiHostname;
      }

      // Every one has a unique_id and name
      Config["unique_id"] = String(MQTT_SENSOR_UNIQUE_ID[i]) + ChipID;
      Config["name"] = String(MQTT_SENSOR_NAME[i]);

      // Sensors
      if (i >= 0 && i < 3) {
        Config["state_topic"] = BASETOPIC + String(MQTT_TOPIC[MQTT_TOPIC_POS[i]]);                                    // Needs a positioner
        if (MQTT_UNITS_POS[i] > 0) { Config["unit_of_measurement"] = String(MQTT_SENSOR_UNITS[MQTT_UNITS_POS[i]]); }  // Don't send nothing
        Config["value_template"] = String(MQTT_SENSOR_VALUE_TEMPLATE[i]);
        Config["icon"] = String(MQTT_MDI_ICONS[i]);

        MQTT_DISCOVERY_TOPIC = "homeassistant/sensor/";
      }

      // Number Input
      if (i >= 3 && i < 5) {
        Config["min"] = 0;
        Config["max"] = 50;
        Config["mode"] = "box";
        Config["platform"] = "number";
        Config["step"] = 0.01;
        Config["unit_of_measurement"] = "°C";
        Config["command_topic"] = BASETOPIC + String(MQTT_TOPIC[i]);
        Config["state_topic"] = BASETOPIC + String(MQTT_TOPIC[2]);
        Config["value_template"] = String(MQTT_SENSOR_VALUE_TEMPLATE[i]);
        Config["icon"] = String(MQTT_MDI_ICONS[i]);

        // Add Availability Topics
        Config["availability"]["topic"] = BASETOPIC + String(MQTT_TOPIC[0]);

        MQTT_DISCOVERY_TOPIC = "homeassistant/number/";
      }
      
      size_t buf_size = serializeJson(Config, Buffer_Payload);
      Buffer_Topic = MQTT_DISCOVERY_TOPIC + ChipID + String(MQTT_DISCOVERY_OBJ_ID[i]) + "/config";

      if (MQTTStream == 1) {
        MQTTClient1.publish(Buffer_Topic.c_str(), (uint8_t*)&Buffer_Payload, buf_size, true);
      } else if (MQTTStream == 2) {
        MQTTClient2.publish(Buffer_Topic.c_str(), (uint8_t*)&Buffer_Payload, buf_size, true);
      }


      MQTT_DISCOVERY_TOPIC = "";  // Clear everything ready for next loop to save RAM
      Buffer_Topic = "";
      Config.clear();
    }

    // Generate Publish Message
    DEBUG_PRINTLN("Published Discovery Topics!");
  }

  void initializeMQTTClient1() {
    DEBUG_PRINT("Attempting MQTT connection to: ");
    DEBUG_PRINT(mqttSettings.hostname);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(mqttSettings.port);
    MQTTClient1.setServer(mqttSettings.hostname, atoi(mqttSettings.port));
  }

  void MQTTonConnect(void) {
    DEBUG_PRINTLN("MQTT ON CONNECT");
    MQTTClient1.publish(MQTT_LWT.c_str(), "online");
    delay(50);

    MQTTClient1.subscribe(MQTTCommandController1Current.c_str());
    MQTTClient1.subscribe(MQTTCommandController2Current.c_str());

    delay(10);
    PublishDiscoveryTopics(1, MQTT_BASETOPIC);
#ifdef ESP8266                       // Define the Witty ESP8266 Ports
    analogWrite(Green_RGB_LED, 30);  // Green LED on, 25% brightness
    digitalWrite(Red_RGB_LED, LOW);  // Turn the Red LED Off
#endif
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
    leds[0] = CRGB::Green;
    FastLED.setBrightness(100);  // LED on, reduced brightness
    FastLED.show();
#endif
  }


  uint8_t MQTTReconnect() {
    if (MQTTClient1.connected()) {
      return 1;
    } else if (strcmp(mqttSettings.hostname, "IPorDNS") != 0 && strcmp(mqttSettings.hostname, "") != 0) {
      DEBUG_PRINT("With Client ID: ");
      DEBUG_PRINT(mqttSettings.clientId);
      DEBUG_PRINT(", Username: ");
      DEBUG_PRINT(mqttSettings.user);
      DEBUG_PRINT(" and Password: ");
      DEBUG_PRINTLN(mqttSettings.password);

      if (MQTTClient1.connect(mqttSettings.clientId, mqttSettings.user, mqttSettings.password, MQTT_LWT.c_str(), 0, true, "offline")) {
        DEBUG_PRINTLN("MQTT Server Connected");
        MQTTonConnect();
#ifdef ESP8266                              // Define the Witty ESP8266 Ports
        digitalWrite(Red_RGB_LED, LOW);     // Turn off the Red LED
        digitalWrite(Green_RGB_LED, HIGH);  // Flash the Green LED
        delay(10);
        digitalWrite(Green_RGB_LED, LOW);
#endif
        return 1;
      } else {
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
        //FastLED.setBrightness(255);  // LED on, reduced brightness
        leds[0] = CRGB::Orange;
        //
#endif
        switch (MQTTClient1.state()) {
          case -4:
            DEBUG_PRINTLN("MQTT_CONNECTION_TIMEOUT");
            break;
          case -3:
            DEBUG_PRINTLN("MQTT_CONNECTION_LOST");
            break;
          case -2:
            DEBUG_PRINTLN("MQTT_CONNECT_FAILED");
            break;
          case -1:
            DEBUG_PRINTLN("MQTT_DISCONNECTED");
            break;
          case 0:
            DEBUG_PRINTLN("MQTT_CONNECTED");
            break;
          case 1:
            DEBUG_PRINTLN("MQTT_CONNECT_BAD_PROTOCOL");
            break;
          case 2:
            DEBUG_PRINTLN("MQTT_CONNECT_BAD_CLIENT_ID");
            break;
          case 3:
            DEBUG_PRINTLN("MQTT_CONNECT_UNAVAILABLE");
            break;
          case 4:
            DEBUG_PRINTLN("MQTT_CONNECT_BAD_CREDENTIALS");
            break;
          case 5:
            DEBUG_PRINTLN("MQTT_CONNECT_UNAUTHORIZED");
            break;
        }
        return 0;
      }
      return 0;
    } else {
      DEBUG_PRINTLN("Primary MQTT Not Set");
      return 0;
    }
  }


  void handleMQTTState() {
    if (!MQTTClient1.connected()) {
#ifdef ARDUINO_M5STACK_ATOMS3  // Define the M5Stack LED
      leds[0] = CRGB::Orange;  // Turn the LED Orange
      FastLED.show();
#endif
#ifdef ESP8266                          // Define the Witty ESP8266 Ports
      analogWrite(Green_RGB_LED, 30);   // Green LED on, 25% brightness
      digitalWrite(Red_RGB_LED, HIGH);  // Add the Red LED to the Green LED = Orange
#endif
      MQTTReconnect();
    }
  }


  void RecalculateMQTT2Topics() {
    // The base topic may change via WiFi Manager
    MQTT_2_LWT = MQTT_2_BASETOPIC + "/LWT";
    MQTT_2_STATUS = MQTT_2_BASETOPIC + "/Status";
    MQTT_2_COMMAND = MQTT_2_BASETOPIC + "/Command";

    MQTT_2_STATUS_TEMP = MQTT_2_STATUS + "/Temperature";
    MQTT_2_STATUS_WIFISTATUS = MQTT_2_STATUS + "/WiFiStatus";

    MQTT_2_COMMAND_C1 = MQTT_2_COMMAND + "/Controller1";
    MQTT_2_COMMAND_C2 = MQTT_2_COMMAND + "/Controller2";

    MQTTCommand2Controller1Current = MQTT_2_COMMAND_C1;
    MQTTCommand2Controller2Current = MQTT_2_COMMAND_C2;
  }





  void initializeMQTT2Client() {
    DEBUG_PRINT("Attempting MQTT connection to: ");
    DEBUG_PRINT(mqttSettings.hostname2);
    DEBUG_PRINT(":");
    DEBUG_PRINTLN(mqttSettings.port2);
    MQTTClient2.setServer(mqttSettings.hostname2, atoi(mqttSettings.port2));
  }



  void MQTT2onConnect(void) {
    DEBUG_PRINTLN("MQTT 2 ON CONNECT");
    MQTTClient2.publish(MQTT_2_LWT.c_str(), "online");
    delay(50);

    DEBUG_PRINTLN(MQTTCommand2Controller1Current.c_str());
    MQTTClient2.subscribe(MQTTCommand2Controller1Current.c_str());
    MQTTClient2.subscribe(MQTTCommand2Controller2Current.c_str());

    delay(10);
    PublishDiscoveryTopics(2, MQTT_2_BASETOPIC);
  }


  uint8_t MQTT2Reconnect() {
    if (MQTTClient2.connected()) {
      return 1;
    } else if (strcmp(mqttSettings.hostname2, "IPorDNS") != 0 && strcmp(mqttSettings.hostname2, "") != 0) {
      DEBUG_PRINT("With Client ID: ");
      DEBUG_PRINT(mqttSettings.clientId2);
      DEBUG_PRINT(", Username: ");
      DEBUG_PRINT(mqttSettings.user2);
      DEBUG_PRINT(" and Password: ");
      DEBUG_PRINTLN(mqttSettings.password2);

      if (MQTTClient2.connect(mqttSettings.deviceId, MQTT_2_LWT.c_str(), 0, true, "offline")) {
        DEBUG_PRINTLN("MQTT Server 2 Connected");
        MQTT2onConnect();
        return 1;
      } else {
        switch (MQTTClient2.state()) {
          case -4:
            DEBUG_PRINTLN("MQTT_2_CONNECTION_TIMEOUT");
            break;
          case -3:
            DEBUG_PRINTLN("MQTT_2_CONNECTION_LOST");
            break;
          case -2:
            DEBUG_PRINTLN("MQTT_2_CONNECT_FAILED");
            break;
          case -1:
            DEBUG_PRINTLN("MQTT_2_DISCONNECTED");
            break;
          case 0:
            DEBUG_PRINTLN("MQTT_2_CONNECTED");
            break;
          case 1:
            DEBUG_PRINTLN("MQTT_2_CONNECT_BAD_PROTOCOL");
            break;
          case 2:
            DEBUG_PRINTLN("MQTT_2_CONNECT_BAD_CLIENT_ID");
            break;
          case 3:
            DEBUG_PRINTLN("MQTT_2_CONNECT_UNAVAILABLE");
            break;
          case 4:
            DEBUG_PRINTLN("MQTT_2_CONNECT_BAD_CREDENTIALS");
            break;
          case 5:
            DEBUG_PRINTLN("MQTT_2_CONNECT_UNAUTHORIZED");
            break;
        }
        return 0;
      }
      return 0;
    } else {
      DEBUG_PRINTLN("Secondary MQTT Not Set");
      return 0;
    }
  }


  void handleMQTT2State() {
    if (!MQTTClient2.connected()) {
      MQTT2Reconnect();
    }
    MQTTClient2.loop();
  }

#endif