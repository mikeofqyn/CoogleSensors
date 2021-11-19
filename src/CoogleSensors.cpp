#include "CoogleSensors.h"


//
// Used by mqttCallbackHandler to access the CoogleSensor object (yeah, dirty), set upon initialization
//
static CoogleSensors* CoogleThat = NULL;


CoogleSensors::CoogleSensors() : CoogleIOT(LED_BUILTIN)
{

}

CoogleSensors::CoogleSensors(Print& the_tty) : CoogleIOT(LED_BUILTIN, the_tty) 
{

}

CoogleSensors::~CoogleSensors()
{
    CoogleThat = NULL;  // Used by MQTT callback function
}

bool CoogleSensors::is_online() {
    return this->mqttActive();
}

bool CoogleSensors::begin()
{

    CoogleThat = this;  // Used by MQTT callback function
  
    is_ONLINE = false;
    
    //
    // Initialize CoogleIOT
    // 
    if (&Tty == &Serial) Serial.begin(SERIAL_BAUD);
    Tty.println("CoogleSensors starting");

    unsigned long rseed = micros();

    this->enableSerial(SERIAL_BAUD);

    Tty.println("Initializing CoogleIOT");

    this->initialize();

    rseed += micros();

    this->logPrintf(INFO, "CoogleIOT Initialized. Local AP %s @ %s", this->getAPName().c_str(), WiFi.softAPIP().toString().c_str());

    // 
    // WiFi OK?
    //
    if (WiFi.status() == WL_CONNECTED)     {
        CLIENT_ADDRESS = WiFi.localIP().toString();
        this->logPrintf(INFO, "Connected to remote AP %s @ %s", this->getRemoteAPName().c_str(), CLIENT_ADDRESS.c_str());
    }
    else {
        // Wait up to 10 min. for the user to reconfigure via the buil in portal
        this->logPrintf(ERROR, "Could not connect to SSID %s. Please reconfigure", this->getRemoteAPName().c_str());
        unsigned int m = millis();
        while ((millis() - m) < (10 * 60 * 1000)) {
            this->loopWebServer();
            yield();
        }
        ESP.restart();
    }
    // Here the WiFi might be connected or not 

    rseed += micros(); // add more randomness to de random seed

    //
    // Generate new client id if default is set (or "*" has been entered)
    //
    String cli_id = this->getMQTTClientId();
    if ((cli_id == COOGLEIOT_DEFAULT_MQTT_CLIENT_ID) || 
        (cli_id == "") || 
        (cli_id == "*"))   {
        randomSeed(rseed);
        long num_id = random(999999999);
        String CLIENT_ID = CLIENT_PREFIX + String(num_id);
        this->setMQTTClientId(CLIENT_ID);
        this->logPrintf(INFO, "Client ID set to %s", CLIENT_ID.c_str());
        this->setMQTTSpecific1Name("Location");
        this->setMQTTSpecific2Name("Room");
    } else  {
        CLIENT_ID = this->getMQTTClientId();
    }

    // 
    // MQTT Broker connected?
    //
    if (this->mqttActive())  {
        this->logPrintf(INFO, "MQTT Active with client ID: %s", CLIENT_ID.c_str());
        mqtt = this->getMQTTClient();
        if (!mqtt) {
            this->error("Could not get mqtt broker");
            is_ONLINE = false;
        }
        else {
            mqtt->setKeepAlive(COOGS_MQTT_KEEPALVE);
            mqtt->setSocketTimeout(COOGS_MQTT_SOCKET_TIMEOUT);
            mqtt->setCallback(mqttCallbackHandler);
            mqtt->subscribe(COOGS_COMMAND_TOPIC);
            this->logPrintf(INFO, "Subscribed to command topic: %s", COOGS_COMMAND_TOPIC);
            this->publish_heartbeat();
            this->info("MQTT initialized.");
            is_ONLINE = true;
        }
    }
    else
    {
        this->logPrintf(ERROR, "Could not connect to MQTT broker %s. Check connection data (address/port/user/password)", this->getMQTTHostname().c_str());
        is_ONLINE = false;
    }
    if (!is_ONLINE) {
        this->flashSOS();
        this->info("Waiting up to 10 minutes. Please reconfigure and reset");
        unsigned int n = millis();
        while ((millis() - n) < (10 * 60 * 1000)) {
            this->loopWebServer();
        }
    }
    return is_ONLINE;
}


