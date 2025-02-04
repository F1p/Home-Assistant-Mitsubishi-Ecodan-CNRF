//-- MQTT Home Assistant Auto Discovery --//

const int discovery_topics PROGMEM = 26;

// Build the sensor JSON structure
const char MQTT_DISCOVERY_OBJ_ID[][3] PROGMEM = { "aa", "ab", "ac", "am", "an", "ao", "ap", "aq", "ar", "as", "at", "au", "av", "aw", "ax", "ay", "az", "ba", "bb", "ad", "ae", "af", "ag", "ah", "ai", "aj", "ak", "al" };

const char MQTT_SENSOR_UNIQUE_ID[][32] PROGMEM = {
  "ashp_cnrf_bridge_lwt_",
  "ashp_cnrf_bridge_firmware_",
  "ashp_cnrf_bridge_rssi_",
  "ashp_cnrf_z1_input_",
  "ashp_cnrf_z2_input_",
  "ashp_cnrf_power_",
  "ashp_cnrf_opmode_",
  "ashp_cnrf_z1_mode_",
  "ashp_cnrf_z2_mode_",
  "ashp_cnrf_z1_sp_",
  "ashp_cnrf_z2_sp_",
  "ashp_cnrf_tmr_mode_",
  "ashp_cnrf_ec_",
  "ashp_cnrf_dhw_",
  "ashp_cnrf_hol_",
  "ashp_cnrf_holhrs_",
  "ashp_cnrf_climate_z1_",
  "ashp_cnrf_climate_z2_",
  "ashp_cnrf_qty_",
  "ashp_cnrf_rc1_",
  "ashp_cnrf_rc2_",
  "ashp_cnrf_rc3_",
  "ashp_cnrf_rc4_",
  "ashp_cnrf_rc5_",
  "ashp_cnrf_rc6_",
  "ashp_cnrf_rc7_",
  "ashp_cnrf_rc8_"
};

const char MQTT_MDI_ICONS[][30] PROGMEM = {
  "mdi:cloud-check-variant",
  "mdi:alpha-v-box",
  "mdi:signal-variant",
  "mdi:counter",
  "mdi:counter",
  "mdi:power",
  "mdi:auto-mode",
  "mdi:format-list-bulleted",
  "mdi:format-list-bulleted",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:format-list-bulleted",
  "mdi:home-alert",
  "mdi:thermometer-plus",
  "mdi:bag-suitcase",
  "mdi:clock-digital",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:counter",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer",
  "mdi:thermometer"
};


const char MQTT_SENSOR_NAME[][40] PROGMEM = {
  "Bridge Status",
  "Bridge Firmware Version",
  "Bridge WiFi Signal",
  "Zone 1 Input Source",
  "Zone 2 Input Source",
  "Power",
  "System Operation Mode",
  "Heating/Cooling Operation Mode Zone 1",
  "Heating/Cooling Operation Mode Zone 2",
  "Zone 1 Setpoint",
  "Zone 2 Setpoint",
  "Timer Setting",
  "Error Code",
  "DHW Forced Boost",
  "Holiday Mode",
  "Holiday Mode Hours",
  "Zone 1 Thermostat",
  "Zone 2 Thermostat",
  "Sensor Quantity",
  "RC1 Input",
  "RC2 Input",
  "RC3 Input",
  "RC4 Input",
  "RC5 Input",
  "RC6 Input",
  "RC7 Input",
  "RC8 Input"
};

const char MQTT_TOPIC[][34] PROGMEM = {
  "/LWT",                 //0
  "/Status/WiFiStatus",   //1
  "/Status/Temperature",  //2
  "/Command/QTY",
  "/Command/RC"
};

int MQTT_TOPIC_POS[] PROGMEM = {
  0,
  1,
  1,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2
};
int MQTT_UNITS_POS[] PROGMEM = {
  0,
  0,
  1,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  2,
  2,
  2,
  2,
  2,
  2,
  2,
  2
};


const char MQTT_SENSOR_UNITS[][6] PROGMEM = {
  "",
  "dBm",
  "°C"
};

const char MQTT_CMD_TEMPLATE[][50] PROGMEM = {
  "{ 'type':'rc', 'action':",
  ", 'value': {{ value }} }"
};

const char MQTT_SENSOR_VALUE_TEMPLATE[][50] PROGMEM = {
  "{{ value if value is defined else 'Unknown' }}",
  "{{ value_json.Firmware }}",
  "{{ value_json.RSSI }}",
  "{{ value_json.Z1ActiveInput }}",
  "{{ value_json.Z2ActiveInput }}",
  "{{ value_json.Power }}",
  "{{ value_json.SystemOperationMode }}",
  "{{ value_json.Zone1OperationMode }}",
  "{{ value_json.Zone2OperationMode }}",
  "{{ value_json.Z1Setpoint }}",
  "{{ value_json.Z2Setpoint }}",
  "{{ value_json.TimerProhibit }}",
  "{{ value_json.ErrorCode }}",
  "{{ value_json.DHWForce }}",
  "{{ value_json.Holiday }}",
  "{{ value_json.HolidayHrs }}",
  "{{ value_json.Z1Setpoint }}",
  "{{ value_json.Z2Setpoint }}",
  "{{ value_json.QTY }}",
  "{{ value_json.RC1Input }}",
  "{{ value_json.RC2Input }}",
  "{{ value_json.RC3Input }}",
  "{{ value_json.RC4Input }}",
  "{{ value_json.RC5Input }}",
  "{{ value_json.RC6Input }}",
  "{{ value_json.RC7Input }}",
  "{{ value_json.RC8Input }}"
};
