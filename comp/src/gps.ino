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
#include <TinyGPS.h>

#define ENABLE_GPS 2

TinyGPS gps;
SoftwareSerial gps_serial(4, 5); // RX, TX

bool setDynamicModel();
void sendUBX(byte* message, byte length);
bool getUBX_ACK(byte* message);

bool setupGPS() {
    pinMode(ENABLE_GPS, OUTPUT);

    Serial.begin(9600);
    gps_serial.begin(9600);
    delay(500);

    digitalWrite(ENABLE_GPS, HIGH);
    delay(500);

    bool success = true;//  setDynamicModel(); 

    Serial.println(success ? "Setup complete." : "Setup failed.");

    return success;
}

Location getGPSData() {
    while (gps_serial.available()) {
        byte b = gps_serial.read();

        if (!gps.encode(b)) continue; // wait for valid data

        Location position;
        unsigned long fix_age;

        gps.get_position(&position.latitude, &position.longitude, &fix_age);
        position.altitude = gps.altitude();

        return position;
    }
}

/**
 * Set the dynamic model the GPS should use, which is airborne.  Allows for 
 * operation above 12km, and full 3D positioning.
 */
bool setDynamicModel() {
    bool gps_success = false;

    byte dynamic_model_command[] = {
        0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06,
        0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00,
        0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC 
    };
    byte command_length = sizeof(dynamic_model_command) / sizeof(byte);

    for (int i = 0; i < 5; i++) { // try at most five times
        sendUBX(dynamic_model_command, command_length);
        if (getUBX_ACK(dynamic_model_command)) {
            return true;
        }
    }

    return false;
}

/**
 * Send a command/message to the GPS unit.
 */
void sendUBX(byte* message, byte length) {
    gps_serial.flush();
    gps_serial.write(0xFF);
    delay(500);
    for (int i = 0; i < length; i++) {
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
