#pragma once
#ifndef COOGS__H
#define COOGS__H

#include <time.h>
#include <CoogleIOT.h>

//
// Configuration
//

#define SERIAL_BAUD 115200
#define COOGS_STATUS_INTERVAL  60000  // 1m

#define COOGS_MQTT_KEEPALVE 20
#define COOGS_MQTT_SOCKET_TIMEOUT 10

const String COGS_MESSAGE_VERSION = "3";

const String CLIENT_PREFIX       =  "SENSED";

#define COOGS_TOPIC_ROOT  "sensed"

//
// Topics to publish to
//
const char COOGS_STATUS_TOPIC[]       = COOGS_TOPIC_ROOT "/status";
const char COOGS_STATS_TOPIC[]        = COOGS_TOPIC_ROOT "/statistics";
const char COOGS_ERROR_TOPIC[]        = COOGS_TOPIC_ROOT "/error";
const char COOGS_DATA_TOPIC_PREFIX[]  = COOGS_TOPIC_ROOT "/data/";   // data 
//
// Topics to suscribe to
//
const char COOGS_COMMAND_TOPIC[]      = COOGS_TOPIC_ROOT "/command";  // To receive commands



//
// Status srtings and error codes
//
const char COOGS_STATUS_READY[]      =  "READY";
const char COOGS_STATUS_SHUTDOWN[]   =  "SHUTDOWN";

const char COOGS_ERROR_BAD_COMMAND[] =  "BAD_COMMAND";
const char COOGS_ERROR_UNSUSCRIBED[] =  "UNSUSCRIBED_TOPIC"; // Received message on unsuscribed topic


//
// Available commands
//
const char COOGS_COMMAND_STATUS[]       =  "status";   
const char COOGS_COMMAND_STATISTICS[]   =  "stats";
const char COOGS_COMMAND_RESTART[]      =  "restart";

//
// Standard return codes
//
const int COOGS_RETCODE_OK              =     0;
const int COOGS_RETCODE_MQTT_NOT_READY  = -9999;
const int COOGS_RETCODE_JSON_OVERFLOW   = -9998;
const int COOGS_RETCODE_MSG_TOO_LONG    = -9997;
const int COOGS_RETCODE_SERIALIZE_ERR   = -9980;
const int COOGS_RETCODE_PUBLISH_ERROR   = -9900;



#include <ArduinoJson.h>
// TO DO: ArduinoJson.h is included in CoogleIOT directory, must decide whether to
// use this an move it to the libraries directory or delete it and install it anew 
// from the libraries manager and then check for compatibility issues. (try a diff?)
//
// Apart from the excellent book, much help was provided by the online assistant:
//
// https://arduinojson.org/v5/assistant/

//
// Message buffer for ArduinoJSON
//
//
// Sizes estimated with ample margin. See README_json.txt
//
#define COOGS_JSON_BUFFER_SIZE       1024   // Memory used by ArduinoJson library
#define COOGS_MAX_JSON_MESSAGE_LEN    840   // Ouput message max size

class CoogleSensors:public CoogleIOT
{
    public:
      CoogleSensors();
      ~CoogleSensors();
      bool begin();
      void loop();
      bool is_online();


      //
      // Publishing 
      //
      int publish_c_str(char* topic, char *message, bool retain = false);
      void JSON_header(JsonObject& obj);
      int publish_JSON(char* topic, JsonObject& jso, bool retain = false);
      bool publish_measurement(char* measurement_name, int num_data, char **tags, float *values);
      int publish_error(const char* error_code, const char *from, const char *desc=NULL);
      int publish_status(const char* status_str);
      int publish_heartbeat();
      int publish_shutdown_ack();
      int publish_stats();

    private:
      PubSubClient  *mqtt = NULL;

      String CLIENT_ID;
      String CLIENT_ADDRESS = "";

      bool is_ONLINE = false;
      
      //
      // Stats
      //
      unsigned long last_status_millis = 0;
      unsigned long sensed_sent = 0;      // sensed packets handled to mqtt
      unsigned long json_buf_ovf = 0;     // Number of json buffer overflows (should be 0)
      unsigned long json_msg_ovf = 0;     // Number of json message overflows (should be 0)
      unsigned long serialize_errors = 0; // Errors serializing to buffer (shold never happen)
      unsigned long publish_errors = 0;   // Unpublished messages due to errore
      unsigned long invalid_messages = 0; // Number of invalid SENSED messages
};


//
// Handle commands received from MQTT
//
void mqttCallbackHandler(char *topic, byte *payload, unsigned int length);

#endif
