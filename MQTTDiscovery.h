//-- MQTT Home Assistant Auto Discovery --//

const int discovery_topics PROGMEM = 5;

// Build the sensor JSON structure
const char MQTT_DISCOVERY_OBJ_ID[][3] PROGMEM = { "aa", "ab", "ac", "ad", "ae" };

const char MQTT_SENSOR_UNIQUE_ID[][32] PROGMEM = {
  "ashp_cnrf_bridge_lwt_",
  "ashp_cnrf_bridge_firmware_",
  "ashp_cnrf_bridge_rssi_",
  "ashp_cnrf_temp1_",
  "ashp_cnrf_temp2_"
};


const char MQTT_MDI_ICONS[][30] PROGMEM = {
  "mdi:cloud-check-variant",
  "mdi:alpha-v-box",
  "mdi:signal-variant",
  "mdi:thermometer",
  "mdi:thermometer"  //5
};


const char MQTT_SENSOR_NAME[][40] PROGMEM = {
  "CNRF Bridge Status",
  "CNRF Bridge Firmware Version",
  "CNRF Bridge WiFi Signal",
  "Controller 1 Temperature",
  "Controller 2 Temperature",
};

const char MQTT_TOPIC[][34] PROGMEM = {
  "/LWT",                               //0
  "/Status/WiFiStatus",                 //1
  "/Status/Temperature",                //2
  "/Command/Controller1",                //3
  "/Command/Controller2",                //4
};


int MQTT_TOPIC_POS[] PROGMEM = {
  0,
  1,
  1,
  2,
  2  //5
};

int MQTT_UNITS_POS[] PROGMEM = {
  0,
  0,
  1,
  2,
  2
};


const char MQTT_SENSOR_UNITS[][6] PROGMEM = {
  "",
  "dBm",
  "°C"
};

const char MQTT_SENSOR_VALUE_TEMPLATE[][50] PROGMEM = {
  "{{ value if value is defined else 'Unknown' }}",
  "{{ value_json.Firmware }}",
  "{{ value_json.RSSI }}",
  "{{ value_json.Input1 }}",
  "{{ value_json.Input2 }}"  //5  
};
