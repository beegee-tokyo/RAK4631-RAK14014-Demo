/**
 * @file init.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Initialize Serial and WB_IO2
 * @version 0.1
 * @date 2022-12-10
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>

// Initialize Serial for debug output
Serial.begin(115200);

pinMode(WB_IO2, OUTPUT);
digitalWrite(WB_IO2, HIGH);

time_t serial_timeout = millis();
// On nRF52840 the USB serial is not available immediately
while (!Serial)
{
	if ((millis() - serial_timeout) < 5000)
	{
		delay(100);
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	}
	else
	{
		break;
	}
}
