
#ifndef _REMOTE_KEYPAD_H_
#define F_CPU 8000000UL
#include <stdbool.h>
#include <util/delay.h>
#define _REMOTE_KEYPAD_H_

extern unsigned char GetRPTREnabled();
extern void Repeater_Setting();
//extern Eeprom_ReadByte();
extern void lcd_clear();
extern void lcd_cmd (char cmd);
//extern void _delay_ms(unsigned int n);
extern unsigned char GetKey();
extern void Eeprom_WriteByte(unsigned int Addr,unsigned char Data);
extern void lcd_string_f(const char str[]);
extern void Reset();
extern unsigned char Eeprom_ReadByte(unsigned int Addr);

#endif
