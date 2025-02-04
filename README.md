# Mitsubishi Ecodan Air-to-Water Bridge for CNRF to MQTT

This software is to communicate with the Mitsubishi Ecodan FTC controller over the CNRF Port,
The CNRF Port is typically used to communicate with the Mitsibushi Wireless Thermostats, therefore this software can replicates the communications used and allow you to use generic temperature sensors as inputs, via Home Assistant to input current room temperature into the Ecodan for Auto-Adapt & Thermostat control.

This project uses the same hardware as for the CN105, the software is also completely interchangable via the "Update" page.


# Hardware

To learn about the Hardware and Device Recovery, please read [Hardware and Recovery.md](https://github.com/F1p/Mitsubishi-CN105-Protocol-Decode/blob/master/documentation/Hardware%20and%20Recovery.md)

Pre-Compiled software is for offical hardware:

 - Generation 1 ESP8266 ebay sold hardware: https://www.ebay.co.uk/itm/325967595655
 - Generation 2 ESP32 ebay sold hardware: https://www.ebay.co.uk/itm/326347231581
 - ESP32 Ethernet ebay sold hardware: https://www.ebay.co.uk/itm/326278295259

You can bring your own, compile in Arduino or flash the hardware above with pre-compiled.


# Mitsibushi CN105 Protocol

To learn all about the Mitsibushi CNRF Protocol, please read [CNRF Protocol.md](https://github.com/F1p/Home-Assistant-Mitsubishi-Ecodan-CNRF/blob/main/CNRF%20Protocol.md)