﻿#include <stdlib.h>
#include "arduino.h"
#include "../config.h"
#include "../pins.h"
#include "../rtc/datatime.h"
#include "../sd/cardrw.h"
#include "../utility/errors.h"
#include "../utility/buzzer.h"
#include "../utility/debug.h"
#include "../lcd/display.h"

#include "manage.h"
#include "feed.h"

Card_rw_Class card_rw;
Error_Class error;
Buzzer_Class buzzer;
Feed_Class feed;

void Manage_Class::setup()
{
	// do the setup of all of the modules
	// also load all the data form the datastore
	data_time.rtc_setup();
	card_rw.setup();
	feed.setup();
	buzzer.setup();

	//card_rw.load_base_data();
	//card_rw.load_cycle_data();
	//card_rw.load_daily_data();

	pinMode(LED_BOARD_PIN, OUTPUT);
}

// build the food data that have to be displayed
// if the food has not been released use the original_gr_meal
// if has been released use the adj_gr_meal that have the exact value dropped 
void Manage_Class::update_current_food_schedule()
{
	uint16_t current_food_array[n_meals]; // allocate an array with n_meals len

	// copy the std valued in the current_food_array
	memcpy(current_food_array, original_gr_meal, sizeof(uint16_t)*n_meals);

	// update just the already dropped values with the adj_gr_meal
	// if a value has been dropped set the value as negative
	// the negative value is used in the display function to know that has been dropped
	for (int8_t i = 0; i < index_of_this_meal; i++)
	{
		current_food_array[i] = adj_gr_meal[i] * -1;  // * -1 convert to negative
	}
	
	// if there is a future adj weight in the array display its
	// the future value is update once the previour cycle is finisced
	if (adj_gr_meal[index_of_this_meal] != 0)
		current_food_array[index_of_this_meal] = adj_gr_meal[index_of_this_meal];
	
	display.today_food(current_food_array, n_meals);
}

// update the display the food data that have to be displayed
// get the futire time from timetable that is builted as:
// {hour1, minute1, hour2, minute2, ...}
void Manage_Class::update_next_food_schedule()
{
	int8_t hour = timetable[index_of_this_meal * 2];
	
	// the misutes are saved in range 1-4 as quarters.
	// to get the right quarter you have to multiply the value * 15
	int8_t minute = timetable[index_of_this_meal * 2 + 1] * 15;

	// update the value in the display. 
	// This will update only the string and not also refresh the display
	display.next_food_schedule(hour, minute);
}

// calculate how many days has been past since the birth of the dog
void Manage_Class::past_life()
{
	if (day >= date_of_birth_dog[0])
		d_nope = 0; //il giorno è stato superato
	else
		d_nope = 1; //il giorno non è stato superato

	if (year == date_of_birth_dog[2] && month >= date_of_birth_dog[1])
	{
		past_time_months = past_months = (month - date_of_birth_dog[1]) - d_nope;
	}
	else
	{
		past_time_years = year - date_of_birth_dog[2];
		past_time_months = ((past_time_years * 12 - date_of_birth_dog[1]) + month) - d_nope;
		past_months = ((12 - date_of_birth_dog[1]) + month) - d_nope;
	}

	if (d_nope == 0)
		past_days = day - date_of_birth_dog[0];
	else
		past_days = (days_per_month[month - 1] - date_of_birth_dog[0]) + day;
}

bool Manage_Class::birthday()
{

	//controlla se è il giorno del suo compleanno
	if (month == date_of_birth_dog[1] && day == date_of_birth_dog[0])
	{
		happy_birthday = true;
		return true;
	}
	else
	{
		happy_birthday = false;
		return false;
	}
}

//calcola le porzioni in base ai pasti che deve dare al giorno
void Manage_Class::portion_calculation(uint16_t food)
{

	gr_today_food = gr_today_food_left = food;

	uint16_t waste;
	uint16_t portion;

	waste = gr_today_food % n_meals;
	portion = (gr_today_food - waste) / n_meals;

	original_gr_meal[0] = portion + waste;
	for (byte i = 1; i < n_meals; i++)
	{
		original_gr_meal[i] = portion;
	}
}

