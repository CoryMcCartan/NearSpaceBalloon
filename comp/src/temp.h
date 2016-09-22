/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#include <Wire.h>

#ifndef TEMP_H_
#define TEMP_H_

/**
 * Set up sensor.
 *
 * @return true if setup worked, false if not.
 */
bool setupTemperatureSensor();

/**
 * Reads current temperature from sensor.
 *
 * @return current temperature, in Kelvins.
 */
float getTemperature();

/**
 * Converts temperature in Kelvin to Celsius.
 *
 * @param kelvin a temperature in Kelvin
 * @return the temperature in Celsius.
 */
float toCelsius(float kelvin);

/**
 * Converts temperature in Kelvin to Farenheit.
 *
 * @param kelvin a temperature in Kelvin
 * @return the temperature in Farenheit.
 */
float toFarenheit(float kelvin);

#endif // TEMP_H_
