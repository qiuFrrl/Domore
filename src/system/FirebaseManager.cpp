#include "system/FirebaseManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <vector>

#include "database.h"
#include "config/PairCode.h"
#include "system/WifiTypes.h"

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

    DynamicJsonDocument doc(2048);
    const DeserializationError error = deserializeJson(doc, http.getStream());
    http.end();

    if (error)
    {
        Serial.printf("Firebase JSON failed: %s\n", error.c_str());
        return;
    }

    // Kumpulkan daftar baru dari Firebase ke vector sementara dulu
    std::vector<WifiCredential> incoming;
    JsonObject root = doc.as<JsonObject>();
    for (JsonPair kv : root)
    {
        JsonObject wifiObj = kv.value().as<JsonObject>();
        if (!wifiObj.isNull())
        {
            const char *ssid = wifiObj["ssid"] | "";
            const char *password = wifiObj["password"] | "";
            if (ssid[0] != '\0')
            {
                WifiCredential cred;
                cred.ssid = ssid;
                cred.password = password;
                incoming.push_back(cred);
            }
        }
    }

    // Jika data sama persis dengan yang ada, tidak perlu tulis NVS
    if (wifiManager.credentialsMatchList(incoming))
    {
        Serial.println("FirebaseManager: WiFi credentials unchanged, skipping NVS write");
        return;
    }

    // Ada perubahan, bersihkan lama dan simpan yang baru
    wifiManager.clearDynamicCredentials();
    for (const auto &cred : incoming)
    {
        wifiManager.addCredential(cred.ssid, cred.password);
        Serial.printf("FirebaseManager: Fetched %s\n", cred.ssid.c_str());
    }
}

void FirebaseManager::updateStatus_raw(bool connected, bool wasConnected, uint32_t nowMs)
{
    // WiFi baru terkoneksi: kirim status online langsung
    if (connected && !wasConnected)
    {
        setStatus("online");
        _statusSynced = true;
        _lastHeartbeatMs = nowMs;
    }
    // WiFi baru putus
    else if (!connected && wasConnected && _statusSynced)
    {
        _statusSynced = false;
        Serial.println("FirebaseManager: WiFi lost (networkTask)");
    }
    // Heartbeat periodik
    else if (connected && _statusSynced && (nowMs - _lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS))
    {
        sendHeartbeat();
        _lastHeartbeatMs = nowMs;
    }
}

void FirebaseManager::updateStatus(WifiManager &wifiManager, uint32_t nowMs)
{
    const bool connected = wifiManager.isConnected();

    // WiFi just connected → set online + immediate heartbeat
    if (connected && !_wasConnected)
    {
        setStatus("online");
        _statusSynced = true;
        _lastHeartbeatMs = nowMs;
    }
    // WiFi just disconnected
    else if (!connected && _wasConnected && _statusSynced)
    {
        _statusSynced = false;
        Serial.println("FirebaseManager: WiFi lost, status will be stale until reconnect");
    }
    // Periodic heartbeat while connected
    else if (connected && _statusSynced && (nowMs - _lastHeartbeatMs >= HEARTBEAT_INTERVAL_MS))
    {
        sendHeartbeat();
        _lastHeartbeatMs = nowMs;
    }

    _wasConnected = connected;
}

void FirebaseManager::setStatus(const char *status)
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(4000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String url = String(DATABASE_URL) + "/robot/account/" + PAIR_CODE + ".json";
    if (!http.begin(client, url))
    {
        Serial.println("FirebaseManager: setStatus HTTP begin failed");
        return;
    }

    String payload = "{\"nickname\":\"" + String(PAIR_CODE) + "\","
                     "\"status\":\"" + String(status) + "\","
                     "\"updatedAt\":{\".sv\":\"timestamp\"}}";

    http.addHeader("Content-Type", "application/json");
    const int httpCode = http.PATCH(payload);

    if (httpCode == HTTP_CODE_OK)
    {
        Serial.printf("FirebaseManager: Status set to '%s'\n", status);
    }
    else
    {
        Serial.printf("FirebaseManager: setStatus failed, HTTP %d\n", httpCode);
    }

    http.end();
}

void FirebaseManager::sendHeartbeat()
{
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setTimeout(4000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    String url = String(DATABASE_URL) + "/robot/account/" + PAIR_CODE + "/updatedAt.json";
    if (!http.begin(client, url))
    {
        Serial.println("FirebaseManager: heartbeat HTTP begin failed");
        return;
    }

    // PUT server timestamp directly to updatedAt field
    http.addHeader("Content-Type", "application/json");
    const int httpCode = http.PUT("{\".sv\":\"timestamp\"}");

    if (httpCode == HTTP_CODE_OK)
    {
        Serial.println("FirebaseManager: Heartbeat sent");
    }
    else
    {
        Serial.printf("FirebaseManager: Heartbeat failed, HTTP %d\n", httpCode);
    }

    http.end();
}
}
