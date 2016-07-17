Arduino ESP8266 Si7021 ThingSpeak Logger
========================================

A temperature and humidity logger using a ESP8266 in conjunction with a Si7021 i2c temperature and humidity sensor.  The ESP8266 will also act as a HTTP server and will return a response in json containing the current temperature and humidity read from the Si7021.

This example app relies on the Si7021 library from [LowPowerLab]("https://github.com/LowPowerLab") which can be found at: [https://github.com/LowPowerLab/SI7021]("https://github.com/LowPowerLab/SI7021")

Please change the following definitions to suit your purposes:

    #define AP_SSID   "xxxxx" // Access Point SSID
    #define AP_PSK    "xxxxx" // Access Point PSK
    #define TS_APIKEY "xxxxx" // ThingSpeak API Key

The frequency of the update sent to ThingSpeak can be changed by altering the following line:

    #define FIVEMIN (1000UL * 60 * 5) // Update Frequency
