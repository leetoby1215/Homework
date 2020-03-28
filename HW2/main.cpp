#include "mbed.h"
#define sample 1024
#define ON false
#define OFF true
DigitalIn Switch(SW3);
DigitalOut redLED(LED_RED);
DigitalOut greenLED(LED_GREEN);
AnalogIn Ain(A0);
AnalogOut Aout(DAC0_OUT);
BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

int main() {
	int frequency = 0;
	int tmp = 0;
	float ADCdata_tmp[sample];
	float ADCdata[sample];

	while (1) {
		if (!Switch) {
			greenLED = OFF;
			redLED = ON;
			tmp = frequency;
			display = table[tmp / 100];
			wait(1);
			tmp %= 100;
			display = table[tmp / 10];
			wait(1);
			tmp %= 10;
			display = table[tmp] | 0x80;
			wait(1);
		} else {
			greenLED = ON;
			redLED = OFF;
			display = 0x00;
			for (int i = 0; i < sample; i++) {
				if (!Switch) break;
				Aout = Ain;
				ADCdata_tmp[i] = Ain;
				wait(1.0/sample);
			}
			for (int i = 0; i < sample; i++) {
				ADCdata[i] = ADCdata_tmp[i];
			}
			frequency = 0;
			for (int i = 1; i < sample - 1; i++) {
				if (ADCdata[i] > ADCdata[i - 1] && ADCdata[i + 1] < ADCdata[i]) {
					frequency++;
				}
			}
		}
	}
}