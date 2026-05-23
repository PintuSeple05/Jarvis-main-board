#ifndef _RTC_LIB_H 
#define _RTC_LIB_H

#include <avr/io.h>
#include <util/delay.h>

/* Forward declarations for RTC and EEPROM functions to break circular dependency with serial_lib.h */
unsigned char RTC_GetHour(void);
unsigned char RTC_GetMinute(void);
unsigned char RTC_GetSecond(void);
unsigned char RTC_GetDay(void);
unsigned char RTC_GetMonth(void);
unsigned char RTC_GetYear(void);

void RTC_SetSecond(unsigned char a);
void RTC_SetMinute(unsigned char a);
void RTC_SetHour(unsigned char a);
void RTC_SetDay(unsigned char a);
void RTC_SetMonth(unsigned char a);
void RTC_SetYear(unsigned char a);

unsigned char Eeprom_ReadByte(unsigned int Addr);
void Eeprom_WriteByte(unsigned int Addr, unsigned char Data);

#include "serial_lib.h"

/* =====================================================================================
   I2C PIN DEFINITIONS (PC5 = SDA, PC6 = SCL)
   Open-drain via DDR control (CodeVision correct method)
   ===================================================================================== */

/*#define SDA_DDR     DDRC.5
#define SDA_PORT    PORTC.5
#define SDA_PIN     PINC.5

#define SCL_DDR     DDRC.6
#define SCL_PORT    PORTC.6*/
#define SDA_PIN     PC5
#define SCL_PIN     PC6

#define ACK         1
#define NO_ACK      0

/* =====================================================================================
   KEYPAD MATRIX (PCF8574) - NUMERIC KEYPAD
   ===================================================================================== */

const unsigned char keypad_matrix[4][4] =
{
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

/* =====================================================================================
   HEX KEYPAD MATRIX (PCF8574) - HEXADECIMAL KEYPAD (0-F)
   Layout:
   0 1 2 3
   4 5 6 7
   8 9 A B
   C D E F
   ===================================================================================== */

const unsigned char hexpad_matrix[4][4] =
{
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}
};

/* =====================================================================================
   SMALL DELAY
   ===================================================================================== */

void _nop_(void)
{
    unsigned char i;
    for(i=0;i<100;i++);
}

/* =====================================================================================
   BCD <-> HEX CONVERSION
   ===================================================================================== */

