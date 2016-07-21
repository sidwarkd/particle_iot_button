// This #include statement was automatically added by the Particle IDE.
#include "HttpClient.h"
//#define LOGGING

int LED = D7;
int BUTTON = D2;

HttpClient http;
http_request_t request;
http_response_t response;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/x-www-form-urlencoded" },
    { "User-Agent", "Photon/0.5.2"},
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers with NULL
};

// Struct for keeping track of register state and job id
struct CloudInfo{
  uint8_t isRegistered;
  char requestId[33];
};

void callCloudService(void);
void registerDevice(void);

void setup() {

    #ifdef LOGGING
    Serial.begin(9600);
    #endif

    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);

    // Set up the request here to save processing on wakeup
    request.hostname = "iot-button.appspot.com";
    request.port = 80;

    registerDevice();

    // Go into a low power mode
    System.sleep(BUTTON, RISING);
}

void loop() {
    // Make a cloud service call
    callCloudService();
    
    // Go back to sleep
    System.sleep(BUTTON, RISING);
}

void registerDevice(){
    int indexOfValue;
    String requestId;
    CloudInfo info;

    EEPROM.get(0, info);
    if(info.isRegistered != 1){
        #ifdef LOGGING
        Serial.println("Registering device...");
        #endif

        // First run, let's register the device
        request.path = "/register";
        request.body = String("deviceId=" + System.deviceID() + "&name=iot_button&f=json");
        http.post(request, response, headers);

        // If registration was successful let's create a hard-coded
        // job for now
        if(response.status == 200){
            #ifdef LOGGING
            Serial.println("Device successfully registered. Adding job");
            #endif

            // Create a hard-coded Ubidots job for now
            request.path = "/jobs/save";
            request.body = String("deviceId=" + System.deviceID() + "&method=POST&f=json&requestUrl=http://things.ubidots.com/api/v1.6/devices/iot-button?token=ECurkqBlQkpYA3WvpLlBWh6sAcDLnNOJjNkJXkjVgmZHNaNXzkE7JZQt0KdF&headers={\"Content-Type\":\"application/json\"}&payload={\"last_press\":10}");
            http.post(request, response, headers);

            if(response.status == 200){
                #ifdef LOGGING
                Serial.println("Job created successfully.");
                #endif

                // Parse the job id and update CloudInfo in EEPROM
                indexOfValue = response.body.indexOf("requestId");

                #ifdef LOGGING
                Serial.println(response.body);
                Serial.print("Index of requestId: ");
                Serial.println(indexOfValue);
                #endif

                if(indexOfValue != -1){
                    indexOfValue += 13;
                    requestId = response.body.substring(indexOfValue, indexOfValue + 32);

                    strcpy(info.requestId,requestId.c_str());

                    #ifdef LOGGING
                    Serial.println("Parsed requestId: " + requestId);
                    Serial.print("info.requestId:");
                    Serial.println(String(info.requestId));
                    #endif

                    info.isRegistered = 1;
                    Serial.println("Writing CloudInfo to EEPROM");
                    EEPROM.put(0, info);
                }
            }
            #ifdef LOGGING
            else{
                Serial.println("Job creation failed");
            }
            #endif
        }
        #ifdef LOGGING
        else{
            Serial.println("Register failed: " + String(response.status));
        }
        #endif
    }
    #ifdef LOGGING
    else{
      Serial.println("Device already registered");
    }
    #endif
}

void callCloudService(){
    CloudInfo info;

    request.path = "/jobs/execute";

    // Send the deviceId and requestId
    EEPROM.get(0, info);
    if(info.isRegistered == 1){
        pinSetFast(LED);
        request.body = String("deviceId=" + System.deviceID() + "&requestId=" + String(info.requestId) + "&f=json");
        #ifdef LOGGING
        Serial.println(request.body);
        #endif
        http.post(request, response, headers);
        pinResetFast(LED);
    }
}
