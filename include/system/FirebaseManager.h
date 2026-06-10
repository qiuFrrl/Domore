#pragma once

#include <Arduino.h>
#include "system/WifiManager.h"

namespace robodesk
{
class FirebaseManager
{
public:
    void fetchWifi(WifiManager &wifiManager);
};
}
