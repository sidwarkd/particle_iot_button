// This #include statement was automatically added by the Particle IDE.
#include "HttpClient/HttpClient.h"

int LED = D7;
int BUTTON = D2;

HttpClient http;
http_request_t request;
http_response_t response;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    { "Content-Type", "application/x-www-form-urlencoded" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers with NULL
};

void callCloudService(void);

void setup() {
    pinMode(LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    
    // Set up the request
    request.hostname = "iot-button.appspot.com";
    request.port = 80;
    request.path = "/jobs/execute";
    
    //callCloudService(); // Call the register service here
    
    // Go into a low power mode
    System.sleep(BUTTON, RISING);
}

void loop() {
    // Make a cloud service call
    callCloudService();
    
    // Go back to sleep
    System.sleep(BUTTON, RISING);
}

void callCloudService(){
    pinSetFast(LED);
    delay(500);
    pinResetFast(LED);
    
    // Send the deviceId and requestId
    request.body = "deviceId=12345&requestId=3be49556e1ff4c0b8c69f0c52e949306&f=json";

    // Get request
    http.post(request, response, headers);
}