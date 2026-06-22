#pragma once

#include <Arduino.h>
#include <string.h>

namespace robodesk
{
struct WifiCredential
{
    String ssid;
    String password;
};
}
