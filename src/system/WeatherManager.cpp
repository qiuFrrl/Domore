#include "system/WeatherManager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <string.h>

#include "config/AppConfig.h"

namespace robodesk
{
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
        client.setInsecure();

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
