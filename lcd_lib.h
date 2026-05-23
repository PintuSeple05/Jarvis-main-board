#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#define RS PB1
#define EN PB0
#define LED PD1
//extern void delay(unsigned int a);
void LCD_Enable()
{
    //EN = 1;
	PORTB |= (1 << EN);
    _delay_us(5);
    PORTB &= ~(1 << EN);
    _delay_us(100);
}

void LCD_Send4Bit(unsigned char data)
{
    PORTB &= 0x0F;

    if(data & 0x01) PORTB |= (1<<4);
    if(data & 0x02) PORTB |= (1<<5);
    if(data & 0x04) PORTB |= (1<<6);
    if(data & 0x08) PORTB |= (1<<7);

    LCD_Enable();
}

void lcd_cmd(char cmd)
{
    //RS = 0;
	PORTB &= ~(1 << RS);
    LCD_Send4Bit(cmd >> 4);
    LCD_Send4Bit(cmd & 0x0F);
    _delay_ms(2);
}

void lcd_data(unsigned char data)
{
    //RS = 1;
	PORTB |= (1 << RS);
    LCD_Send4Bit(data >> 4);
    LCD_Send4Bit(data & 0x0F);
    _delay_ms(2);
}

// ================= REQUIRED FUNCTIONS =================

// INIT
void lcd_init()
{
    DDRB |= (1<<0)|(1<<1)|(1<<4)|(1<<5)|(1<<6)|(1<<7);
    DDRD |= (1<<1);

    _delay_ms(20);

    //RS = 0;
    PORTB &= ~(1 << RS);
    LCD_Send4Bit(0x03);
    _delay_ms(5);

    LCD_Send4Bit(0x03);
    _delay_us(200);

    LCD_Send4Bit(0x03);
    _delay_us(200);

    LCD_Send4Bit(0x02);

    lcd_cmd(0x28);
    lcd_cmd(0x0C);
    lcd_cmd(0x06);
    lcd_cmd(0x01);
}

// CLEAR
void lcd_clear()
{
    lcd_cmd(0x01);
}

// CURSOR
void LcdSetCursor(unsigned char row, unsigned char col)
{
    if(row == 0)
        lcd_cmd(0x80 + col);
    else
        lcd_cmd(0xC0 + col);
}

// PRINT CHAR
void LcdPrintChar(char data)
{
    lcd_data(data);
}

// PRINT STRING
void lcd_string(const char *str)
{    int i=0;
    while(*str)
    {
        lcd_data(*str++);
    }
    while(i < 16)   // remaining clear
    {
        lcd_data(' ');
        i++;
    }
}
void lcd_string_f(const char str[])
{
    unsigned char i = 0;

    // print string
    while(str[i] != '\0' && i < 16)
    {
        lcd_data(str[i]);
        i++;
    }

    // clear remaining part
    while(i < 16)
    {
        lcd_data(' ');
        i++;
    }
}

// PRINT NUMBER
void lcd_num(unsigned char num)
{
//    char buf[16];
//    ltoa(num, buf);
//    //sprintf(buf, "%ld", num);
//    lcd_string(buf);
    lcd_data(num/10+48);
	lcd_data(num%10+48);
}

// BACKLIGHT CONTROL (simple ON/OFF ya pseudo analog)
void LcdLightControl(unsigned char val)
{
    if(val > 0)
        //LED = 1;
		PORTD |= (1 << LED);
    else
        PORTD &= ~(1 << LED);
}
void lcd_clear_line()
{
	unsigned char i;
	for(i=0;i<16;i++)
	{
		lcd_data(' ');
	}
}
void lcd_build(unsigned char location, const unsigned char *ptr)
{
	unsigned char i;
	if(location<8)
	{
		lcd_cmd(0x40+(location*8));
		for(i=0;i<8;i++)
		{
			lcd_data(pgm_read_byte(&ptr[i]));
		}
	}
}