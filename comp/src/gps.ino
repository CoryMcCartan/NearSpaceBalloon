/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2016.  All Rights Reserved.
 */

#include "gps.h"
#include <SoftwareSerial.h>

#define ENABLE_GPS 12

// parsing cases
#define LAT 1
#define LONG 3
#define SAT 6
#define ALT 8

SoftwareSerial gps_serial(4, 5); // RX, TX

byte index = 0;
byte buffer[82];

bool setupGPS() {
    pinMode(ENABLE_GPS, OUTPUT);

    gps_serial.begin(9600);
    delay(100);

    digitalWrite(ENABLE_GPS, HIGH);
    delay(400);

    return setDynamicModel(LOW_ALTITUDE); 
}

void getGPSData(ProbeInfo * info) {
    bool newData = false;

    while (gps_serial.available()) {
        char c = gps_serial.read();

        if ((c == '$') || (index >= 80)) // start of new line
            index = 0;

        if (c != '\r')  // store every character in buffer
            buffer[index++] = c;

        if (c == '\n') {
            // $GNGGA is a info string
            if ((buffer[1] == 'G') && (buffer[3] == 'G') && (buffer[4] == 'G') && (buffer[5] == 'A'))
                processGNGGA(info);
            // $GNVTG is a velocity string
            if ((buffer[1] == 'G') && (buffer[3] == 'V') && (buffer[4] == 'T') && (buffer[5] == 'G'))
                processGNVTG(info);

            index = 0;
        }
    }
}

/**
 * Parse NMEA GNGGA string for coordinates and # of satellites in view
 */
void processGNGGA(ProbeInfo * info) {
    bool intPart = true;
    float altitude = 0;
    float lat_min = 0;
    float lon_min = 0;
    byte lat_deg = 0;
    byte lon_deg = 0;
    byte satellites = 0;

    // iterate over string (starting at char 7 after $GNGGA) char-by-char
    byte i, j, k;
    for (i = 7, j = 0, k = 0; i < index && j < 9; i++) {
        char c = buffer[i];
        // new segment
        if (c == ',') {
            j++;
            k = 0;
            intPart = true;
            continue;
        }

        switch (j) {
            case LAT:
                if (k < 2)
                    parseInt(&lat_deg, c);
                else 
                    parseFloat(&lat_min, c, &intPart);
                break;
            case LONG:
                if (k < 3)
                    parseInt(&lon_deg, c);
                else 
                    parseFloat(&lon_min, c, &intPart);
                break;
            case SAT:
                parseInt(&satellites, c);
                break;
            case ALT:
                parseFloat(&altitude, c, &intPart);
                break;
        }

        k++;
    }

    info->satellites = satellites;
    info->altitude = altitude;
    info->latitude = ((float) lat_deg) + lat_min / 60.0;
    info->longitude = ((float) lon_deg) + lon_min / 60.0;
}


/**
 * Parse NMEA GNVTG string for ground speed of satellites
 */
void processGNVTG(ProbeInfo * info) {
    bool intPart = true;
    float speed = 0;

    // iterate over string (starting at char 7 after $GNVTG) char-by-char
    byte i, j;
    for (i = 7, j = 0; i < index && j < 9; i++) {
        char c = buffer[i];
        // new segment
        if (c == ',') {
            j++;
            intPart = true;
            continue;
        }

        if (j == 7) {
            parseFloat(&speed, c, &intPart);
        }
    }

    info->speed = speed / 3.6; // km/h -> m/s
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
void parseFloat(float * val, char c, bool * intPart) {
    if (c >= '0' && c <= '9' && *intPart) {
        *val *= 10;
        *val += (float) (c - '0');
    } else {
        *intPart = false;

        *val *= 10;
        *val += ((float) (c - '0'));
        *val /= 10;
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
        sendUBX(dynamic_model_command, 44); // 44 is length
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
    gps_serial.flush();
    gps_serial.write(0xFF);
    delay(500);
    for (byte i = 0; i < length; i++) {
        gps_serial.write(message[i]);
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
        if (gps_serial.available()) {
            b = gps_serial.read();

            // check that bytes arrive in order
            if (b == ackPacket[ackByteID]) {
                ackByteID++; // move on to next byte
            } else {
                ackByteID = 0; // reset and look again
            }
        }
    }
}
