#include "system/WeatherManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <string.h>

#include "config/AppConfig.h"

namespace robodesk
{
    // ISRG Root X1 — Root CA for Let's Encrypt (used by api.open-meteo.com)
    // Valid until: 2035-06-04
    static const char OPEN_METEO_ROOT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogZiUvsKEXnpYA6+7kA3PEBDmW0OzDnPmBk2Mjn6hc4nKe5
kO2D1LbSR2fP9NwSt6+AI1kAJBBicAmCOnXIBScCfG4oM4sF2rZVhLcxBHRKMQIT
eDOp2RWXAMnFPISlFNQT92KQrKAN0lTIrLdMO6YUr1sO9W/uekuRDcYKncnBLMEN
RcMAi0rH0bWDJgFCUmgeSNJl3E6rm7McS7fvXUVQ3EHc+kGD5pFrJ0p1LD5gLiqM
cs8x2g/N7J+U7jRslZB5OaX/53TZ3eVeATmZhs0WD7sUFUMF4/NqFe0b95UnKLvI
cgJBB0pEYrIS0p6bbP9+W0SOBdm9ENWGMxICqIrnFY5mG5qMgP93jtd4F4OqEdFh
RF+GSHYqHaKCEwPROFSjuIUES2Xp/TN6MKpDg5lM1e3yfB/sWNlTDhRh0aSUanex
mDKhl/lLDbq0Bxe9bJo/VqiVrK3r0YFEi0YhB8QARAE7VmZ3URRiDMjg2YI=
-----END CERTIFICATE-----
)EOF";

    void WeatherManager::begin()
    {
        _data.valid = false;
        strncpy(_data.summary, "Waiting", sizeof(_data.summary));
        _data.summary[sizeof(_data.summary) - 1] = '\0';
    }

    void WeatherManager::update(uint32_t nowMs, bool wifiConnected, Coordinates coordinates)
    {
        if (!wifiConnected)
        {
            if (!_data.valid)
            {
                strncpy(_data.summary, "No WiFi", sizeof(_data.summary));
                _data.summary[sizeof(_data.summary) - 1] = '\0';
            }
            return;
        }

        if (!coordinates.valid)
        {
            if (!_data.valid)
            {
                strncpy(_data.summary, "No GPS", sizeof(_data.summary));
                _data.summary[sizeof(_data.summary) - 1] = '\0';
            }
            return;
        }

        const uint32_t interval = _data.valid ? WEATHER_REFRESH_MS : WEATHER_RETRY_MS;
        if (_lastAttemptAt != 0 && nowMs - _lastAttemptAt < interval)
        {
            return;
        }

        _lastAttemptAt = nowMs;
        refresh(coordinates);
    }

    bool WeatherManager::refresh(Coordinates coordinates)
    {
        if (!coordinates.valid)
        {
            return false;
        }

        char url[220];
        snprintf(
            url,
            sizeof(url),
            "https://api.open-meteo.com/v1/forecast?latitude=-2.9761&longitude=104.7754&current=temperature_2m,weather_code");

        WiFiClientSecure client;
        client.setCACert(OPEN_METEO_ROOT_CA);

        HTTPClient http;
        http.setTimeout(4000);
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        if (!http.begin(client, url))
        {
            strncpy(_data.summary, "HTTP begin", sizeof(_data.summary));
            _data.summary[sizeof(_data.summary) - 1] = '\0';
            Serial.println("Weather HTTP begin failed");
            return false;
        }

        const int status = http.GET();
        if (status != HTTP_CODE_OK)
        {
            http.end();
            snprintf(_data.summary, sizeof(_data.summary), "HTTP %d", status);
            Serial.printf("Weather HTTP failed: %d\n", status);
            Serial.println(http.errorToString(status));
            return false;
        }

        const String payload = http.getString();
        http.end();

        StaticJsonDocument<768> doc;
        const DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            strncpy(_data.summary, "JSON fail", sizeof(_data.summary));
            _data.summary[sizeof(_data.summary) - 1] = '\0';
            Serial.printf("Weather JSON failed: %s\n", error.c_str());
            return false;
        }

        JsonObject current = doc["current"];
        if (current.isNull())
        {
            const char *reason = doc["reason"] | "No current";
            strncpy(_data.summary, reason, sizeof(_data.summary));
            _data.summary[sizeof(_data.summary) - 1] = '\0';
            Serial.printf("Weather response missing current object: %s\n", reason);
            return false;
        }

        _data.temperatureC = current["temperature_2m"] | 0.0f;
        _data.humidity = 0;
        _data.weatherCode = current["weather_code"] | 0;
        strncpy(_data.summary, summaryForCode(_data.weatherCode), sizeof(_data.summary));
        _data.summary[sizeof(_data.summary) - 1] = '\0';
        _data.updatedAtMs = millis();
        _data.valid = true;

        Serial.printf(
            "Weather updated: %.1f C, %u%%, code %u\n",
            _data.temperatureC,
            _data.humidity,
            _data.weatherCode);

        return true;
    }

    const WeatherData &WeatherManager::data() const
    {
        return _data;
    }

    const char *WeatherManager::summaryForCode(uint16_t code) const
    {
        if (code == 0)
        {
            return "Clear";
        }
        if (code <= 3)
        {
            return "Cloudy";
        }
        if (code == 45 || code == 48)
        {
            return "Fog";
        }
        if ((code >= 51 && code <= 67) || (code >= 80 && code <= 82))
        {
            return "Rain";
        }
        if (code >= 71 && code <= 77)
        {
            return "Snow";
        }
        if (code >= 95)
        {
            return "Storm";
        }

        return "Weather";
    }
}
