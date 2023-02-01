/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Simple demo for the RAK14014 TFT with touch screen
 * @version 0.1
 * @date 2022-12-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include "RAK14014_FT6336U.h"

#include "Free_Fonts.h" // Free fonts
#include "graphics.h" // the images

/** TFT display driver instance */
TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

/** Reset pin for touch screen */
// const uint8_t I2C_SDA = WB_I2C1_SDA;
// const uint8_t I2C_SCL = WB_I2C1_SCL;
const uint8_t RST_N_PIN = WB_IO5;
/** Interrupt pin for touch screen*/
const uint8_t INT_N_PIN = WB_IO6;

/** Touch screen driver instance */
FT6336U ft6336u;

/** Flag for touch screen event */
bool wait_for_touch = true;

/** Structure for touch event results */
struct touch_event_s
{
	uint16_t x_pos1;
	uint16_t y_pos1;
	uint16_t x_pos2;
	uint16_t y_pos2;
	bool double_touch;
};

/** Touch screen events data */
volatile touch_event_s touch_event;

/** Font number used for display */
uint8_t font_num = 16;
/** Font color used for display */
uint8_t font_color = 0;

/** Array with the Free Font used in the demo */
const GFXfont *font_select[] = {FF0, FF1, FF2, FF3, FF4, FF5, FF6, FF7, FF8, FF9, FF10, FF11, FF12, FF13, FF14, FF15, FF16};
/** Array with colors used in the demo */
uint16_t colors[] = {TFT_WHITE, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE,
					 TFT_GREEN, TFT_CYAN, TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_ORANGE, TFT_GREENYELLOW, TFT_PINK, TFT_BROWN, TFT_GOLD, TFT_SILVER,
					 TFT_SKYBLUE, TFT_VIOLET};

/** Flag for display on/off */
bool display_off = false;

/** Image to be shown on screen */
uint8_t current_image = 0;

/**
 * @brief Interrupt callback for touch screen
 * 
 */
static void keyIntHandle(void)
{
	wait_for_touch = false;
}

/**
 * @brief Arduino Setup function
 * 
 */
