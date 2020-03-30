#include "mbed.h"
#define sample 1024
#define ON false
#define OFF true

Serial pc( USBTX, USBRX );
InterruptIn btn(SW3);
DigitalOut redLED(LED_RED);
DigitalOut greenLED(LED_GREEN);
AnalogIn Ain(A0);
AnalogOut Aout(DAC0_OUT);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
Thread thread1;
Thread thread2;
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

bool at_up = false;
int frequency = 0;
float avg = 0;
float ADCdata[sample];
float ADCdata_tmp[sample];

void red() {
	redLED = ON;
	greenLED = OFF;
}

void green() {
	redLED = OFF;
	greenLED = ON;
}

int main() {
	redLED = OFF;
	greenLED = ON;
	btn.fall(&red);
	btn.rise(&green);

	while (true) {
		if (btn) {
			for (int i = 0; i < sample; i++) {
				if (!btn) break;
				Aout = Ain;
				ADCdata_tmp[i] = Ain;
				wait(1.0/sample);
			}
			for (int i = 0; i < sample; i++) {
				ADCdata[i] = ADCdata_tmp[i];
				avg += ADCdata_tmp[i];
			}
			avg /= sample;
			frequency = 0;
			for (int i = 0; i < sample; i++) {
				if (!at_up && ADCdata[i] > avg) {
					frequency++;
					at_up = true;
				}
				if (ADCdata[i] < avg / 5) {
					at_up = false;
				}
			}
		} else {
			display = table[frequency / 100 % 10];
			wait(1);
			display = table[frequency / 10 % 10];
			wait(1);
			display = table[frequency / 1 % 10] | 0x80;
			wait(1);
			display = 0x00;
			for (int i = 0; i < sample; i++) {
				pc.printf("%1.3f\r\n", ADCdata[i]);
			}
		}
	}
}

/*
	int frequency = 0;
	int tmp = 0;
	float avg = 0;
	float ADCdata_tmp[sample] = {0};
	float ADCdata[sample] = {0};
	
	while (1) {
		if (Switch) {
			greenLED = ON;
			redLED = OFF;
			display = 0x00;
			for (int i = 0; i < sample; i++) {
				if (!Switch) break;
				Aout = Ain;
				ADCdata_tmp[i] = Ain;
				wait(1.0/sample);
			}
			avg = 0;
			for (int i = 0; i < sample; i++) {
				ADCdata[i] = ADCdata_tmp[i];
				avg += ADCdata_tmp[i];
			}
			avg /= sample;
			frequency = 0;
			for (int i = 0; i < sample - 1; i++) {
				if (ADCdata[i] < avg && ADCdata[i + 1] > avg) {
					frequency++;
				}
			}
		} else {
			greenLED = OFF;
			redLED = ON;
			display = table[frequency / 100 % 10];
			wait(1);
			tmp %= 100;
			display = table[frequency / 10 % 10];
			wait(1);
			tmp %= 10;
			display = table[frequency / 1 % 10] | 0x80;
			wait(1);
			for (int i = 0; i < sample; i++) {
				pc.printf("%1.3f\r\n", ADCdata[i]);
			}
		}
	}
*/