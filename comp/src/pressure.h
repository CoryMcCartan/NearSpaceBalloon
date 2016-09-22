/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#include <SFE_BMP180.h>
#include <Wire.h>

#ifndef PRESSURE_H_
#define PRESSURE_H_

/**
 * Set up sensor.
 *
 * @return true if setup worked, false if not.
 */
bool setupPressureSensor();

/**
 * Reads current pressure from sensor.
 *
 * @return current pressure, in millibars.
 */
float getPressure();

#endif // PRESSURE_H_