void setup()
{
	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	// Initialize Serial for debug output
	Serial.begin(115200);

	// Setup the LCD
	tft.init();
	tft.setRotation(3);
	tft.setSwapBytes(true);
	tft.fillScreen(TFT_BLACK);

	ft6336u.begin();

	attachInterrupt(digitalPinToInterrupt(INT_N_PIN), keyIntHandle, FALLING);

	tft.fillRect((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, TFT_BLACK);
	tft.pushImage((320 - boot_1_s_x) / 2, (240 - boot_2_s_y) / 4, boot_1_s_x, boot_1_s_y, boot_1_s);
	tft.pushImage((320 - boot_2_s_x) / 2, (240 - boot_2_s_y) / 2, boot_2_s_x, boot_2_s_y, boot_2_s);
	tft.pushImage(0, 210, left_arrow_x, left_arrow_y, left_arrow);
	tft.pushImage(290, 210, right_arrow_x, right_arrow_y, right_arrow);

	tft.setTextFont(1);

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
}

/**
 * @brief Arduino loop function
 * 
 */
void loop()
{
	Serial.printf("Showing image # %d\n", current_image);
	tft.fillRect(35, 205, 254, 45, TFT_BLACK);
	if (current_image == 0)
	{
		tft.fillRect(60, 0, 200, 35, TFT_BLACK);
	}
	else
	{
		tft.fillRect(60, 0, 200, 40, TFT_BLACK);
		tft.setFreeFont(font_select[font_num]); // Select the font
		uint16_t txt1_wdth = 0;
		uint16_t txt2_wdth = 0;

		// Set the font colour to be white with a black background, set text size multiplier to 1
		tft.setTextColor(colors[font_color], TFT_BLACK, true);

		txt1_wdth = tft.textWidth("Hello");
		txt2_wdth = tft.textWidth("RAKstar");

		if (font_num == 0)
		{
			tft.setCursor(160 - (txt1_wdth / 2), 10);
			tft.println("Hello");
			tft.setCursor(160 - (txt2_wdth / 2), 220);
			tft.println("RAKstar");
		}
		else
		{
			tft.setCursor(160 - (txt1_wdth / 2), 30);
			tft.println("Hello");
			tft.setCursor(160 - (txt2_wdth / 2), 239);
			tft.println("RAKstar");
		}

		// We can now plot text on screen using the "print" class
		// tft.printf("F %d %04X", font_num, colors[font_color]);
		Serial.printf("F %d %04X\n", font_num, colors[font_color]);

		font_num++;
		font_color++;
		if (font_num == 17)
		{
			font_num = 0;
		}
		if (font_color == 24)
		{
			font_color = 0;
		}
	}

	attachInterrupt(digitalPinToInterrupt(INT_N_PIN), keyIntHandle, FALLING);

	while (wait_for_touch)
	{
		yield();
	}
	wait_for_touch = true;
	if (ft6336u.read_td_status() == 0)
	{
		// Serial.println("Finger up event");
		return;
	}
	detachInterrupt(digitalPinToInterrupt(INT_N_PIN));
	// Touch screen has rotated coordintes
	touch_event.y_pos1 = ft6336u.read_touch1_x();
	touch_event.x_pos1 = ft6336u.read_touch1_y();
	touch_event.y_pos2 = ft6336u.read_touch2_x();
	touch_event.x_pos2 = ft6336u.read_touch2_y();

	// Serial.print("FT6336U Touch Position 1 org: (");
	// Serial.print(touch_event.x_pos1);
	// Serial.print(" , ");
	// Serial.print(touch_event.y_pos1);
	// Serial.println(")");

	// Switch x axis if rotation is flipped
	if (tft.getRotation() == 3)
	{
		touch_event.y_pos1 = 240 - touch_event.y_pos1;
		touch_event.y_pos2 = 240 - touch_event.y_pos2;
	}

	uint8_t td_status = ft6336u.read_td_status();
	if (td_status == 2)
	{
		touch_event.double_touch = true;
	}
	else
	{
		touch_event.double_touch = false;
	}

	// Serial.print("FT6336U TD Status: ");
	// Serial.println(ft6336u.read_td_status());
	// Serial.print("FT6336U Touch Event/ID 1: (");
	// Serial.print(ft6336u.read_touch1_event());
	// Serial.print(" / ");
	// Serial.print(ft6336u.read_touch1_id());
	// Serial.println(")");
	// Serial.print("FT6336U Touch Position 1: (");
	// Serial.print(touch_event.x_pos1);
	// Serial.print(" , ");
	// Serial.print(touch_event.y_pos1);
	// Serial.println(")");
	// Serial.print("FT6336U Touch Weight/MISC 1: (");
	// Serial.print(ft6336u.read_touch1_weight());
	// Serial.print(" / ");
	// Serial.print(ft6336u.read_touch1_misc());
	// Serial.println(")");
	// Serial.print("FT6336U Touch Event/ID 2: (");
	// Serial.print(ft6336u.read_touch2_event());
	// Serial.print(" / ");
	// Serial.print(ft6336u.read_touch2_id());
	// Serial.println(")");
	// Serial.print("FT6336U Touch Position 2: (");
	// Serial.print(ft6336u.read_touch2_x());
	// Serial.print(" , ");
	// Serial.print(ft6336u.read_touch2_y());
	// Serial.println(")");
	// Serial.print("FT6336U Touch Weight/MISC 2: (");
	// Serial.print(ft6336u.read_touch2_weight());
	// Serial.print(" / ");
	// Serial.print(ft6336u.read_touch2_misc());
	// Serial.println(")");

	if (touch_event.double_touch)
	{
		Serial.println("Double Touch detected");
		if (display_off == false)
		{
			digitalWrite(TFT_BL, LOW);
			display_off = true;
		}
		else
		{
			digitalWrite(TFT_BL, HIGH);
			display_off = false;
		}
	}

	if (display_off)
	{
		// Backlight is off, do nothing
		return;
	}

	uint8_t old_image = current_image;

	// Check if go left
	if ((touch_event.y_pos1 > 200) && (touch_event.x_pos1 < 35))
	{
		Serial.println("Left");
		if (current_image == 0)
		{
			current_image = 3;
		}
		else
		{
			current_image -= 1;
		}
	}

	// Check if go right
	if ((touch_event.y_pos1 > 200) && (touch_event.x_pos1 > 285))
	{
		Serial.println("Right");
		if (current_image == 3)
		{
			current_image = 0;
		}
		else
		{
			current_image += 1;
		}
	}

	// Check if center push
	if ((touch_event.x_pos1 > 100) && (touch_event.x_pos1 < 220) && (touch_event.y_pos1 > 60) && (touch_event.y_pos1 < 180))
	{
		Serial.println("Center");
		current_image = 0;
	}

	Serial.printf("Next image is # %d\n", current_image);

	if (current_image == old_image)
	{
		return;
	}

	switch (current_image)
	{
	case 0:
		tft.fillRect((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, TFT_BLACK);
		tft.pushImage((320 - boot_1_s_x) / 2, (240 - boot_2_s_y) / 4, boot_1_s_x, boot_1_s_y, boot_1_s);
		tft.pushImage((320 - boot_2_s_x) / 2, (240 - boot_2_s_y) / 2, boot_2_s_x, boot_2_s_y, boot_2_s);
		break;
	case 1:
		tft.fillRect((320 - boot_1_s_x) / 2, (240 - boot_2_s_y) / 4, boot_1_s_x, boot_1_s_y, TFT_BLACK);
		tft.fillRect((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, TFT_BLACK);
		tft.pushImage((320 - feeling_good_s_x) / 2, (240 - feeling_good_s_y) / 2, feeling_good_s_x, feeling_good_s_y, feeling_good_s);
		break;
	case 2:
		tft.fillRect((320 - boot_1_s_x) / 2, (240 - boot_2_s_y) / 4, boot_1_s_x, boot_1_s_y, TFT_BLACK);
		tft.fillRect((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, TFT_BLACK);
		tft.pushImage((320 - unconfortable_s_x) / 2, (240 - unconfortable_s_y) / 2, unconfortable_s_x, unconfortable_s_y, unconfortable_s);
		break;
	case 3:
		tft.fillRect((320 - boot_1_s_x) / 2, (240 - boot_2_s_y) / 4, boot_1_s_x, boot_1_s_y, TFT_BLACK);
		tft.fillRect((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, TFT_BLACK);
		tft.pushImage((320 - exhausted_s_x) / 2, (240 - exhausted_s_y) / 2, exhausted_s_x, exhausted_s_y, exhausted_s);
		break;
	}
}