unsigned char BCD2HEX(unsigned char bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

unsigned char HEX2BCD(unsigned char hex)
{
    unsigned char bcd = 0;
    if(hex <= 99)
    {
        bcd = (hex / 10) << 4;
        bcd |= (hex % 10);
    }
    return bcd;
}

/* =====================================================================================
   I2C LOW LEVEL (OPEN DRAIN)
   ===================================================================================== */

static void SDA_LOW(void)
{
    //SDA_DDR = 1;
    //SDA_PORT = 0;
	 DDRC |= (1 << SDA_PIN);      // output
	 PORTC &= ~(1 << SDA_PIN);    // LOW
}

static void SDA_RELEASE(void)
{
    DDRC &= ~(1 << SDA_PIN);     // input
    PORTC |= (1 << SDA_PIN);
}

static void SCL_LOW(void)
{
    DDRC |= (1 << SCL_PIN);      // output
    PORTC &= ~(1 << SCL_PIN);    // LOW
}

static void SCL_RELEASE(void)
{
    DDRC &= ~(1 << SCL_PIN);     // input
    PORTC |= (1 << SCL_PIN);     // pull-up ON
}

/* =====================================================================================
   I2C START / STOP CONDITIONS
   ===================================================================================== */

void I2C_Start(void)
{
    SDA_RELEASE();
    SCL_RELEASE();
    _nop_(); _nop_();

    SDA_LOW();
    _nop_(); _nop_();

    SCL_LOW();
}

void I2C_Stop(void)
{
    SDA_LOW();
    _nop_(); _nop_();

    SCL_RELEASE();
    _nop_(); _nop_();

    SDA_RELEASE();
    _nop_(); _nop_();
}

/* =====================================================================================
   I2C WRITE BYTE
   ===================================================================================== */

void I2C_Write(unsigned char Data)
{
    unsigned char i;

    for(i=0;i<8;i++)
    {
        if(Data & 0x80)
            SDA_RELEASE();
        else
            SDA_LOW();

        SCL_RELEASE();
        _nop_(); _nop_();
        SCL_LOW();

        Data <<= 1;
    }

    SDA_RELEASE();
    SCL_RELEASE();
    _nop_(); _nop_();
    SCL_LOW();
}

/* =====================================================================================
   I2C READ BYTE
   ===================================================================================== */

unsigned char I2C_Read(unsigned char ACK_Bit)
{
    unsigned char i, Data = 0;

    SDA_RELEASE();

    for(i=0;i<8;i++)
    {
        Data <<= 1;

        SCL_RELEASE();
        _nop_(); _nop_();

        if(PINC & (1 << SDA_PIN))
            Data |= 1;

        SCL_LOW();
        _nop_();
    }

    if(ACK_Bit == ACK)
        SDA_LOW();
    else
        SDA_RELEASE();

    SCL_RELEASE();
    _nop_(); _nop_();
    SCL_LOW();

    SDA_RELEASE();
    return Data;
}

/* =====================================================================================
   RTC (DS1307) LOW LEVEL FUNCTIONS
   ===================================================================================== */

unsigned char RTC_ReadByte(unsigned char Addr)
{
    unsigned char Data;

    I2C_Start();
    I2C_Write(0xD0);
    I2C_Write(Addr);
    I2C_Start();
    I2C_Write(0xD1);
    Data = I2C_Read(NO_ACK);
    I2C_Stop();

    return Data;
}

void RTC_WriteByte(unsigned char Addr, unsigned char Data)
{
    I2C_Start();
    I2C_Write(0xD0);
    I2C_Write(Addr);
    I2C_Write(Data);
    I2C_Stop();
}

/* =====================================================================================
   PCF8574 I2C EXPANDER FUNCTIONS
   ===================================================================================== */

unsigned char PCF8574_ReadByte(void)
{
    unsigned char Data;

    I2C_Start();
    I2C_Write(0x71);
    Data = I2C_Read(NO_ACK);
    I2C_Stop();

    return Data;
}

void PCF8574_WriteByte(unsigned char Data)
{
    I2C_Start();
    I2C_Write(0x70);
    I2C_Write(Data);
    I2C_Stop();
    _delay_ms(10);
}


unsigned char PCF8574_ex_ReadByte(void)
{
    unsigned char Data;

    I2C_Start();
    I2C_Write(0x73);
    Data = I2C_Read(NO_ACK);
    I2C_Stop();

    return Data;
}

void PCF8574_ex_WriteByte(unsigned char Data)
{
    I2C_Start();
    I2C_Write(0x73); // 0x73 is read, 0x72 is write for PCF8574 at address 0x73 (A2=0, A1=0, A0=1)
    I2C_Write(Data);
    I2C_Stop();
    _delay_ms(10);
}

/* =====================================================================================
   COMPATIBILITY ALIASES FOR PCF8574 (REQUIRED BY LEGACY CODE)
   ===================================================================================== */
//
//#define PCF8574_ex_ReadByte()    PCF8574_ReadByte()
//#define PCF8574_ex_WriteByte(x)  PCF8574_WriteByte(x)

/* =====================================================================================
   EXTERNAL EEPROM (24Cxx) FUNCTIONS
   ===================================================================================== */

unsigned char Eeprom_ReadByte(unsigned int Addr)
{
    unsigned char Data;

    I2C_Start();
    I2C_Write(0xA0);
    I2C_Write((unsigned char)(Addr >> 8));
    I2C_Write((unsigned char)Addr);
    I2C_Start();
    I2C_Write(0xA1);
    Data = I2C_Read(NO_ACK);
    I2C_Stop();

    return Data;
}

void Eeprom_WriteByte(unsigned int Addr, unsigned char Data)
{
    I2C_Start();
    I2C_Write(0xA0);
    I2C_Write((unsigned char)(Addr >> 8));
    I2C_Write((unsigned char)Addr);
    I2C_Write(Data);
    I2C_Stop();
    delay(10);
}

/* =====================================================================================
   RTC HIGH LEVEL FUNCTIONS - GET TIME/DATE
   ===================================================================================== */

unsigned char RTC_GetHour(void)   { return BCD2HEX(RTC_ReadByte(0x02)); }
unsigned char RTC_GetMinute(void) { return BCD2HEX(RTC_ReadByte(0x01)); }
unsigned char RTC_GetSecond(void) { return BCD2HEX(RTC_ReadByte(0x00)); }
unsigned char RTC_GetDay(void)    { return BCD2HEX(RTC_ReadByte(0x04)); }
unsigned char RTC_GetMonth(void)  { return BCD2HEX(RTC_ReadByte(0x05)); }
unsigned char RTC_GetYear(void)   { return BCD2HEX(RTC_ReadByte(0x06)); }

/* =====================================================================================
   RTC HIGH LEVEL FUNCTIONS - SET TIME/DATE
   ===================================================================================== */

void RTC_SetSecond(unsigned char a){ RTC_WriteByte(0x00, HEX2BCD(a)); }
void RTC_SetMinute(unsigned char a){ RTC_WriteByte(0x01, HEX2BCD(a)); }
void RTC_SetHour(unsigned char a)  { RTC_WriteByte(0x02, HEX2BCD(a)); }
void RTC_SetDay(unsigned char a)   { RTC_WriteByte(0x04, HEX2BCD(a)); }
void RTC_SetMonth(unsigned char a) { RTC_WriteByte(0x05, HEX2BCD(a)); }
void RTC_SetYear(unsigned char a)  { RTC_WriteByte(0x06, HEX2BCD(a)); }

/* =====================================================================================
   NUMERIC KEYPAD FUNCTIONS (PCF8574 WITH STANDARD 4x4 MATRIX)
   ===================================================================================== */

void Keypad_Init(void)
{
    PCF8574_WriteByte(0xFF);
}

unsigned char Keypad_IsPressed(void)
{
    PCF8574_WriteByte(0x0F);
    if((PCF8574_ReadByte() & 0xF0) != 0xF0)
        return 1;
    return 0;
}

unsigned char Keypad_GetKey(void)
{
    unsigned char row, col, data;

    for(row=0; row<4; row++)
    {
        data = 0xFF;
        data &= ~(1 << row);
        PCF8574_WriteByte(data);

        _nop_(); _nop_();

        col = PCF8574_ReadByte() & 0xF0;

        if(col != 0xF0)
        {
            if(!(col & 0x10)) return keypad_matrix[row][0];
            if(!(col & 0x20)) return keypad_matrix[row][1];
            if(!(col & 0x40)) return keypad_matrix[row][2];
            if(!(col & 0x80)) return keypad_matrix[row][3];
        }
    }
    return 0xFF;
}

unsigned char Keypad_GetKey_NonBlocking(void)
{
    if(Keypad_IsPressed())
    {
        _delay_ms(20);
        return Keypad_GetKey();
    }
    return 0xFF;
}

/* =====================================================================================
   HEX KEYPAD FUNCTIONS (PCF8574 WITH HEXADECIMAL 4x4 MATRIX 0-F)
   ===================================================================================== */

unsigned char Hexpad_IsPressed(void)
{
    PCF8574_WriteByte(0x0F);
    if((PCF8574_ReadByte() & 0xF0) != 0xF0)
        return 1;
    return 0;
}

unsigned char Hexpad_GetKey(void)
{
    unsigned char row, col, data;

    for(row = 0; row < 4; row++)
    {
        data = 0xFF;
        data &= ~(1 << row);
        PCF8574_WriteByte(data);

        _nop_(); _nop_();

        col = PCF8574_ReadByte() & 0xF0;

        if(col != 0xF0)
        {
            if(!(col & 0x10)) return hexpad_matrix[row][0];
            if(!(col & 0x20)) return hexpad_matrix[row][1];
            if(!(col & 0x40)) return hexpad_matrix[row][2];
            if(!(col & 0x80)) return hexpad_matrix[row][3];
        }
    }
    return 0xFF;
}

unsigned char Hexpad_GetKey_NonBlocking(void)
{
    if(Hexpad_IsPressed())
    {
        _delay_ms(20);
        return Hexpad_GetKey();
    }
    return 0xFF;
}

/* =====================================================================================
   HEX CHARACTER TO VALUE CONVERSION (0x00 - 0x0F)
   Returns 0xFF if invalid character
   ===================================================================================== */

unsigned char Hexpad_CharToValue(unsigned char ch)
{
    if(ch >= '0' && ch <= '9')
        return (ch - '0');
    else if(ch >= 'A' && ch <= 'F')
        return (ch - 'A' + 10);
    else if(ch >= 'a' && ch <= 'f')
        return (ch - 'a' + 10);
    return 0xFF;
}

/* =====================================================================================
   VALUE TO HEX CHARACTER CONVERSION (0x00 - 0x0F)
   Returns 0xFF if invalid value
   ===================================================================================== */

unsigned char Hexpad_ValueToChar(unsigned char value)
{
    if(value <= 9)
        return ('0' + value);
    else if(value <= 0x0F)
        return ('A' + (value - 10));
    return 0xFF;
}

/* =====================================================================================
   TEST FUNCTIONS
   ===================================================================================== */

void TestKeypad(void)
{
    lcd_clear();
    lcd_cmd(0x80);
    lcd_string("Keypad Test:");
    
    while(1) {
        unsigned char rawKey = Keypad_GetKey();
        if(rawKey != 0xFF) {
            lcd_cmd(0xC0);
            lcd_data(rawKey);
            serial_putc(rawKey);
            _delay_ms(200);
        }
    }
}

/* =====================================================================================
   HEX KEYPAD TEST FUNCTION
   Displays hex key pressed and its numeric value on LCD
   ===================================================================================== */

void TestHexpad(void)
{
    unsigned char hexKey;
    unsigned char hexValue;
    
    lcd_clear();
    lcd_cmd(0x80);
    lcd_string_f("Hex Keypad Test");
    _delay_ms(500);
    
    while(1) 
    {
        hexKey = Hexpad_GetKey_NonBlocking();
        if(hexKey != 0xFF)
        {
            hexValue = Hexpad_CharToValue(hexKey);
            
            lcd_cmd(0xC0);
            lcd_string_f("Key:");
            lcd_data(hexKey);
            lcd_string_f(" Val:");
            
            if(hexValue < 10)
                lcd_data('0' + hexValue);
            else
                lcd_data('A' + (hexValue - 10));
            
            serial_putc(hexKey);
            _delay_ms(300);
        }
    }
}

#endif 