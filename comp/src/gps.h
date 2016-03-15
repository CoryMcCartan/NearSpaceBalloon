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

typedef struct {
    long latitude;
    long longitude;
    long altitude;
} Location;

/**
 * Set up GPS sensor. Return
 * 
 * @return true if successful, false otherwise.
 */
bool setupGPS();

Location getGPSData();

#endif // GPS_H_