///////////////////////////////////////////////////////////////////////////////////
// Houskeepeing -- IMPORTANT Call frequently on every main loop() or while idle
//
void  CoogleSensors::loop() {
  //
  // Important, do CoogleIOT housekeeping stuff //
  //
  CoogleIOT::loop();   

  //
  // Status heartbeat
  //
  if ((millis()-last_status_millis) > COOGS_STATUS_INTERVAL) {
    if (is_ONLINE) {
      this->publish_heartbeat();
    } else {
      /*DBG*/ // iot->info("Offline. Not Publishing");
    }
    last_status_millis = millis();
  }    
}


///////////////////////////////////////////////////////////////////////////////////
// Publish C string
//
int CoogleSensors::publish_c_str(char* topic, char* message, bool retain) {
    // MQTT Available
    if (!this->mqttActive()) {
        publish_errors++;
        /*DBG*/ this->info("Can't publish C string, MQTT not active");
        return COOGS_RETCODE_MQTT_NOT_READY;
    }

    // Tty.print(">");Tty.println(message);

    // Publish
    if (!mqtt->publish(topic, message, retain)) {
        /*DBG*/ this->info("Can't publish C string, publish error");
        publish_errors++;
        return COOGS_RETCODE_PUBLISH_ERROR;
    }
    return COOGS_RETCODE_OK;
}

///////////////////////////////////////////////////////////////////////////////////
// Publish JSON Message
//
int CoogleSensors::publish_JSON(char* topic, JsonObject& obj, bool retain) {
    //
    // Add Timestamps
    //
    time_t tnow = time(NULL);
    if (!obj.set("timet", (unsigned long)tnow) || !obj.set("timestamp", ctime(&tnow))) {  // JSON object full
        json_buf_ovf++; // Number of json buffer overflows (should be 0)
        publish_errors++;
        return COOGS_RETCODE_JSON_OVERFLOW;
    }

    // Compute the length of the minified JSON document
    int lenmin = obj.measureLength();
    if (lenmin > COOGS_MAX_JSON_MESSAGE_LEN) {
        json_msg_ovf++;  // Number of json message overflows (should be 0)
        publish_errors++;
        return COOGS_RETCODE_MSG_TOO_LONG;
    }

    // Produce a minified JSON document
    char message[COOGS_MAX_JSON_MESSAGE_LEN];
    int lenreal = obj.printTo(message);
    if (lenreal != lenmin) {
        serialize_errors++; // Should never happen ;)
        publish_errors++;
        this->logPrintf(ERROR, "ArduinoJson printTo length mismatch: %d / %d", lenreal, lenmin);
        return COOGS_RETCODE_SERIALIZE_ERR;
    }

    // Publish
    return CoogleSensors::publish_c_str(topic, message, retain);

}

///////////////////////////////////////////////////////////////////////////////////
// Add data common to all messages to the json object
//
void CoogleSensors::JSON_header(JsonObject& obj) {

    //
    // Add sensor identification and location (app specific data 1 & 2)
    //
    obj["msg_version"] = COGS_MESSAGE_VERSION;
    String n1 = this->getMQTTSpecific1Name();
    String n2 = this->getMQTTSpecific2Name();
    obj["sensor_id"] = CLIENT_ID;
    obj["address"] = CLIENT_ADDRESS;
    if (n1 != "")
        obj[n1] = this->getMQTTAppSpecific1();
    if (n2 != "")
        obj[n2] = this->getMQTTAppSpecific2();

    //
    // Add Timestamps
    //
    time_t tnow = time(NULL);
    obj.set("timet", (unsigned long)tnow);
    obj.set("timestamp", ctime(&tnow));

    // 
    // Free heap (for debugging purposes)
    //
    obj["heap"] = ESP.getFreeHeap();

}


///////////////////////////////////////////////////////////////////////////////////
// Publish status 
//
int CoogleSensors::publish_status(const char* status_str) {
    //
    StaticJsonBuffer<COOGS_JSON_BUFFER_SIZE> jb;
    JsonObject& obj = jb.createObject();
    //
    // Add headers
    //
    JSON_header(obj);
    //
    obj["status"] = status_str;
    return this->publish_JSON((char*)COOGS_STATUS_TOPIC, obj, false);
}

///////////////////////////////////////////////////////////////////////////////////
// Publish heartbeat (status READY message)
//
int CoogleSensors::publish_heartbeat() {
    return this->publish_status(COOGS_STATUS_READY);
}


///////////////////////////////////////////////////////////////////////////////////
// Publish shutdown acknowledgement
//
int CoogleSensors::publish_shutdown_ack() {
    return this->publish_status(COOGS_STATUS_SHUTDOWN);
}

static const char *na(const char *x) {
  return x? x: "N/A"; 
}

