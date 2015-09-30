/*
 * NEAR SPACE BALLOON PROJECT
 * COMMUNICATION CODE
 *
 * GRINNELL SPACE EXPLORATION AGENCY
 *
 * (c) 2015.  All Rights Reserved.
 */

#define RADIOPIN 9
#define BITRATE (1000 / 50) // n baud
#define BASE 0
#define RANGE 255

void setupComms() {
	pinMode(RADIOPIN, OUTPUT);
	setPWMFrequency(RADIOPIN, 1);
}

/*
 * Set PWM frequency as fast as possible.
 */
void setPWMFrequency(int pin, int divisor) {
	byte mode;

	if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch (divisor) {
			case 1:
				mode = 0x01;
				break;
			case 8:
				mode = 0x02;
				break;
			case 64:
				mode = 0x03;
				break;
			case 256:
				mode = 0x04;
				break;
			case 1024:
				mode = 0x05;
				break;
			default:
				return;
		}
		
		if (pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
		} else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	} else if (pin == 3 || pin == 11) {
		switch (divisor) {
			case 1:
				mode = 0x01;
				break;
			case 8:
				mode = 0x02;
				break;
			case 32:
				mode = 0x03;
				break;
			case 64:
				mode = 0x04;
				break;
			case 128:
				mode = 0x05;
				break;
			case 256:
				mode = 0x06;
				break;
			case 1024:
				mode = 0x07;
				break;
			default:
				return;
		}

		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}

void send(int data) {
	// isolate 16 bit int into 4-bit numbers
	byte a = data;
	byte b = data >> 4;
	byte c = data >> 8;
	byte d = data >> 12;

	// hadamard encode
	byte* a_enc = hadamard_enc(a);
	byte* b_enc = hadamard_enc(b);
	byte* c_enc = hadamard_enc(c);
	byte* d_enc = hadamard_enc(d);

	// send
	sendByte(a_enc[0]);
	sendByte(a_enc[1]);
	sendByte(b_enc[0]);
	sendByte(b_enc[1]);
	sendByte(c_enc[0]);
	sendByte(c_enc[1]);
	sendByte(d_enc[0]);
	sendByte(d_enc[1]);
}

void sendByte(byte d) {
	// send each bit
	analogWrite(RADIOPIN, BASE + RANGE*(d & 1)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 2)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 4)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 8)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 16)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 32)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 64)); 
	delay(BITRATE);
	analogWrite(RADIOPIN, BASE + RANGE*(d & 128)); 
	delay(BITRATE);
}

byte* hadamard_enc(byte msg) {
	byte* enc = new byte[2];

	msg &= 15; // mask upper bits

	for (byte ct = 0; ct < 16; ct++) {
		byte i = ct + 16;
		// multiply corresponding bits
		byte a = i & msg;
		// sum (mod 2)
		a ^= a >> 4;
		a ^= a >> 2;
		a ^= a >> 1;
		a &= 1;
		// store in enc array
		enc[ct / 8] += a << (7 - ct % 8);
	}

	return enc;
}

byte hadamard_dec(byte msg[]) {
	byte x = 0;
	
	for (byte i = 0; i < 4; i++) {
		byte j = random(16);
		byte k;
		for (k = 0; k < 16; k++) {
			if ((j ^ k) == 1 << i) break;
		}
		byte c = (msg[j/8] >> (j%8)) & 1;
		byte d = (msg[k/8] >> (k%8)) & 1;

		x += (c ^ d) << i;
	}

	return x;
}
