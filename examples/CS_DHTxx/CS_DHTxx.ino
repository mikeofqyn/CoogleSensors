///////////////////////////////////////////////////////////////////////////////////
// Send temperature and humidity data to an MQTT broker using CoogleSensor lib.
// See CoogleSensor's README.md for documentation
///////////////////////////////////////////////////////////////////////////////////

//                                                                             ///
// CoogleSensor provides a configuration web portal and MQTT integration using ///
// CoogleIOT library                                                           /// 
//                                                                             ///

#include <CoogleSensors.h>

CoogleSensors CS;

#include <CoogleSensors.h>

//
// DHT Humidity / Temperature sensors library for ESP microcontrollers
//

#include <DHTesp.h>

#ifndef ESP8266
#pragma message(DHTxx LIBRARY IS FOR ESP8266 ONLY!)
#error USE AN ESP8266 BOARD.
#endif

#define SENSOR_TYPE DHT22  // DHT11 or DHT22
#define DHT_SENSOR_PIN 2   // DHT Sensor attached to GPIO2
#define xstr(a) str(a)
#define str(a) #a

DHTesp dht;

unsigned long DHT_MEASUREMENT_INTERVAL = 20000;  // 20s

char* MEASUREMENT_NAME = xstr(SENSOR_TYPE);
char* labels[] = { "temperature", "humidity" };
float values[] = { 0., 0. };


//
// SETUP
///////////////////////////////////////////////////////////////////////////////////

void setup()
{
    //
    // CoogleSensors library
    //
    bool initOK = CS.begin();

    //
    // DHT
    //
    // dht.setup(DHT_SENSOR_PIN, DHTesp::DHT22); // Connect DHT sensor 
    dht.setup(DHT_SENSOR_PIN, DHTesp::SENSOR_TYPE); // Connect DHT sensor

    if (DHT_MEASUREMENT_INTERVAL < dht.getMinimumSamplingPeriod()) {
        DHT_MEASUREMENT_INTERVAL = dht.getMinimumSamplingPeriod();
    }

    //
    // Finished 
    //
    CS.info(initOK ? "Sensor initialized" : "Sensor not initialized");
}

//
// MAIN LOOP 
///////////////////////////////////////////////////////////////////////////////////

unsigned long last_measurement = millis();

void loop()
{
    ///
    // Important, do CoogleIOT housekeeping stuff 
    //////////////////////////////////////////////
    CS.loop();   

    //
    // Check if measurement interval has elapsed
    //
    if ((millis() - last_measurement) < DHT_MEASUREMENT_INTERVAL) {
        return;
    }
    last_measurement = millis();

    //
    // Read from sensor and publish
    //
    float temperature = dht.getTemperature();
    float humidity = dht.getHumidity();

    Serial.print("MQTT: "); Serial.print(CS.is_online()? "ONLINE" : "OFFLINE");
    Serial.print("\tDHT Status: ");
    Serial.print(dht.getStatusString());
    Serial.print("\tHumidity: ");
    Serial.print(humidity, 1);
    Serial.print("%\tTemp: ");
    Serial.print(temperature, 1);
    Serial.print(" C\t");
    Serial.print(dht.toFahrenheit(temperature), 1);
    Serial.print(" F\tHeat Index: ");
    Serial.println(dht.computeHeatIndex(temperature, humidity, false), 1);

    if (CS.is_online()) {
        values[0] = temperature;
        values[1] = humidity;
        CS.publish_measurement(MEASUREMENT_NAME, 2, labels, values);
    }

}
