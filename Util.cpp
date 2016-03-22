#include "Util.h"

#define BITHIGH(N) (1 << N)
#define BITLOW(N) ~(1 << N)
#define set(out, pin, value) ((value) ? (out |= BITHIGH(pin)) : (out &= BITLOW(pin)))

void pinMode(uint8_t pin, uint8_t mode) {
	if(pin < 8) {
		set(DDRD, pin, mode);
	} else {
		pin -= 8;
		set(DDRB, pin, mode);
	}
	return;
}


void pinOut(uint8_t pin, uint8_t data) {
	if(pin < 8) {
		set(PORTD, pin, data);
	} else {
		pin -= 8;
		set(PORTB, pin, data);
	}
	return;
}