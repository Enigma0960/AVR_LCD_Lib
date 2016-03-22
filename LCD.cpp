#include "LCD.h"

//#define strlen(n) sizeof(n) / sizeof(const char)

LCD::LCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

LCD::LCD(uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
	init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

LCD::LCD(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) {
	init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void LCD::init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) {
	pinRS = rs;
	pinRW = rw;
	pinE = enable;
	
	pinData[0] = d0;
	pinData[1] = d1;
	pinData[2] = d2;
	pinData[3] = d3;
	pinData[4] = d4;
	pinData[5] = d5;
	pinData[6] = d6;
	pinData[7] = d7;
	
	if (fourbitmode)
		displayFunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	else
		displayFunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
}

void LCD::begin(uint8_t cols, uint8_t rows, uint8_t charsize) {
	if (rows > 1) {
		displayFunction |= LCD_2LINE;
	}
	numLines = rows;
	
	setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);
	
	if ((charsize != LCD_5x8DOTS) && (rows == 1)) {
		displayFunction |= LCD_5x10DOTS;
	}

	pinMode(pinRS, OUTPUT);
	pinMode(pinE, OUTPUT);	
	if (pinRW != 255)
		pinMode(pinRW, OUTPUT);
	
	for (int i=0; i<((displayFunction & LCD_8BITMODE) ? 8 : 4); ++i)
		pinMode(pinData[i], OUTPUT);
	
	_delay_us(50000);
	
	pinOut(pinRS, LOW);
	pinOut(pinE, LOW);
	if (pinRW != 255)
		pinOut(pinRW, LOW);
	
	if (! (displayFunction & LCD_8BITMODE)) {
		write4bits(0x03);
		_delay_us(4500);
		
		write4bits(0x03);
		_delay_us(4500);
		
		write4bits(0x03);
		_delay_us(150);
		
		write4bits(0x02);
		} else {
		
		command(LCD_FUNCTIONSET | displayFunction);
		_delay_us(4500);
		
		command(LCD_FUNCTIONSET | displayFunction);
		_delay_us(150);
		
		command(LCD_FUNCTIONSET | displayFunction);
	}

	command(LCD_FUNCTIONSET | displayFunction);

	displayControl = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();

	clear();
	
	displayMode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	command(LCD_ENTRYMODESET | displayMode);
}

void LCD::clear() {
	command(LCD_CLEARDISPLAY);
	_delay_us(2000);
}

void LCD::home() {
	command(LCD_RETURNHOME);
	_delay_us(2000);
}

void LCD::noDisplay() {
	displayControl &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::display() {
	displayControl |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::noBlink() {
	displayControl &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::blink() {
	displayControl |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::noCursor() {
	displayControl &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::cursor() {
	displayControl |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | displayControl);
}

void LCD::scrollDisplayLeft() {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCD::scrollDisplayRight() {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void LCD::leftToRight() {
	displayMode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | displayMode);
}

void LCD::rightToLeft() {
	displayMode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | displayMode);
}

void LCD::autoscroll() {
	displayMode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | displayMode);
}

void LCD::noAutoscroll() {
	displayMode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | displayMode);
}

void LCD::setRowOffsets(int row0, int row1, int row2, int row3) {
	rowOffsets[0] = row0;
	rowOffsets[1] = row1;
	rowOffsets[2] = row2;
	rowOffsets[3] = row3;
}

void LCD::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7;
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
	command(LCD_SETDDRAMADDR);
}

void LCD::setCursor(uint8_t cols, uint8_t rows) {
	const size_t max_lines = sizeof(rowOffsets) / sizeof(*rowOffsets);
	if ( rows >= max_lines )
		rows = max_lines - 1;
	if ( rows >= numLines )
		rows = numLines - 1;
	
	command(LCD_SETDDRAMADDR | (cols + rowOffsets[rows]));
}

size_t LCD::write(uint8_t value) {
	send(value, HIGH);
	return 1;
}

void LCD::command(uint8_t value) {
	send(value, LOW);
}

void LCD::send(uint8_t value, uint8_t mode) {
	pinOut(pinRS, mode);

	if (pinRW != 255)
		pinOut(pinRW, LOW);
	
	if (displayFunction & LCD_8BITMODE)
		write8bits(value);
	else {
		write4bits(value>>4);
		write4bits(value);
	}
}

void LCD::write4bits(uint8_t value) {
	for (int i = 0; i < 4; i++)
		pinOut(pinData[i], (value >> i) & 0x01);
	pulseEnable();
}

void LCD::write8bits(uint8_t value) {
	for (int i = 0; i < 8; i++)
		pinOut(pinData[i], (value >> i) & 0x01);
	pulseEnable();
}

void LCD::pulseEnable() {
	pinOut(pinE, LOW);
	_delay_us(1);
	pinOut(pinE, HIGH);
	_delay_us(1);
	pinOut(pinE, LOW);
	_delay_us(100);
}

/*-------------------------------------------*/

size_t LCD::write(const char* str) {
	if (str == NULL) return 0;
	return write((const uint8_t *)str, strlen(str));
}

size_t LCD::write(const char* buffer, size_t size) {
	return write((const uint8_t *)buffer, size);
}

size_t LCD::write(const uint8_t* buffer, size_t size) {
	size_t n = 0;
	while (size--)
		if (write(*buffer++)) n++;
		else break;
	return n;
}

size_t LCD::print(const char str[]) {
	return write(str);
}

size_t LCD::print(char c) {
	return write(c);
}

size_t LCD::print(unsigned char b, int base) {
	return print((unsigned long) b, base);
}

size_t LCD::print(int n, int base) {
	return print((long) n, base);
}

size_t LCD::print(unsigned int n, int base) {
	return print((unsigned long) n, base);
}

size_t LCD::print(long n, int base) {
	if (base == 0) {
		return write(n);
	} else if (base == 10) {
		if (n < 0) {
			int t = print('-');
			n = -n;
			return printNumber(n, 10) + t;
		}
		return printNumber(n, 10);
	} else return printNumber(n, base);
}

size_t LCD::print(unsigned long n, int base) {
	if (base == 0) return write(n);
	else return printNumber(n, base);
}

size_t LCD::print(double n, int digits) {
	return printFloat(n, digits);
}

size_t LCD::printNumber(unsigned long number, uint8_t base) {
	char buf[8 * sizeof(long) + 1];
	char* str = &buf[sizeof(buf) - 1];

	*str = '\0';

	if (base < 2) base = 10;

	do {
		unsigned long m = number;
		number /= base;
		char c = m - base * number;
		*--str = c < 10 ? c + '0' : c + 'A' - 10;
	} while(number);

	return write(str);
}

size_t LCD::printFloat(double number, uint8_t digits) {
	size_t n = 0;
	
	if (isnan(number)) return print("nan");
	if (isinf(number)) return print("inf");
	if (number > 4294967040.0) return print ("ovf");
	if (number <-4294967040.0) return print ("ovf");
	
	if (number < 0.0) {
		n += print('-');
		number = -number;
	}

	double rounding = 0.5;
	for (uint8_t i=0; i<digits; i++)
	rounding /= 10.0;
	
	number += rounding;

	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	n += print(int_part);

	if (digits > 0) n += print(".");

	while (digits-- > 0) {
		remainder *= 10.0;
		int toPrint = int(remainder);
		n += print(toPrint);
		remainder -= toPrint;
	}
	
	return n;
}