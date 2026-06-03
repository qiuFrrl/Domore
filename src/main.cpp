#include <Arduino.h>

#include "app/RobodeskApp.h"

robodesk::RobodeskApp app;

void setup()
{
    Serial.begin(115200);
    delay(100);
    app.begin();
}

void loop()
{
    app.update();
}
