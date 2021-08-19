#include <WiFi.h>
#include "FS.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_NeoPixel.h>

#define LEDPIN 18

const char *ssid = "YourSSID";
const char *password = "YourPassword";
const bool apMode = false;

AsyncWebServer *server;

Adafruit_NeoPixel pixel(1, LEDPIN, NEO_GRB);

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Color Picker");
    pixel.begin();
    pixel.clear();

    // ### Start SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    // ### Start WiFi
    if (apMode)
    {
        // Start access point
        WiFi.softAP(ssid, password);
        Serial.println();
        Serial.println("AP running");
        Serial.print("IP address: ");
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        // Connect to WiFi
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(1000);
            Serial.println("Connecting to WiFi..");
        }
        Serial.println("Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }

    // ### Start Server
    server = new AsyncWebServer(80);

    // ### Server Routes
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/index.html", "text/html"); });

    server->on("/index.js", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/index.js", "text/javascript"); });

    server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
               { request->send(SPIFFS, "/favicon.ico", "image/x-icon"); });

    // ### API
    server->on(
        "/api/led", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
        {
            char *str = (char *)data;
            Serial.println(atoi(str));
            pixel.setPixelColor(0, pixel.gamma32(atoi(str)));
            pixel.show();

            request->send(200);
        });

    server->begin();
}

void loop()
{
}