///////////////////////////////////////////////////////////////////////////////////
// Publish error
//
int CoogleSensors::publish_error(const char* error_code, const char* from, const char* desc) {
    StaticJsonBuffer<COOGS_JSON_BUFFER_SIZE> jb;
    JsonObject& obj = jb.createObject();
    //
    // Add headers
    //
    JSON_header(obj);
    //
    obj["error"] = na(error_code);
    obj["from"] = na(from);
    obj["description"] = na(desc);
    return this->publish_JSON((char*)COOGS_ERROR_TOPIC, obj, false);
}


///////////////////////////////////////////////////////////////////////////////////
// Publish statistiscs
//
int CoogleSensors::publish_stats() {
    StaticJsonBuffer<COOGS_JSON_BUFFER_SIZE> jb;
    JsonObject& obj = jb.createObject();
    //
    // Add headers
    //
    JSON_header(obj);
    //
    obj["status"] = COOGS_STATUS_READY;
    unsigned long ms = millis();
    unsigned long s = millis() / 1000UL;
    unsigned long h = s / 60;
    int id = h / 24;
    int is = s - (h * 60);
    int ih = h - (id * 24);
    char uptime[30];
    sprintf(uptime, "%d d, %ud h, %d s", id, ih, is);
    obj["up_time"] = uptime;
    sprintf(uptime, "%lu", ms);
    obj["up_ms"] = ms;
    obj["sensed_sent"] = sensed_sent;
    obj["publish_errors"] = publish_errors;
    obj["invalid_messages"] = invalid_messages;
    obj["buffer_overflows"] = json_buf_ovf;
    obj["message_overflows"] = json_msg_ovf;
    obj["serializing_errors"] = serialize_errors;

    return this->publish_JSON((char*)COOGS_STATS_TOPIC, obj, false);
}


///////////////////////////////////////////////////////////////////////////////////
// Publish merasured data
//
bool CoogleSensors::publish_measurement(char* measurement_name, int num_data, char** tags, float *values) {
    int len = 0;
    StaticJsonBuffer<COOGS_JSON_BUFFER_SIZE> jb;
    JsonObject& obj = jb.createObject();

    //
    // Add headers
    //
    JSON_header(obj);
    //
    // Add measurement name and data tags and values
    //
    obj.set("name", measurement_name);
    for (int idatum = 0; idatum < num_data; idatum++) {
        obj.set(tags[idatum], values[idatum]);
    }
    obj["n_data"] = num_data;
    if (!obj.set("n_data", num_data)) {
        this->error("Can't publish message. JSON Buffer too small?");
        return false;
    }
    
    // publish
    //
    char sensor_topic[2 + strlen(COOGS_DATA_TOPIC_PREFIX) + strlen(measurement_name)];
    strcpy(sensor_topic, COOGS_DATA_TOPIC_PREFIX);
    strcat(sensor_topic, measurement_name);
    sensed_sent++;
    return publish_JSON(sensor_topic, obj, true);
}


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////// CALLBACK ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

//
// Handle commands received from MQTT. CSensor object defined in CoogleSensors.h
//

void mqttCallbackHandler(char* topic, byte* payload, unsigned int len)
{

    if (!CoogleThat) return; // CoogleSensor object not created, see CoogleSensor.h
  
    char command[128];
    if (len >= (sizeof(command) - 1)) len = sizeof(command) - 1;
    memcpy(command, payload, len);
    command[len] = 0;

    CoogleThat->logPrintf(INFO, "COMMAND: %s ", command);

    if (strcmp(topic, COOGS_COMMAND_TOPIC) == 0) {
        CoogleThat->logPrintf(INFO, "Received command: %s", command);
        if (strstr(command, COOGS_COMMAND_STATUS) == &command[0]) {
            CoogleThat->publish_heartbeat();
        }
        else if (strstr(command, COOGS_COMMAND_STATISTICS) == &command[0]) {
            CoogleThat->publish_stats();
        }
        else if (strstr(command, COOGS_COMMAND_RESTART) == &command[0]) {
            CoogleThat->publish_shutdown_ack();
            CoogleThat->info("restart command received");
            CoogleThat->publish_status(COOGS_STATUS_SHUTDOWN);
            delay(300);
            ESP.restart();
        }
        else {
            CoogleThat->publish_error(COOGS_ERROR_BAD_COMMAND, "mqttCallbackHandler", command);
            CoogleThat->error("Invalid command");
        }
    }
    else {
        CoogleThat->publish_error(COOGS_ERROR_UNSUSCRIBED, "mqttCallbackHandler", topic);
        CoogleThat->logPrintf(ERROR, "Unexpected (unsuscribed) topic: %s", topic);
    }
}
