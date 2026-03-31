String MQTT_BASETOPIC = "Ecodan/CNRF";

String MQTT_LWT = MQTT_BASETOPIC + "/LWT";
String MQTT_STATUS = MQTT_BASETOPIC + "/Status";
String MQTT_COMMAND = MQTT_BASETOPIC + "/Command";

String MQTT_STATUS_TEMP = MQTT_STATUS + "/Temperature";
String MQTT_STATUS_WIFISTATUS = MQTT_STATUS + "/WiFiStatus";

String MQTT_COMMAND_QTY = MQTT_COMMAND + "/QTY";
String MQTT_COMMAND_RC = MQTT_COMMAND + "/RC";

String MQTTCommandControllerQTY = MQTT_COMMAND_QTY;
String MQTTCommandControllerRC = MQTT_COMMAND_RC;

const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
char snprintbuffer[41] = "";
char DeviceID[15] = "";
const char ClientPrefix[15] = "CNRFBridge-";
char WiFiHostname[40] = "";



// Programs

#if defined(ESP8266) || defined(ESP32)  // ESP32 or ESP8266 Compatiability
void readSettingsFromConfig() {
  // Clean LittleFS for testing
  //LittleFS.format();

  // Read configuration from LittleFS JSON
  DEBUG_PRINTLN(F("Mounting File System..."));
#ifdef ESP8266
  if (LittleFS.begin()) {
#endif
#ifdef ESP32
    if (LittleFS.begin("/storage")) {
#endif
      DEBUG_PRINTLN(F("Mounted File System"));
      if (LittleFS.exists("/config.json")) {
        //file exists, reading and loading
        DEBUG_PRINTLN(F("Reading config file"));
        File configFile = LittleFS.open("/config.json", "r");
        if (configFile) {
          DEBUG_PRINTLN(F("Opened config file"));
          JsonDocument doc;
          DeserializationError error = deserializeJson(doc, configFile);
          if (error) {
            DEBUG_PRINT(F("Failed to read file: "));
            DEBUG_PRINTLN(error.c_str());
          } else {
            DEBUG_PRINTLN(F("Parsed JSON: "));
            serializeJson(doc, DEBUGPORT);
            DEBUG_PRINTLN();

            // Build in safety check, otherwise ESP will crash out and you can't get back in

            // Heatmiser Connections
            if (doc.containsKey(mqttSettings.wm_hm_hostname_identifier)) {
              if ((strlen(doc[mqttSettings.wm_hm_hostname_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_hm_hostname_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.heatmiser, doc[mqttSettings.wm_hm_hostname_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("Hostname Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_hm_z1_identifier)) {
              if ((strlen(doc[mqttSettings.wm_hm_z1_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_hm_z1_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.z1_name, doc[mqttSettings.wm_hm_z1_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("Z1 Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_hm_z2_identifier)) {
              if ((strlen(doc[mqttSettings.wm_hm_z2_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_hm_z2_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.z2_name, doc[mqttSettings.wm_hm_z2_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("Z2 Parse Failed"));
            }
#ifdef ESP8266
            snprintf(snprintbuffer, deviceId_max_length, (String(ESP.getChipId(), HEX)).c_str());
#endif
#ifdef ESP32
            snprintf(snprintbuffer, deviceId_max_length, (String(ESP.getEfuseMac(), HEX)).c_str());
#endif
            strcpy(mqttSettings.deviceId, snprintbuffer);

            if (doc.containsKey(mqttSettings.wm_mqtt_hostname_identifier)) {
              if ((strlen(doc[mqttSettings.wm_mqtt_hostname_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_mqtt_hostname_identifier]) + 1) <= hostname_max_length)) {
                strcpy(mqttSettings.hostname, doc[mqttSettings.wm_mqtt_hostname_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("MQTT Hostname Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_port_identifier)) {
              if ((strlen(doc[mqttSettings.wm_mqtt_port_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_mqtt_port_identifier]) + 1) <= port_max_length)) {
                strcpy(mqttSettings.port, doc[mqttSettings.wm_mqtt_port_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("MQTT Port Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_user_identifier)) {
              if ((strlen(doc[mqttSettings.wm_mqtt_user_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_mqtt_user_identifier]) + 1) <= user_max_length)) {
                strcpy(mqttSettings.user, doc[mqttSettings.wm_mqtt_user_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("MQTT Username Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_password_identifier)) {
              if ((strlen(doc[mqttSettings.wm_mqtt_password_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_mqtt_password_identifier]) + 1) <= password_max_length)) {
                strcpy(mqttSettings.password, doc[mqttSettings.wm_mqtt_password_identifier]);
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("MQTT Password Parse Failed"));
            }
            if (doc.containsKey(mqttSettings.wm_mqtt_basetopic_identifier)) {
              if ((strlen(doc[mqttSettings.wm_mqtt_basetopic_identifier]) > 0) && ((strlen(doc[mqttSettings.wm_mqtt_basetopic_identifier]) + 1) <= basetopic_max_length)) {
                strcpy(mqttSettings.baseTopic, doc[mqttSettings.wm_mqtt_basetopic_identifier]);
                MQTT_BASETOPIC = mqttSettings.baseTopic;
              }
            } else {
              shouldSaveConfig = true;
              DEBUG_PRINTLN(F("MQTT Basetopic Parse Failed"));
            }
          }
        }
        configFile.close();
      } else {
        DEBUG_PRINTLN(F("No config file exists, using placeholder values"));
        // Populate the Dynamic Variables (Device ID)
#ifdef ESP8266
        snprintf(DeviceID, deviceId_max_length, (String(ESP.getChipId(), HEX)).c_str());
#endif
#ifdef ESP32
        snprintf(DeviceID, deviceId_max_length, (String(ESP.getEfuseMac(), HEX)).c_str());
#endif
        strcpy(mqttSettings.deviceId, DeviceID);
      }
    } else {
      DEBUG_PRINTLN(F("Failed to mount File System"));
    }
  }



  void RecalculateMQTTTopics() {
    // The base topic may change via WiFi Manager
    MQTT_LWT = MQTT_BASETOPIC + "/LWT";
    MQTT_STATUS = MQTT_BASETOPIC + "/Status";
    MQTT_COMMAND = MQTT_BASETOPIC + "/Command";

    MQTT_STATUS_TEMP = MQTT_STATUS + "/Temperature";
    MQTT_STATUS_WIFISTATUS = MQTT_STATUS + "/WiFiStatus";

    MQTT_COMMAND_QTY = MQTT_COMMAND + "/QTY";
    MQTT_COMMAND_RC = MQTT_COMMAND + "/RC";

    MQTTCommandControllerQTY = MQTT_COMMAND_QTY;
    MQTTCommandControllerRC = MQTT_COMMAND_RC;
  }


  void saveConfig() {
    // Read MQTT Portal Values for save to file system
    DEBUG_PRINTLN(F("Copying Portal Values..."));
    strcpy(mqttSettings.deviceId, custom_device_id.getValue());
    strcpy(mqttSettings.heatmiser, custom_hm_server.getValue());
    strcpy(mqttSettings.z1_name, custom_hm_z1.getValue());
    strcpy(mqttSettings.z2_name, custom_hm_z2.getValue());
    strcpy(mqttSettings.hostname, custom_mqtt_server.getValue());
    strcpy(mqttSettings.port, custom_mqtt_port.getValue());
    strcpy(mqttSettings.user, custom_mqtt_user.getValue());
    strcpy(mqttSettings.password, custom_mqtt_pass.getValue());
    strcpy(mqttSettings.baseTopic, custom_mqtt_basetopic.getValue());

    DEBUG_PRINT(F("Saving config... "));
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      DEBUG_PRINTLN(F("[FAILED] Unable to open config file for writing"));
      return;
    } else {
      JsonDocument doc;
      doc[mqttSettings.wm_device_id_identifier] = mqttSettings.deviceId;
      doc[mqttSettings.wm_hm_hostname_identifier] = mqttSettings.heatmiser;
      doc[mqttSettings.wm_hm_z1_identifier] = mqttSettings.z1_name;
      doc[mqttSettings.wm_hm_z2_identifier] = mqttSettings.z2_name;
      doc[mqttSettings.wm_mqtt_hostname_identifier] = mqttSettings.hostname;
      doc[mqttSettings.wm_mqtt_port_identifier] = mqttSettings.port;
      doc[mqttSettings.wm_mqtt_user_identifier] = mqttSettings.user;
      doc[mqttSettings.wm_mqtt_password_identifier] = mqttSettings.password;
      doc[mqttSettings.wm_mqtt_basetopic_identifier] = mqttSettings.baseTopic;

      if (serializeJson(doc, configFile) == 0) {
        DEBUG_PRINTLN("[FAILED]");
      } else {
        DEBUG_PRINTLN("[DONE]");
        serializeJson(doc, DEBUGPORT);
        DEBUG_PRINTLN();
#ifndef ARDUINO_WT32_ETH01
        if (WiFi.status() == WL_CONNECTED) {
          DEBUG_PRINTLN(F("Restarting Web Server..."));  // Restart the web server now it's on WiFi
          wifiManager.stopWebPortal();
          wifiManager.startWebPortal();
          MDNS.end();
          MDNS.begin("heatpump-cnrf");
          MDNS.addService("http", "tcp", 80);
        }
#endif
      }
    }
    configFile.close();
    shouldSaveConfig = false;
  }

  // Callback notifying us of the need to save config
  void saveConfigCallback() {
    saveConfig();
  }



  // Handle Webhook Callbacks
  void handleRoute() {
    if (wifiManager.server->hasArg("rc1")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc1");
      RCInput[0] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc2")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc2");
      RCInput[1] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc3")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc3");
      RCInput[2] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc4")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc4");
      RCInput[3] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc5")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc5");
      RCInput[4] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc6")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc6");
      RCInput[5] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc7")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc7");
      RCInput[6] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    }
    if (wifiManager.server->hasArg("rc8")) {  // Pull Argument room1
      String input = wifiManager.server->arg("rc8");
      RCInput[7] = input.toFloat();
      wifiManager.server->send(200, "text/plain", "success");
    } else {
      wifiManager.server->send(400, "text/plain", "failed");
    }
  }


  // Callback for webhooks
  void bindServerCallback() {
    wifiManager.server->on("/webhook", handleRoute);
  }

  void initializeWifiManager() {
    DEBUG_PRINTLN("Starting WiFi Manager");
    // Reset Wifi settings for testing
    wifiManager.setTitle("Ecodan Bridge CNRF");

    // Add the custom MQTT parameters here
    wifiManager.addParameter(&custom_hm_server);
    wifiManager.addParameter(&custom_hm_z1);
    wifiManager.addParameter(&custom_hm_z2);
    wifiManager.addParameter(&custom_mqtt_server);
    wifiManager.addParameter(&custom_mqtt_user);
    wifiManager.addParameter(&custom_mqtt_pass);
    wifiManager.addParameter(&custom_mqtt_port);
    wifiManager.addParameter(&custom_mqtt_basetopic);
    wifiManager.addParameter(&custom_device_id);

    // Set or Update the values
    custom_hm_server.setValue(mqttSettings.heatmiser, hostname_max_length);
    custom_hm_z1.setValue(mqttSettings.z1_name, hostname_max_length);
    custom_hm_z2.setValue(mqttSettings.z2_name, hostname_max_length);

    custom_device_id.setValue(mqttSettings.deviceId, deviceId_max_length);
    custom_mqtt_server.setValue(mqttSettings.hostname, hostname_max_length);
    custom_mqtt_port.setValue(mqttSettings.port, port_max_length);
    custom_mqtt_user.setValue(mqttSettings.user, user_max_length);
    custom_mqtt_pass.setValue(mqttSettings.password, password_max_length);
    custom_mqtt_basetopic.setValue(mqttSettings.baseTopic, basetopic_max_length);

    //set minimum quality of signal so it ignores AP's under that quality
    //defaults to 8%
    //wifiManager.setMinimumSignalQuality();

    snprintf(WiFiHostname, 40, "%s%s", ClientPrefix, mqttSettings.deviceId);
    WiFi.hostname(WiFiHostname);
#ifdef ESP8266                         // Define the Witty ESP8266 Ports
    digitalWrite(Blue_RGB_LED, HIGH);  // Turn the Blue LED On
#endif
    wifiManager.setConfigPortalBlocking(false);             // Non-Blocking portal
    wifiManager.setBreakAfterConfig(true);                  // Saves settings, even if WiFi Fails
    wifiManager.setSaveConfigCallback(saveConfigCallback);  // Set config save callback
    wifiManager.setAPClientCheck(true);                     // Avoid timeout if client connected to softap
    wifiManager.setWebServerCallback(bindServerCallback);   // Callback for the webhook route

#ifndef ARDUINO_WT32_ETH01
    wifiManager.setConfigPortalTimeout(600);  // Timeout before launching the config portal (WiFi Only)
    if (!wifiManager.autoConnect("Ecodan CNRF Bridge AP")) {
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
    JsonDocument Config;
    char Buffer_Payload[2048];

    // Publish all the discovery topics
    for (int i = 0; i <= discovery_topics; i++) {

      if (i == 0) {  // If the first topic
        Config["device"]["identifiers"] = WiFiHostname;
        Config["device"]["manufacturer"] = "F1p";
        Config["device"]["model"] = ChipModel;
        Config["device"]["serial_number"] = ChipID;
        Config["device"]["name"] = "Ecodan CNRF";
#ifdef ARDUINO_WT32_ETH01
        Config["device"]["cu"] = "http://" + ETH.localIP().toString() + ":80";
#else
      Config["device"]["cu"] = "http://" + WiFi.localIP().toString() + ":80";
#endif
        Config["device"]["sw_version"] = FirmwareVersion;
      } else {  // Otherwise post just identifier
        Config["device"]["identifiers"] = WiFiHostname;
      }

      // Every one has a unique_id and name
      Config["unique_id"] = String(MQTT_SENSOR_UNIQUE_ID[i]) + ChipID;
      Config["name"] = String(MQTT_SENSOR_NAME[i]);

      // Sensors
      if (i >= 0 && i < 16) {
        Config["state_topic"] = BASETOPIC + String(MQTT_TOPIC[MQTT_TOPIC_POS[i]]);                                    // Needs a positioner
        if (MQTT_UNITS_POS[i] > 0) { Config["unit_of_measurement"] = String(MQTT_SENSOR_UNITS[MQTT_UNITS_POS[i]]); }  // Don't send nothing
        Config["value_template"] = String(MQTT_SENSOR_VALUE_TEMPLATE[i]);
        Config["icon"] = String(MQTT_MDI_ICONS[i]);

        MQTT_DISCOVERY_TOPIC = "homeassistant/sensor/";
      }

      // Climate
      if (i >= 16 && i < 18) {
        MQTT_DISCOVERY_TOPIC = "homeassistant/climate/";
      }

      // QTY Select
      if (i >= 18 && i < 19) {
        Config["command_topic"] = BASETOPIC + String(MQTT_TOPIC[3]);
        Config["state_topic"] = BASETOPIC + String(MQTT_TOPIC[2]);
        Config["value_template"] = String(MQTT_SENSOR_VALUE_TEMPLATE[i]);
        Config["options"][0] = "1";
        Config["options"][1] = "2";
        Config["options"][2] = "3";
        Config["options"][3] = "4";
        Config["options"][4] = "5";
        Config["options"][5] = "6";
        Config["options"][6] = "7";
        Config["options"][7] = "8";
        Config["icon"] = String(MQTT_MDI_ICONS[i]);

        // Add Availability Topics
        Config["availability"]["topic"] = BASETOPIC + String(MQTT_TOPIC[0]);

        MQTT_DISCOVERY_TOPIC = "homeassistant/select/";
      }

      // Number
      if (i >= 19 && i < 27) {
        Config["min"] = 0;
        Config["max"] = 50;
        Config["mode"] = "box";
        Config["platform"] = "number";
        Config["step"] = 0.01;
        Config["unit_of_measurement"] = "°C";
        Config["command_topic"] = BASETOPIC + String(MQTT_TOPIC[4]);
        Config["command_template"] = String(MQTT_CMD_TEMPLATE[0]) + String(i - 18) + String(MQTT_CMD_TEMPLATE[1]);
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

    MQTTClient1.subscribe(MQTTCommandControllerQTY.c_str());
    delay(25);
    MQTTClient1.subscribe(MQTTCommandControllerRC.c_str());

    delay(10);
    PublishDiscoveryTopics(1, MQTT_BASETOPIC);
#ifdef ESP8266                       // Define the Witty ESP8266 Ports
    analogWrite(Green_RGB_LED, 30);  // Green LED on, 25% brightness
    digitalWrite(Red_RGB_LED, LOW);  // Turn the Red LED Off
#endif
#ifdef ARDUINO_M5STACK_ATOMS3       // Define the M5Stack LED
    myLED.setPixel(0, L_GREEN, 1);  // set the LED colour and show it
    myLED.brightness(LED_BRIGHT, 1);
#endif
  }




  uint8_t MQTTReconnect() {
    if (MQTTClient1.connected()) {
      return 1;
    }
#ifdef ARDUINO_WT32_ETH01
    else if (strcmp(mqttSettings.hostname, "IPorDNS") != 0 && strcmp(mqttSettings.hostname, "") != 0) {  // Do not block MQTT attempt on Ethernet
#else
  else if (strcmp(mqttSettings.hostname, "IPorDNS") != 0 && strcmp(mqttSettings.hostname, "") != 0 && WiFi.status() == WL_CONNECTED) {  // WiFi should be active to attempt connections (as MQTT connect is blocking)
#endif
      initializeMQTTClient1();
      DEBUG_PRINT("with Device ID: ");
      DEBUG_PRINT(mqttSettings.deviceId);
      DEBUG_PRINT(", Username: ");
      DEBUG_PRINT(mqttSettings.user);
      DEBUG_PRINT(" and Password: ");
      DEBUG_PRINTLN(mqttSettings.password);

      if (MQTTClient1.connect(mqttSettings.deviceId, mqttSettings.user, mqttSettings.password, MQTT_LWT.c_str(), 0, true, "offline")) {
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
        if (!wifiManager.getConfigPortalActive()) {  // Not got config portal open, change to orange:
          myLED.setPixel(0, L_ORANGE, 1);            // set the LED colour and show it
        }
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
      DEBUG_PRINTLN(F("Skipping MQTT Connection as Username/Password is empty or no WiFi connection is available"));
      return 0;
    }
  }


  void handleMQTTState() {
    if (!MQTTClient1.connected()) {
#ifdef ARDUINO_M5STACK_ATOMS3                      // Define the M5Stack LED
      if (!wifiManager.getConfigPortalActive()) {  // Not got config portal open, change to orange:
        myLED.setPixel(0, L_ORANGE, 1);            // set the LED colour and show it
      }
#endif
#ifdef ESP8266                          // Define the Witty ESP8266 Ports
      analogWrite(Green_RGB_LED, 30);   // Green LED on, 25% brightness
      digitalWrite(Red_RGB_LED, HIGH);  // Add the Red LED to the Green LED = Orange
#endif
      MQTTReconnect();
      delay(10);
    }
  }




#endif