void Manage_Class::update_dayly_food_value()
{

	if (birthday() && its_adult)
	{

		if (food_for_oldness)
		{

			if (past_time_years >= age_for_mature && past_time_years < age_for_ageing)
			{
				its_adult = false;
				its_mature = true;
				its_ageing = false;
				adult_portion_to_calculate = true;
			}
			if (past_time_years >= age_for_ageing)
			{
				its_adult = false;
				its_mature = false;
				its_ageing = true;
				adult_portion_to_calculate = true;
			}
		}
	}

	if (adult_portion_to_calculate)
	{
		//situazione in cui è adulto o è il primo giorno in cui diventa adulto
		if (its_adult || (months_for_adult < past_time_months && past_time_years < age_for_mature))
		{
			portion_calculation(value_gr_food_adult);
			its_adult = true;
			adult_portion_to_calculate = false;
		}
		//situazione in cui è mature
		if (its_mature)
		{
			portion_calculation(value_gr_food_mature);
			adult_portion_to_calculate = false;
		}
		//situazione in cui è ageing
		if (its_ageing)
		{
			portion_calculation(value_gr_food_ageing);
			adult_portion_to_calculate = false;
		}
	}
	//situazione in cui è sotto o ha 15 mesi
	if (!its_adult && !its_mature && !its_ageing)
	{
		int16_t gap = value_gr_food_15_months[past_time_months] - value_gr_food_15_months[past_time_months - 1];
		uint16_t val = value_gr_food_15_months[past_time_months - 1] + ((gap / 30) * past_days);
		portion_calculation(val);
	}
}

void Manage_Class::firstStart()
{

	card_rw.load_base_data(); //carico i dati sul cane
	data_time.get_data_time(&year, &month, &day, &hour, &minute, &second);
	past_life(); //controlla quanto tempo ha il cane e lo memorizza nelle variabili di classe

	if (months_for_adult < past_time_months && past_time_years < age_for_mature)
	{
		portion_calculation(value_gr_food_adult);
		its_adult = true;
		its_mature = false;
		its_ageing = false;
	}

	if (food_for_oldness)
	{

		if (past_time_years >= age_for_mature && past_time_years < age_for_ageing)
		{
			its_adult = false;
			its_mature = true;
			its_ageing = false;
		}
		if (past_time_years >= age_for_ageing)
		{
			its_adult = false;
			its_mature = false;
			its_ageing = true;
		}
	}
}

bool Manage_Class::its_the_moment()
{

	while (index_of_this_meal < n_meals)
	{
		// check wich meal have to be erogated
		if (done_meal[index_of_this_meal] == true) 
			index_of_this_meal++;
		
		// late release, the our of the schedule has passed.
		else if (hour > timetable[index_of_this_meal * 2])
			return true;
		
		// is the time in the hour
		// the hour is right and we are weaiting the minute 
		else if (hour == timetable[index_of_this_meal * 2] && minute >= (15 * timetable[(index_of_this_meal * 2) + 1]))
			return true;

		return false;
	}
}

// check if the reset have to be done
bool Manage_Class::reset_to_do()
{
	if (hour == 0 && minute <= 1 && second <= 30)

		// if is not all done the user has not check the machine
		if (!today_all_done)
			error.system_status(1);
		else
			return true;

	else
		return false;
}

bool Manage_Class::food_left_in_tank()
{

	if (tank_food_left <= 0)
		return error.system_status(FATAL_ERROR_9103); //false

	if (tank_food_left < STATIC_WARNING_THRESHOLD_LOW_FOOD)
		error.system_status(FATAL_ERROR_9103);

	if (adj_gr_meal[index_of_this_meal] > tank_food_left)
		return error.system_status(FATAL_ERROR_9102); //true

	//prox sensor assente
	//if (digitalWrite(PROX_SENSOR_1_PIN) == LOW) //controlla se il sesore organico vede le crochette nel serbatoio

	return true;
}

