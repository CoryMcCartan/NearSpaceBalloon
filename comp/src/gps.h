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

#define LOW_ALTITUDE 0x03
#define HIGH_ALTITUDE 0x06

typedef struct {
    float latitude;   // decimal degrees
    float longitude;  // decimal degrees
    float altitude;   // meters
    byte satellites;  
    float speed;      // meters per second

} ProbeInfo;

/**
 * Set up GPS sensor. 
 * 
 * @return true if successful, false otherwise.
 */
bool setupGPS();

/**
 * Get current GPS coordinates and store them in given ProbeInfo struct.
 */
void getGPSData(ProbeInfo * info);

#endif // GPS_H_
