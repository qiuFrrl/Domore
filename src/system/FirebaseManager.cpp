#include "system/FirebaseManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include "database.h"
#include "config/PairCode.h"

namespace robodesk
{
// GTS Root R1 — Root CA for Google Trust Services (used by Firebase RTDB)
// Valid until: 2036-06-22
const char FIREBASE_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw
CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU
MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDaw
MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp
Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA
A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo
27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w
Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSME3yNBnk/MV4ykq4SbyYC
m0hEGjzDIChJzYzBf8dLVMFE9Vbdwo1iWAGR645bheEhzFWHMdaBaD0TmeA4p8aI
LQhOUyLbf/1DfIHKIcF1dT5RBkmUO3YCX50zNc+03YHJFIE1XBKQ6P3+B5MZoKPB
Yz8GnGBEPddWnWOAK5h6aPGwQ5SOn2gnQJkJ8SaUBpymv/OMZYEJb4mEaMg1NMMX
TJOQSO8JiGlW1+E9OGMT/wB8QMRz+Jbr3oLmFW8kTidZaEE0GceM1dS5u7ty3Pd5
u6S15vv1jl/8Fisvj6AZpMgpS7FUHGXPQP7pXEbFJSTXOX3eKRDhtvPGb2HQNB+m
p1d4Q2d50swJi/j3GEq2f52JjNd0Y54afZfLjhN/M2r1GVhIjjMM/2VPyObZ9JhM
Dp+8SDAj3ekFOjMgiRHSbRV/36dSsQz5qN79w3SG9C8n6RYbMh0JR3xWF19kZ9OB
bD70uf1fJcsOPLMPaOzaoOY2HHjKJEkxz8xNTwbPowIDAQABo0IwQDAOBgNVHQ8B
Af8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQU5K8rJnEaK0gnhS9S
ZizXmmKR7WkwDQYJKoZIhvcNAQEMBQADggIBAJ+qQibbC5u+/x6Wki4+XNHEkzp1
EceMddgMkORfYgLQYOvzI5aL+pFT/Ge3O3fH9a2cYFikFclQbpIB5Ij9QW+UPCkL
lPDBoVXFBE7t9N5/m0KqlY1sOBgpK7gIwgFxzC+LPV/yOZOUZv/xvGiSP0lMmPhF
m4mNJJezVQJEMfayjsso8gEshBEn4xbVXxKV0VCFnbP0bgXaJQ0YFVhFY1Whi7a8
0GbFXJViEa0HYdRb7xA/sM3uYOKS1FI/P9yPE2g6ZG2JY1uXL7cIag4FR4EzWls
y/u0G7SpEl/k+X/FRuiTMQGKLSxIf0c4RFREDrKv9FYmUFJFyOtjfhS0qBIAY71e
K+5GSqpd8MZs0qH6bk6oXkhJ7VSwGiRMRCxJmp1t2igXMKBBZ2fHYT85kUhVOF8c
VyPFwTNEV0bPu/l2kN/B/BINnK4ggaFxGF2S30hgCCjflq/W3B8rCKTE5vtlcQzV
aXUf7qHJnFX2mQGMFtax9cMHO3jBOCKGBCK4BVTPB3YDMcgVJFVj+dFOpBJfBABn
J3mLfls/E2j5dIco5MhIQqEFrL+GAuFHj1wR1zcIX/3u8ZDrMg+mfnKu/CcF0E1D
KPJDZFlGl8VE6j/pZ0gZgkbQIVz7lMOBEmJCbmIMWKlLig4Tfd3DwES3OiFX4d21
EliHJBpBsLsBF2W8
-----END CERTIFICATE-----
)EOF";

void FirebaseManager::fetchWifi(WifiManager &wifiManager)
{
    if (!wifiManager.isConnected())
    {
        return;
    }

    WiFiClientSecure client;
    client.setCACert(FIREBASE_ROOT_CA);

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

    wifiManager.clearDynamicCredentials();

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
                wifiManager.addCredential(ssid, password);
                Serial.printf("FirebaseManager: Fetched %s\n", ssid);
            }
        }
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
    client.setCACert(FIREBASE_ROOT_CA);

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
    client.setCACert(FIREBASE_ROOT_CA);

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
