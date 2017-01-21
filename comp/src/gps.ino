/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2017.  All Rights Reserved.
 */

#include "gps.h"

// pin to enable gps
#define ENABLE_GPS 12

// parsing cases
#define LAT 1
#define LONG 3
#define SAT 6
#define ALT 8

byte index = 0;
byte buffer[82];

bool setupGPS() {
    pinMode(ENABLE_GPS, OUTPUT);

    Serial.begin(9600); // start serial communication with GPS module

    digitalWrite(ENABLE_GPS, HIGH);
    delay(400);

    return setDynamicModel(LOW_ALTITUDE); 
}

void getGPSData(ProbeInfo * info) {
    bool newData = false;

    // read characters from serial stream one at a time, if available.
    while (Serial.available()) {
        char c = Serial.read();

        if ((c == '$') || (index >= 80)) // start of new line
            index = 0;

        if (c != '\r')  // store every character in buffer
            buffer[index++] = c;

        if (c == '\n') {
            // $GNVTG is a velocity string
            if ((buffer[1] == 'G') && (buffer[3] == 'V') && (buffer[4] == 'T') && (buffer[5] == 'G'))
                processGNVTG(info);
            // $GNRMC is combined pos/vel
            //if ((buffer[1] == 'G') && (buffer[3] == 'R') && (buffer[4] == 'M') && (buffer[5] == 'C'))
            //    processGNRMC(info);
            // $GNGGA is a info string
            if ((buffer[1] == 'G') && (buffer[3] == 'G') && (buffer[4] == 'G') && (buffer[5] == 'A'))
                processGNGGA(info);

            index = 0;
        }
    }
}

/**
 * Parse NMEA GNGGA string for coordinates and # of satellites in view
 */
void processGNGGA(ProbeInfo * info) {
    byte decPosition = 0;
    double altitude = 0;
    double lat_min = 0;
    double lon_min = 0;
    byte lat_deg = 0;
    byte lon_deg = 0;
    byte satellites = 0;

    // iterate over string (starting at char 7 after $GNGGA) char-by-char
    byte i, j, k;
    // i is position within overal string
    // j is position by comma group (each time we hit a comma, j++)
    // k is position within comma group
    for (i = 7, j = 0, k = 0; i < index && j < 10; i++) {
        char c = buffer[i];
        // new segment
        if (c == ',') {
            j++;
            k = 0;
            decPosition = 0;
            continue;
        }

        // depending on position in string, we parse different variables
        switch (j) {
            case LAT:
                if (k < 2) // first 2 characters of latitude are in degrees
                    parseInt(&lat_deg, c);
                else  // rest of characters are in minutes
                    parseFloat(&lat_min, c, &decPosition);
                break;
            case LONG:
                if (k < 3) // first 3 characters of longitude are in degrees
                    parseInt(&lon_deg, c);
                else  // rest of characters are in minutes
                    parseFloat(&lon_min, c, &decPosition);
                break;
            case SAT:
                parseInt(&satellites, c);
                break;
            case ALT:
                parseFloat(&altitude, c, &decPosition);
                break;
        }

        k++; 
    }

    info->satellites = satellites;
    info->altitude = altitude;
    info->latitude = ((double) lat_deg) + lat_min / 60.0;
    info->longitude = ((double) lon_deg) + lon_min / 60.0;
}


/**
 * Parse NMEA GNVTG string for ground speed of satellites
 */
void processGNVTG(ProbeInfo * info) {
    byte decPosition = 0;
    double speed = 0;

    // iterate over string (starting at char 7 after $GNVTG) char-by-char
    byte i, j;
    // i is position within overal string
    // j is position by comma group
    for (i = 7, j = 0; i < index && j < 9; i++) {
        char c = buffer[i];
        // new segment
        if (c == ',') {
            j++;
            decPosition = 0;
            continue;
        }

        if (j == 7) {
            parseFloat(&speed, c, &decPosition);
        }
    }

    info->speed = speed / 3.6; // km/h -> m/s
}

/**
 * Parse NMEA GNRMC string for position and speed 
 */
