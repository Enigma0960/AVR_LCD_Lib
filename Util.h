#ifndef UTIL_H_
#define UTIL_H_

#include <avr/io.h>

#define LOW 0
#define HIGH 1

#define INPUT 0
#define OUTPUT 1

void pinMode(uint8_t pin, uint8_t mode);
void pinOut(uint8_t pin, uint8_t data);


#endif //UTIL_H_