<<<<<<< HEAD
# CoogleIOT wrapper library to manage sensors and actuators

The library **CoogleIOT** (https://github.com/ThisSmartHouse/CoogleIOT) simplifies the 
development of IOT projects based on Espressif ESP8266 boards.

This wrapper library extends the CoogleIOT API adding some functions to make even easier
the task of generating MQTT messages and control the sensor through mqtt messages.

 * John Coggeshall's CoogleIOT Framework for IOT systems using WiFi
    - http://www.thissmarthouse.net/
    - https://www.arduinolibraries.info/libraries/coogle-iot
    - https://github.com/ThisSmartHouse/CoogleIOT

 * Nick O'Leary's PubSubClient MQTT Client (used by CoogleIOT)
    - https://github.com/knolleary
    - https://github.com/knolleary/pubsubclient
    - https://pubsubclient.knolleary.net/api.html    


 ## Tested with:
 
 + NodeMCU 1.0 ESP8266 (ESP-12E) Module
 + ESP-01 Module (Generic ESP8266 module)

_ESP-12E:_

```
    Using board 'nodemcuv2' from platform in folder:
        C:\Users\<user>\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.7.1
```
_ESP-01:_
   
```
    Using core 'esp8266' from platform in folder: 
        C:\Users\<user>\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\2.7.1
```

## Compatibility Issues 

 Some configuration has to be made on the __PubSubClient__ library which cannot be  made in the sketch , since `#define` macros in the main `.ino` files are not  visible to  the compiler while compiling the libraries' `.cpp` files. 

### PubSubClient default packet size

 The library's default packet size of 128 won't work with CoogleIOT as explained in CoogleIOT's **README.md** doc (MQTT Client Notes, https://github.com/ThisSmartHouse/CoogleIOT#mqtt-client-notes).

__In `PubSubClient.h`__

```C++
 // MQTT_MAX_PACKET_SIZE : Maximum packet size  
 // [NOTE] CHANGED FROM 128 TO 1024 FOR COMPATIBILITY WITH COOGLEIOT
 #ifndef MQTT_MAX_PACKET_SIZE
 #define MQTT_MAX_PACKET_SIZE 1024  // WAS 128
 #endif
 ```

### Old MQTT brokers

Very old versiosn id the Mosquitto MQTT broker rejected version 3_1_1 messages which is the default in the library. In this case, change to 3_1:

__In `PubSubClient.h`__

```C++
 // MQTT_VERSION : Pick the version 
 // [NOTE] Use 3_1 for compatibility with very old mosquitto brokers
 #define MQTT_VERSION MQTT_VERSION_3_1  // remove to use 3_1_!
 #ifndef MQTT_VERSION
 #define MQTT_VERSION MQTT_VERSION_3_1_1
 #endif
```

 
 # CoogleSensors
=======
# CoogleSensors
A CoogleIOT wrapper class to simplify integrating sensors with IoT frameworks
>>>>>>> 7bfe20da0caed73fca2cb846558c29d30de4dc7c
