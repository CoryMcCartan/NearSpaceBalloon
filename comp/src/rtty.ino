/*
 * NEAR SPACE BALLOON PROJECT
 * FLIGHT COMPUTER CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * © 2017.  All Rights Reserved.
 */ 

#include "radio_config.h"
#include "radio.h"
#include "pin.h"

// special checksum constant
#define CRC16 0x1021 

char datastring[80];

// Private function definitions
void sendByte(byte b);
void sendBit(byte bit);
short calculateChecksum(char * str);

void send_rtty_string(char * str) {
    // a few padding 0s
    sendByte(0);
    sendByte(0);
    sendByte(0);
    
    for (char * c = str; *c != '\0'; c++) 
        sendByte(*c);
}

void sendByte(byte b) {
    sendBit(0); // start bit

    for (byte i = 0; i < 7; i++) {
        if (b & 1)
            sendBit(1);
        else
            sendBit(0);

        b = b >> 1; // line up next bit

    }

    sendBit(1); // stop bit
    sendBit(1); // stop bit
}

void sendBit(byte bit) {
    // corresponds to 200 carrier shift
    analogWrite(AUDIO_PIN, bit ? 110 : 100);
    // two-step delay for more accuracy
    delayMicroseconds(9091); // 110 baud
    //delayMicroseconds(10000);
}



short calculateChecksum(char * str) {
    byte i;
    byte count = 80;
    short crc = 0;

    do {
        crc = crc ^ (int) *str++ << 8;
        i = 8;

        do {
            if (crc & 0x8000)
                crc = crc << 1 ^ 0x1021;
            else
                crc = crc << 1;

        } while (--i);

    } while (--count);

    return crc;
}
