#include <U8glib.h>
#include "arduino.h"
#include "../pins.h"
#include "../config.h"
#include "../utility/debug.h"
#include "../planner/manage.h"
#include "../rtc/datatime.h"

#include "display.h"
#include "main_screen_bitmap.h"
#include "custom_bootscreen.h"

U8GLIB_ST7920_128X64_1X u8g(LCD_ENABLE_PIN, LCD_RW_PIN, LCD_RS_PIN); // SPI Com: e=18,rw=17,rs=16

// to display the current system time
char time_string[12];

// to display the next time when the food will be erogated
char food_next_schedule_string[16];

// current text status of the system
char status_text[20];

// the status of the system
int16_t food_weight_tot = 0;
int16_t food_target_tot = 0;
int16_t food_weight = 0;
int16_t food_target = 0;
char food_display[28];

// value to know when the system has done the init cycle
bool is_ready = false;

// do pre operations on the display
void Display_Class::update_status()
{
	switch (display_status_flag)
	{
	case S_READY:
		sprintf(status_text, "Ready");
		break;
	case S_EROGATION:
		sprintf(status_text, "T:%d/%d N:%d/%d\0", food_weight_tot, food_target_tot, food_weight, food_target);
		break;
	}
}

void Display_Class::update_time(){
	uint16_t year;
	byte month;
	byte day;
	byte hour;
	byte minute;
	byte second;

	data_time.get_data_time(&year, &month, &day, &hour, &minute, &second);
	sprintf(time_string, "%02d/%02d %02d:%02d", month, day, hour, minute);
}

// start food erogation once the button is pressed
void Display_Class::update_display_button_state()
{
	if (digitalRead(BTN_ENC_PIN) == LOW)
	{
		u8g.sleepOff(); //power on the display
		DEBUG_PRINTLN("Button Pressed");
		manage.manual_erogation();
	}
}


void Display_Class::display_startup_screen()
{
	u8g.firstPage();
	do {
		u8g.setFont(u8g_font_fub11);
		u8g.drawStr(5, 35, "Power Up");

		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(0, 63, VERSION);
	} while (u8g.nextPage());
}


void Display_Class::display_main_screen()
{
	// u8g.drawXBM(0, 20, 10, 10, LOGO);
	u8g.firstPage();
	do {
		// title
		u8g.setFont(u8g_font_fub11);
		u8g.drawStr(0, 12, "Laika");

		// time
		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(60, 8, time_string);

		// Next food erogation scheduled
		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(0, 30, food_next_schedule_string);

		// food data
		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(0, 40, "Food for today:");
		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(0, 50, food_display);
		
		// status data
		u8g.setFont(u8g_font_6x12);
		u8g.drawStr(0, 63, status_text);
	} while (u8g.nextPage());
}

void Display_Class::next_food_schedule(int8_t hour, int8_t minute){
	sprintf(food_next_schedule_string, "Next on: %02d:%02d\0", hour, minute);
}

// the values are negative if the food is erogated and positive if not
void Display_Class::today_food(uint16_t *values, uint8_t n_meals){

	food_display[0] = '\0';
	char single_meal[6];

	for (int i = 0; i < n_meals; i++)
	{
		sprintf(single_meal, "%d \0", values[i]);
		if ((int16_t)values[i] < 0)
			single_meal[0] = '!';
		
		strcat(food_display, single_meal);
	}
}

void Display_Class::food_val(int16_t weight, int16_t target, int16_t weight_tot, int16_t target_tot){
	food_weight = weight;
	food_target = target;
	food_weight_tot = weight_tot;
	food_target_tot = target_tot;

	update_status();
	display_main_screen();
}

// force the update of the display externally
void Display_Class::update(){
	update_status();
	display_main_screen();
}


bool Display_Class::setup()
{

	if (u8g.getMode() == U8G_MODE_R3G3B2)
	{
		u8g.setColorIndex(255);
	}
	else if (u8g.getMode() == U8G_MODE_GRAY2BIT)
	{
		u8g.setColorIndex(3);
	}
	else if (u8g.getMode() == U8G_MODE_BW)
	{
		u8g.setColorIndex(1);
	}
	else if (u8g.getMode() == U8G_MODE_HICOLOR)
	{
		u8g.setHiColorByRGB(255, 255, 255);
	}

	pinMode(BTN_EN1_PIN, INPUT_PULLUP);
	pinMode(BTN_EN2_PIN, INPUT_PULLUP);
	pinMode(BTN_ENC_PIN, INPUT_PULLUP);

	display_startup_screen();
}

// main function that will periodically update all the ui

void Display_Class::update_ui()
{
	// on fist loop set the display ready
	if (is_ready == false){
		is_ready = true;
		display_status_flag = S_READY;
		update();
	}

	update_time();
	update_display_button_state();
	display_main_screen();
}