void processGNRMC(ProbeInfo * info) {
    byte decPosition = 0;
    double speed = 0;
    double lat_min = 0;
    double lon_min = 0;
    byte lat_deg = 0;
    byte lon_deg = 0;

    // iterate over string (starting at char 7 after $GNVTG) char-by-char
    byte i, j, k;
    // i is position within overal string
    // j is position by comma group (each time we hit a comma, j++)
    // k is position within comma group
    for (i = 7, j = 0, k = 0; i < index && j < 10; i++) {
        char c = buffer[i];
        // new segment
        if (c == ',') {
            j++;
            k = 0;
            decPosition = 0;
            continue;
        }

        switch (j) {
            case 2: // LAT
                if (k < 2)
                    parseInt(&lat_deg, c);
                else 
                    parseFloat(&lat_min, c, &decPosition);
                break;
            case 4: // LONG
                if (k < 3)
                    parseInt(&lon_deg, c);
                else 
                    parseFloat(&lon_min, c, &decPosition);
                break;
            case 6: // SPEED
                parseFloat(&speed, c, &decPosition);
                break;
        }
        
        k++;
    }

    info->speed = speed * 0.51444; // knots -> m/s
    info->latitude = ((double) lat_deg) + lat_min / 60.0;
    info->longitude = ((double) lon_deg) + lon_min / 60.0;
}


/**
 * Parse a character into an existing integer.
 */
void parseInt(byte * val, char c) {
    if (c >= '0' && c <= '9') {
        *val *= 10;
        *val += (byte) (c - '0');
    }
}

/**
 * Parse a character into an existing floating-point number.
 */
void parseFloat(double * val, char c, byte * decPosition) {
    if (c >= '0' && c <= '9' && *decPosition == 0) {
        *val *= 10;
        *val += (double) (c - '0');
    } else if (c == '.') {
        *decPosition = 1;
    } else {
        double digit = ((double) (c - '0'));
        double power = (double) pow(10.0, (float) *decPosition);

        *val *= power;
        *val += digit;
        *val /= power;

        (*decPosition)++;
    }
}

/**
 * Sets GPS dynamic model.
 */
bool setDynamicModel(byte mode) {
    byte dynamic_model_command[] = { // default high altitude
        0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, mode,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
        0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };

    // calculate checksum
    for (byte j = 2; j < 42; j++) {
        dynamic_model_command[42] += dynamic_model_command[j];
        dynamic_model_command[43] += dynamic_model_command[42];
    }

    for (byte i = 0; i < 5; i++) { // try at most five times
        sendUBX(dynamic_model_command, 44); // 44 is length of data
        if (getUBX_ACK(dynamic_model_command)) {
            return true;
        }
    }

    return false; // didn't work
}


/**
 * Send a command/message to the GPS unit.
 */
void sendUBX(byte* message, byte length) {
    Serial.flush();
    Serial.write(0xFF);
    delay(500);
    for (byte i = 0; i < length; i++) {
        Serial.write(message[i]);
    }
}

/**
 * Check if message received properly.
 */
bool getUBX_ACK(byte* message) {
    byte b;
    byte ackByteID = 0;
    byte ackPacket[10]; // the packet we expect

    unsigned long start_time = millis();

    // construct the packet
    ackPacket[0] = 0xB5; // header
    ackPacket[1] = 0x62; // header
    ackPacket[2] = 0x05; // class
    ackPacket[3] = 0x01; // id
    ackPacket[4] = 0x02; // length
    ackPacket[5] = 0x00; 
    ackPacket[6] = message[2]; // ACK class
    ackPacket[7] = message[3]; // ACK ID
    ackPacket[8] = 0x00; // checksum A
    ackPacket[9] = 0x00; // checksum B

    // calculate checksums
    for (byte ubxi = 2; ubxi < 8; ubxi++) {
        ackPacket[8] += ackPacket[ubxi];
        ackPacket[9] += ackPacket[8];
    }

    // wait for result
    while (true) {
        // test for success
        if (ackByteID > 9) return true;

        // 3s timeout
        if (millis() - start_time > 3000) return false;

        // ensure data is available for reading
        if (Serial.available()) {
            b = Serial.read();

            // check that bytes arrive in order
            if (b == ackPacket[ackByteID]) {
                ackByteID++; // move on to next byte
            } else {
                ackByteID = 0; // reset and look again
            }
        }
    }
}
