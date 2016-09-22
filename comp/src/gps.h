/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#ifndef GPS_H_
#define GPS_H_

#define LOW_ALTITUDE 1
#define HIGH_ALTITUDE 2

typedef struct {
    float latitude;
    float longitude;
    float altitude;
} Location;

/**
 * Set up GPS sensor. 
 * 
 * @return true if successful, false otherwise.
 */
bool setupGPS();

/**
 * Get current GPS coordinates and store them in given Location struct.
 */
void getGPSData(Location * position);

#endif // GPS_H_