void Manage_Class::main_function()
{

	//get gate time from the rtc module
	data_time.get_data_time(&year, &month, &day, &hour, &minute, &second);

	// calculare daily values once per day and store them into the sd
	// do this only at the start of the day
	if (daily_ceck_to_do)
	{
		// check the age of the dog and if its is birthday
		past_life();

		// update variables of daily food
		update_dayly_food_value(); 

		index_of_this_meal = 0;
		adj_gr_meal[0] = original_gr_meal[0];

		daily_ceck_to_do = false; //controllo eseguito
		update_current_food_schedule(); // update food schedule on display
		update_next_food_schedule(); // update nex food time on display	
		display.update();

		card_rw.save_daily_data();
	}

	if (!today_all_done && !error_occur_in_feed)
	{ //se c'è un errore in erogazione viene bloccato il ciclo di erogazione

		if (its_the_moment() && food_left_in_tank() && SCHEDULED_AUTO_EROGATION_ENABLED || is_manual)
		{
			//controlla se è l'ora giusta e quale pasto si deve erogare
			// aggiorna l'indice index_of_this_meal
			// update the food left in the box

			// set the is_manual to false, when the food erogation has been triggered manually
			is_manual = false;

			DEBUG_PRINT("index_of_this_meal: ");
			DEBUG_PRINTLN(index_of_this_meal);

			DEBUG_PRINTLN("Start Erogation!");
			display.start_erogation();

			// drop the portion and update the done_meal array if success
			done_meal[index_of_this_meal] = feed.feed(adj_gr_meal[index_of_this_meal]);

			// the amount of foot erogated in this cycle
			int16_t erogated_food = feed.get_total_currently_weight();

			DEBUG_PRINTLN("Erogation Done!");
			DEBUG_PRINT("Erogated weight in g: ");
			DEBUG_PRINTLN(erogated_food);

			// if the meal has been correctly released
			if (done_meal[index_of_this_meal])
			{	
				// if this meal is under the number of daily meals
				if (index_of_this_meal < n_meals)
				{
					// adjust the next meal
					int8_t adj_value = (erogated_food - adj_gr_meal[index_of_this_meal]);
					adj_gr_meal[index_of_this_meal + 1] = original_gr_meal[index_of_this_meal + 1] - adj_value;
					
					// update the erogated food value
					adj_gr_meal[index_of_this_meal] = erogated_food;

					index_of_this_meal++;
					DEBUG_PRINT("Next meal weight: ");
					DEBUG_PRINTLN(adj_gr_meal[index_of_this_meal]);
				}
				else
				{
					today_all_done = true;
					DEBUG_PRINTLN("All done for today");
				}

				// update food left in tank
				tank_food_left -= erogated_food;
				DEBUG_PRINT("Food left in the tank: ");
				DEBUG_PRINTLN(tank_food_left);

				update_current_food_schedule(); // update values on display
				update_next_food_schedule(); // update next hour on display
				display.operation_completed();
				display.update();
			}

			card_rw.save_cycle_data();
		}
	}

	if (today_all_done)
	{
		card_rw.save_cycle_data();
		card_rw.save_daily_data();
		card_rw.save_record();
	}

	// daily reset
	if (reset_to_do())
	{
		DEBUG_PRINTLN("Daily Reset");

		// reset all the values
		daily_ceck_to_do = true;
		today_all_done = false;
		index_of_this_meal = 0;

		for (int i = 0; i < 4; i++)
		{ //resetta i pasti erogati
			done_meal[i] = false;
			adj_gr_meal[i] = 0;
		}

		void(* reboot)(void) = 0;
		reboot();
	}
}

// once the button on the display is pressed it will be erogated a single food dose
void Manage_Class::manual_erogation()
{
	is_manual = true;
	DEBUG_PRINTLN("Manual Erogation");
}
