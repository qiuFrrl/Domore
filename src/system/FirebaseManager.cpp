#include "system/FirebaseManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "database.h"

namespace robodesk
{
void FirebaseManager::fetchWifi(WifiManager &wifiManager)
{
    if (!wifiManager.isConnected())
    {
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(4000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String url = String(DATABASE_URL) + "/robot/wifi.json";
    if (!http.begin(client, url))
    {
        Serial.println("Firebase HTTP begin failed");
        return;
    }

    const int status = http.GET();
    if (status != HTTP_CODE_OK)
    {
        http.end();
        Serial.printf("Firebase HTTP failed: %d\n", status);
        return;
    }

    const String payload = http.getString();
    http.end();

    DynamicJsonDocument doc(2048);
    const DeserializationError error = deserializeJson(doc, payload);

    if (error)
    {
        Serial.printf("Firebase JSON failed: %s\n", error.c_str());
        return;
    }

    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root)
    {
        JsonObject wifiObj = kv.value().as<JsonObject>();
        if (!wifiObj.isNull())
        {
            String ssid = wifiObj["ssid"] | "";
            String password = wifiObj["password"] | "";
            if (ssid.length() > 0)
            {
                wifiManager.addCredential(ssid, password);
                Serial.printf("FirebaseManager: Fetched %s\n", ssid.c_str());
            }
        }
    }
}
}
