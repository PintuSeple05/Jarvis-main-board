#ifndef _SERIAL_LIB_H_
#define _SERIAL_LIB_H_

#include <avr/io.h>
#include <util/delay.h>
#include "globals.h"
#include "remote_keypad.h"
#include "main.h"
#define f_cpu 8000000UL  // 8 MHz clock frequency

// UART Error Flags
#define FRAMING_ERROR    (1<<FE1)
#define PARITY_ERROR     (1<<UPE1)
#define DATA_OVERRUN     (1<<DOR1)
#define BAUD_RATE        9600UL
#define RS485_PIN    PB3
#define RS485_EN()  (PORTB|=(1<<RS485_PIN)) 
#define RS485_DN()  (PORTB&=~(1<<RS485_PIN)) 

// Buffer definitions
#define SERIAL_BUFFER_SIZE 512
#define SERIAL1_BUFFER_SIZE 512
//===============================================by pintu============================================
#define BUFFER_SIZE 100
 volatile char rcv_buffer[BUFFER_SIZE]; // Buffer for received data
 char dest[20]="\0";  
 int rcv_index = 0;// Buffer index      
 int data_ready=0;
//===================================================================================================
// Buffer variables for UART0
volatile char serial_buff[SERIAL_BUFFER_SIZE];
volatile unsigned char ch_start = 0;
volatile unsigned char ch_end = 0;
volatile unsigned char num_start = 0;
volatile unsigned char num_end = 0;
volatile unsigned char b_direct_buffer = 0;
volatile unsigned int buffer_len = 0;
volatile unsigned int buff_index = 0;

// Buffer variables for UART1
volatile char serial1_buff[SERIAL1_BUFFER_SIZE];
volatile unsigned int buffer1_len = 0;
volatile unsigned int buff1_index = 0;
//volatile bit reset_request = 0;  // flag for main loop

// Function declarations
void EnableRemoteInt(unsigned char);
void serial_init(void);
void serial1_init(void);
void serial_putc(char c);
void serial1_putc(char c);
void serial_string(char *str);
void serial1_string(char *str);
void serial_string_f(const char *str);
void serial1_string_f(const char *str);
void serial_println(char *str);
void serial_println_f(const char *str);
void serial1_println_f(const char *str);
void serial_clear_buffer(void);
void serial1_clear_buffer(void);
unsigned char serial_available(void);
unsigned char serial1_available(void);
char serial_getc(void);
char serial1_getc(void);
unsigned char GetCMSEnabled(void);
void Sounder_Silent();
void Reset();
void int_hoot(unsigned char);
void hooter_trig(unsigned char);
void delay(unsigned int );
extern unsigned char remotezonestatus[];
extern bool trigger_flag;
extern bool sounder_flag;
extern unsigned char l_armed;
extern unsigned char l_disarmed;

  // Function implementations
unsigned char GetCMSEnabled(void) {    
    return Eeprom_ReadByte(ADD_CMS_EN);
}

void serial_init(void) {
    // Calculate baud rate value for 9600 baud
    unsigned int ubrr = (f_cpu / (16UL * BAUD_RATE)) - 1;
    
    // First disable the transmitter and receiver to be able to set baud rate
    UCSR0B = 0;
    
    // Set baud rate registers
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr; 
    
     // Enable transmitter and receiver
    //UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit, no parity
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
    
    
    // Enable transmitter and receiver
    UCSR0B = (1 << TXEN0) | (1 << RXEN0);
    
    // Configure RS485 direction control pin
    DDRB |= (1 << 3);     // Set PB3 (RS485_EN) as output
    PORTB &= ~(1 << 3);   // Initially set RS485 to receive mode
    
}

void serial_putc(char c) {
    if (!GetRPTREnabled()) return;  // Check if transmission is enabled
    
    // Enable RS485 transmitter
    PORTB |= (1 << 3);
    
    // Small delay for driver enable to stabilize
    delay(1);
    
    // Wait if a previous transmission is still in progress
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Clear transmit complete flag by writing 1 to it
    UCSR0A |= (1 << TXC0);
    
    // Send the data
    UDR0 = c;
    
    // Wait for transmission to complete
    while (!(UCSR0A & (1 << TXC0)));
    
    // Small delay before disabling driver
    delay(1);
    
    // Disable RS485 transmitter
    PORTB &= ~(1 << 3);
}

// Add this debug function to check if UART is working
void debug_uart(void) {
    lcd_cmd(0x80);  // Move to first line
    lcd_string_f("UART Test");
    
    // Try to send a test character
    serial_putc('T');
    
    lcd_cmd(0xC0);  // Move to second line
    
    // Check if UART registers are set correctly
    if (UCSR0B & (1 << TXEN0)) {
        lcd_string_f("TX Enabled");
    } else {
        lcd_string_f("TX Disabled");
    }
    
    delay(1000);  // Wait to see the display
}
void serial1_putc(char c) {
    if (GetCMSEnabled()) {
        while (!(UCSR1A & (1 << UDRE1)));  // Wait until data register empty
        UDR1 = c;
    }
}

void serial_string(char *str) {
    if (GetRPTREnabled()) {
        RS485_EN();    // Enable transmitter
        _delay_ms(1);     // Small delay for line stabilization
        
        while (*str) {
			UCSR0A |= (1<<TXC0);
            while (!(UCSR0A & (1<<UDRE0)));  // Wait for empty transmit buffer
            UDR0 = *str++;
            while (!(UCSR0A & (1<<TXC0)));   // Wait for complete transmission
        }
        
        // Send CR+LF
        while (!(UCSR0A & (1<<UDRE0)));
        //UDR0 = '\r';
        while (!(UCSR0A & (1<<TXC0)));
        
        while (!(UCSR0A & (1<<UDRE0)));
        //UDR0 = '\n';
        while (!(UCSR0A & (1<<TXC0)));
        
        _delay_ms(10);     // Small delay before disabling
        RS485_DN();    // Disable transmitter
    }
}

void serial1_string(char *str) {
    if (GetCMSEnabled()) {
        while (*str!='\0') {
        serial1_putc(*str);
        str++;
        }
        ///serial_putc('\r');
        //serial_putc('\n');
    }
}

void serial_string_f(const char *str) {
    if (GetRPTREnabled()) {
        while (*str) serial_putc(*str++);
    }
}

void serial1_string_f(const char *str) {
    if (GetCMSEnabled()) {
        while (*str) serial1_putc(*str++);
    }
}

void serial_println(char *str) {
    if (GetRPTREnabled()) {
        serial_string(str);
        serial_putc('\r');
        serial_putc('\n');
    }
}

void serial_println_f(const char *str) {
    if (GetRPTREnabled()) {
        serial_string_f(str);
        serial_putc('\r');
        serial_putc('\n');
    }
}

void serial1_println_f(const char *str) {
    if (GetCMSEnabled()) {
        serial1_string_f(str);
        serial1_putc('\r');
        serial1_putc('\n');
    }
}

void serial_clear_buffer(void) {
    buffer_len = 0;
    buff_index = 0;
    ch_start = 0;
    ch_end = 0;
    num_start = 0;
    num_end = 0;
    b_direct_buffer = 0;
}

void serial1_clear_buffer(void) {
    buffer1_len = 0;
    buff1_index = 0;
}

unsigned char serial_available(void) {
    return (GetRPTREnabled() && buffer_len > 0);
}

unsigned char serial1_available(void) {
    return (GetCMSEnabled() && buffer1_len > 0);
}

char serial_getc(void) {
    char data = 0;
    
    if (GetRPTREnabled() && buffer_len > 0) {
        data = serial_buff[ch_start];
        ch_start = (ch_start + 1) % SERIAL_BUFFER_SIZE;
        buffer_len--;
    }
    
    return data;
}

char serial1_getc(void) {
    char data = 0;
    
    if (GetCMSEnabled() && buffer1_len > 0) {
        data = serial1_buff[buff1_index];
        buff1_index = (buff1_index + 1) % SERIAL1_BUFFER_SIZE;
        buffer1_len--;
    }
    
    return data;
}
// **********************************************************************

// Function to read a single character from UART0

char read_serial_char(void) {
    // Wait until data is available
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}

//=======================================================================UART1 Commands ==========================================
void serial1_init(void) {
    unsigned int ubrr = (f_cpu / (16UL * BAUD_RATE)) - 1;
    
    UBRR1H = (unsigned char)(ubrr >> 8);
    UBRR1L = (unsigned char)ubrr;
    //UCSR1A = (1 << U2X1);
    UCSR1B = (1 << RXEN1) | (1 << TXEN1) | (1 << RXCIE1);  // Enable RX, TX and RX interrupt
    UCSR1C = (1 << UCSZ11) | (1 << UCSZ10);                // 8-bit data, 1 stop bit
    //#asm("sei") 
    serial1_clear_buffer();
}

/*void read_full_string()
{
    char c;
    rcv_index = 0;

    while (1) {
        if (UCSR1A & (1 << RXC1)) {   // Byte available
            c = UDR1;

            if (c == '\0') {          // String ended
                rcv_buffer[rcv_index] = '\0';  
                serial1_string(rcv_buffer);
                return;               // Exit function with full string
            }

            rcv_buffer[rcv_index++] = c;
        }
    }
} */
ISR(USART0_RX_vect) {
char c = UDR1;

    if (rcv_index < BUFFER_SIZE - 1)
    {
        rcv_buffer[rcv_index++] = c;

        if (c == '\0' || c == '\r' || c == '\n')      // or '\r' or '\n' or '\0'
        {
            rcv_buffer[rcv_index] = '\0';
            data_ready = 1;   // FLAG SET
            rcv_index = 0;
        }
    }
}     
//interrupt [USART1_RXC] void usart1_rx_isr(void) 
//{
//  char received_byte;
 // unsigned char status;

//  status = UCSR1A;

//char received_byte = UDR1; 
  //if(rcv_index < BUFFER_SIZE - 1) {    
         
  //      rcv_buffer[rcv_index++] =  received_byte;  // Store in buffer
  //      if (received_byte == '\0') {  // If newline received
  //          rcv_buffer[rcv_index] = '\0';  // Null-terminate string
  //          rcv_index = 0;  // Reset buffer index for next message 
  //          serial1_string(rcv_buffer); 
 //           rcv_buffer[0] = '\0';
 //       }
     /*serial1_string("rcved");
       serial1_string(rcv_buffer);  
       rcv_buffer[0] = '\0';*/
   // }  
 /* if (GetCMSEnabled()) {
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN)) == 0)
    {
      // Echo received byte
      //            while(!(UCSR1A & (1 << UDRE1)));
      //            UDR1 = received_byte;

      if (buff_index < 511) {
        serial_buff[buff_index++] = received_byte;

        // Process 3-character commands
        if (buff_index >= 3)
        {   
               
              
          if (serial_buff[buff_index - 3] >= '1' && serial_buff[buff_index - 3] <= '8' &&
              serial_buff[buff_index - 2] >= '0' && serial_buff[buff_index - 2] <= '9' &&
              serial_buff[buff_index - 1] >= '0' && serial_buff[buff_index - 1] <= '9')
          {
            unsigned char value = ((serial_buff[buff_index - 2] - '0') * 10) + (serial_buff[buff_index - 1] - '0');

            switch (serial_buff[buff_index - 3])
            {
              case '1': // Day Hours (0-23)
                if (value <= 23) {
                  Eeprom_WriteByte(ADD_DAY_HOUR, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1);  
                }
                break;

              case '2': // Day Minutes (0-59)
                if (value <= 59) {
                  Eeprom_WriteByte(ADD_DAY_MINUTE, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '3': // Night Hours (0-23)
                if (value <= 23) {
                  Eeprom_WriteByte(ADD_NIGHT_HOUR, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '4': // Night Minutes (0-59)
                if (value <= 59) {
                  Eeprom_WriteByte(ADD_NIGHT_MINUTE, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '5': // Arm Hours (0-23)
                if (value <= 23) {
                  Eeprom_WriteByte(ADD_ARM_HOUR, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '6': // Arm Minutes (0-59)
                if (value <= 59) {
                  Eeprom_WriteByte(ADD_ARM_MINUTE, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '7': // Disarm Hours (0-23)
                if (value <= 23) {
                  Eeprom_WriteByte(ADD_DISARM_HOUR, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;

              case '8': // Disarm Minutes (0-59)
                if (value <= 59) {
                  Eeprom_WriteByte(ADD_DISARM_MINUTE, value);
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 3];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 2];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = serial_buff[buff_index - 1];
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
                  while(!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';
                  send_ok(1); 
                }
                break;
            }
            buff_index = 0;
            return;
          }

          // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

          // Other 3-character commands

          // Day/ Night Mode
          if (serial_buff[buff_index - 3] == 'D' &&
              serial_buff[buff_index - 2] == 'N')
          {
            switch (serial_buff[buff_index - 1])
            {
              case 'M': // Manual Mode
                Eeprom_WriteByte(ADD_DAY_NIGHT_TYPE, 1);
                buff_index = 0;
                send_ok(1);
                return;

              case 'A': // Automatic Mode
                Eeprom_WriteByte(ADD_DAY_NIGHT_TYPE, 0);
                buff_index = 0;
                send_ok(1);
                return;
            }
            
          }

          // Sounder Delay Time
          else if (serial_buff[buff_index - 3] == 'S' &&
                   serial_buff[buff_index - 2] >= '0' && serial_buff[buff_index - 2] <= '2' &&
                   serial_buff[buff_index - 1] >= '0' && serial_buff[buff_index - 1] <= '9')
          {
            // Convert two digits to numeric value (03-20 seconds)
            unsigned char value = ((serial_buff[buff_index - 2] - '0') * 10) +
                                  (serial_buff[buff_index - 1] - '0');

            // Only accept values 3-20
            if (value >= 3 && value <= 20) {
              Eeprom_WriteByte(ADD_SOUNDER_TIME, value);
              send_ok(1);
            }
            buff_index = 0;
            return;
          }
          // Arm/Disarm Mode
          else if (serial_buff[buff_index - 3] == 'A' &&
                   serial_buff[buff_index - 2] == 'D')
          {
            switch (serial_buff[buff_index - 1])
            {
              case 'M': // Manual Mode
                Eeprom_WriteByte(ADD_ARM_DISARM_TYPE, 1);
                buff_index = 0;
                send_ok(1);
                return;

              case 'A': // Automatic Mode
                Eeprom_WriteByte(ADD_ARM_DISARM_TYPE, 0);
                buff_index = 0;
                send_ok(1);
                return;
            }
          }

          // Night Cut Trigger
          else if (serial_buff[buff_index - 3] == 'N' &&
                   serial_buff[buff_index - 2] == 'C')
          {
            switch (serial_buff[buff_index - 1])
            {
              case 'E': // Enable
                Eeprom_WriteByte(ADD_NIGHT_TRIG, 1);
                buff_index = 0;
                send_ok(1);
                return;

              case 'D': // Disable
                Eeprom_WriteByte(ADD_NIGHT_TRIG, 0);
                buff_index = 0;
                send_ok(1);
                return;
            }
          }

          // System Reset Command
          else if (serial_buff[buff_index - 3] == 'R' &&
                   serial_buff[buff_index - 2] == 'S' &&
                   serial_buff[buff_index - 1] == 'T')
          { 
            send_ok(1);
            Reset();
            buff_index = 0;
            return;
          }

          // System Arm Command
          else if (serial_buff[buff_index - 3] == 'A' &&
                   serial_buff[buff_index - 2] == 'R' &&
                   serial_buff[buff_index - 1] == 'M')
          {
            Eeprom_WriteByte(ADD_ARM_DISARM, 1);
            l_armed = 1;
            l_disarmed = 0;
            buff_index = 0;
            send_ok(1);
            return;
          }
          // System Disarm Command
          else if (serial_buff[buff_index - 3] == 'D' &&
                   serial_buff[buff_index - 2] == 'R' &&
                   serial_buff[buff_index - 1] == 'M')
          {
            Eeprom_WriteByte(ADD_ARM_DISARM, 0);
            l_armed = 0;
            l_disarmed = 1;
            buff_index = 0;
            send_ok(1);
            return;
          }
          // Zone Configuration Commands
          else if (serial_buff[buff_index - 3] == 'Z' &&
                   (serial_buff[buff_index - 2] >= '1' && serial_buff[buff_index - 2] <= '8'))
          {
            unsigned char zone = serial_buff[buff_index - 2] - '1';
            SetZoneMode(zone, serial_buff[buff_index - 1], 1);
            buff_index = 0;
            return;
          }
          
          // Silent
          else if (serial_buff[buff_index - 3] == 'S' &&
                    serial_buff[buff_index - 2] == 'L' &&
                    serial_buff[buff_index - 1] == 'T')
          { 
                Sounder_Silent();
                send_ok(1);
                buff_index = 0;
                return;
          }
          
          // Factory Reset Command
          else if (serial_buff[buff_index - 3] == 'F' &&
                   serial_buff[buff_index - 2] == 'R' &&
                   serial_buff[buff_index - 1] == 'S')
          { 
            send_ok(1);
            FactoryReset();
            buff_index = 0;
            return;
          }
          
          // Logs
          else if (serial_buff[buff_index - 3] == 'L' &&
                   serial_buff[buff_index - 2] == 'L' &&
                   serial_buff[buff_index - 1] == 'T')
          {
            unsigned int log_addr;
            unsigned char zone, type, hour, minute, second, day, month, year;

            for (log_addr = ADD_LOG_BASE; log_addr < ADD_LOG_BASE + (500 * 8); log_addr += 8)
            {
              // Read log entry (8 bytes)
              zone = Eeprom_ReadByte(log_addr);
              if (zone == 0xFF) continue; // Skip empty logs

              type = Eeprom_ReadByte(log_addr + 1);
              hour = Eeprom_ReadByte(log_addr + 2);
              minute = Eeprom_ReadByte(log_addr + 3);
              second = Eeprom_ReadByte(log_addr + 4);
              day = Eeprom_ReadByte(log_addr + 5);
              month = Eeprom_ReadByte(log_addr + 6);
              year = Eeprom_ReadByte(log_addr + 7);

              // Send "Z:"
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = 'Z';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ':';

              // Send zone number (2 digits)
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (zone / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (zone % 10);

              // Send " T:"
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ' ';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = 'T';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ':';

              // Send type
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + type;

              // Send " "
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ' ';

              // Send time HH:MM:SS
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (hour / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (hour % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ':';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (minute / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (minute % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ':';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (second / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (second % 10);

              // Send " "
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ' ';

              // Send date DD/MM/YY
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (day / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (day % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '/';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (month / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (month % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '/';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (year / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (year % 10);

              // Send line end
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';

              delay(10); // Small delay between logs
            }

            buff_index = 0;
            return;
          }
          // Holiday List
          else if (serial_buff[buff_index - 3] == 'H' &&
                   serial_buff[buff_index - 2] == 'L' &&
                   serial_buff[buff_index - 1] == 'T')
          {
            unsigned int holiday_addr;
            unsigned char day, month, year;

            for (holiday_addr = ADD_HOLIDAY_BASE; holiday_addr < ADD_HOLIDAY_BASE + (30 * 3); holiday_addr += 3)
            {
              // Read holiday entry (3 bytes: day, month, year)
              day = Eeprom_ReadByte(holiday_addr);
              if (day == 0xFF) continue; // Skip empty holiday slots

              month = Eeprom_ReadByte(holiday_addr + 1);
              year = Eeprom_ReadByte(holiday_addr + 2);

              // Send "H:"
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = 'H';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = ':';

              // Send date DD/MM/YY
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (day / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (day % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '/';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (month / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (month % 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '/';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (year / 10);
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '0' + (year % 10);

              // Send line end
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '\r';
              while (!(UCSR1A & (1 << UDRE1))); UDR1 = '\n';

              delay(10); // Small delay between holidays
            }

            buff_index = 0;
            return;
          }
        }

        // Handle CR/LF
        if (received_byte == '\r' || received_byte == '\n')
        {
          while (!(UCSR1A & (1 << UDRE1)));
          UDR1 = '\r';
          while (!(UCSR1A & (1 << UDRE1)));
          UDR1 = '\n';

          buffer_len = buff_index;
          buff_index = 0;
        }
      }
    }
  }      */
//}
//=======================================================================
void slice(volatile char *src, int start, int length) {
    int i; 
    //char a[20];  
    //serial1_string("slice run\n");
   // serial1_string(src);
    for (i = 0; i < length && src[start + i] != '\0'; i++) { 
        int j=start + i;
        dest[i] = src[j];  
       // serial1_string(dest);
    }   
    dest[i] = '\0';
    serial1_string("{");
    serial1_string(dest);
    serial1_string("}");
    //dest[i] = '\0';  //Null terminate 
   // rcv_buffer[0] = '\0';
   // return dest;
}
void empty_data(){
int h;
for(h=0;h<20;h++){
   dest[h]='\0';
}
}

void SendPhoneNumbersToUARTs();
void ProcessSerialPhoneUpdate(volatile char rcv_buffer1[20],unsigned int ADD_NUM_BASE) {
    char *comma_ptr;
    unsigned char index;
    unsigned int base_addr;
    unsigned char i = 0;
    char *phone_ptr;

    comma_ptr = strchr((const char*)rcv_buffer, ',');
    if (comma_ptr != NULL) {
        // Extract index (number before comma)
        index = 0;
        phone_ptr = rcv_buffer;
        while (phone_ptr < comma_ptr) {
            if (*phone_ptr >= '0' && *phone_ptr <= '9') {
                index = index * 10 + (*phone_ptr - '0');
            }
            phone_ptr++;
        }

        // Validate index (1 to 10)
        if (index >= 1 && index <= 10) {
            base_addr = ADD_NUM_BASE + (index - 1) * 16;
            phone_ptr = comma_ptr + 1; // Start of phone number

            // Clear the 16-byte slot first
            for (i = 0; i < 16; i++) {
                Eeprom_WriteByte(base_addr + i, ' ');
            }

            // Write new phone number
            i = 0;
            while (*phone_ptr != '\0' && i < 15) {
                Eeprom_WriteByte(base_addr + i, *phone_ptr);
                phone_ptr++;
                i++;
                delay(5);
            }
            // Ensure null termination or trailing spaces if needed, 
            // but the loop above plus the initial clearing handles it.
            
           // SendPhoneNumbersToUARTs();
           // serial_string("PHONE_UPDATED\r\n");
        }
    }
}

//======================================================================
void rcv_action() {
    if (data_ready == 0) return;
    data_ready = 0;
    if(strncmp((const char*)rcv_buffer,"FIRE_Num_",9)==0){
    if (isdigit(rcv_buffer[9])) { 
        slice(rcv_buffer,9,13);
        ProcessSerialPhoneUpdate(dest, ADD_FIRE_NUM_BASE);
    }
    }
    else if(strncmp((const char*)rcv_buffer,"INTR_Num_",9)==0){
     if(isdigit(rcv_buffer[9])){        
      slice(rcv_buffer,9,13);
      ProcessSerialPhoneUpdate(dest, ADD_INTR_NUM_BASE);
     }
    
    }else if(strncmp((const char*)rcv_buffer,"DEL_FIRE_NUM-",13)==0){
        unsigned char index;
        unsigned int base_addr;
        int i;
        //slice(rcv_buffer,13,1);
        index=rcv_buffer[13]-'0';
        index-=1;
        base_addr=ADD_FIRE_NUM_BASE+(index*16);
        for(i=0;i<=15;i++){
        Eeprom_WriteByte(base_addr+i, '\0');
        }
    }else if(strncmp((const char*)rcv_buffer,"DEL_INTR_NUM-",13)==0){
        unsigned char index;
        unsigned int base_addr;
        int i;
        //slice(rcv_buffer,13,1);
        index=rcv_buffer[13]-'0';  
        index-=1;
        base_addr=ADD_INTR_NUM_BASE+(index*16);
        for(i=0;i<=15;i++){
        Eeprom_WriteByte(base_addr+i, '\0');
        }
    }else if(strncmp((const char*)rcv_buffer,"FcallEn-",8)==0){
        Eeprom_WriteByte(ADD_FIRE_CALL_SET, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer,"FcallSq-",8)==0){
        Eeprom_WriteByte(ADD_FIRE_CALL_SEQ, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer,"IcallEn-",8)==0){
        Eeprom_WriteByte(ADD_INTR_CALL_SET, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer,"IcallSq-",8)==0){
        Eeprom_WriteByte(ADD_INTR_CALL_SEQ, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer,"TcallEn-",8)==0){
        Eeprom_WriteByte(ADD_TMPR_CALL_SET, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer,"TcallSq-",8)==0){
        Eeprom_WriteByte(ADD_TMPR_CALL_SEQ, rcv_buffer[8]-'0');
    }else if(strncmp((const char*)rcv_buffer, "ZONE", 4) == 0){  
    
      if(strncmp((const char*)&rcv_buffer[4],"01",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_1_TYPE, 0);
      serial1_string("$ZONE1:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_1_TYPE, 1); 
      serial1_string("$ZONE1:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_1_TYPE, 2);  
      serial1_string("$ZONE1:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_1_TYPE, 3);
      serial1_string("$ZONE1:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"02",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_2_TYPE, 0);
      serial1_string("$ZONE2:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_2_TYPE, 1); 
      serial1_string("$ZONE2:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_2_TYPE, 2);  
      serial1_string("$ZONE2:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_2_TYPE, 3);
      serial1_string("$ZONE2:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"03",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_3_TYPE, 0);
      serial1_string("$ZONE3:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_3_TYPE, 1); 
      serial1_string("$ZONE3:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_3_TYPE, 2);  
      serial1_string("$ZONE3:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_3_TYPE, 3);
      serial1_string("$ZONE3:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"04",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_4_TYPE, 0);
      serial1_string("$ZONE4:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_4_TYPE, 1); 
      serial1_string("$ZONE4:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_4_TYPE, 2);  
      serial1_string("$ZONE4:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_4_TYPE, 3);
      serial1_string("$ZONE4:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"05",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_5_TYPE, 0);
      serial1_string("$ZONE5:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_5_TYPE, 1); 
      serial1_string("$ZONE5:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_5_TYPE, 2);  
      serial1_string("$ZONE5:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_5_TYPE, 3);
      serial1_string("$ZONE5:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"06",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_6_TYPE, 0);
      serial1_string("$ZONE6:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_6_TYPE, 1); 
      serial1_string("$ZONE6:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_6_TYPE, 2);  
      serial1_string("$ZONE6:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_6_TYPE, 3);
      serial1_string("$ZONE6:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"07",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_7_TYPE, 0);
      serial1_string("$ZONE7:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_7_TYPE, 1); 
      serial1_string("$ZONE7:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_7_TYPE, 2);  
      serial1_string("$ZONE7:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_7_TYPE, 3);
      serial1_string("$ZONE7:ISOL");
      }
      }else if(strncmp((const char*)&rcv_buffer[4],"08",2)==0){ 
      if(rcv_buffer[7]=='F'){  
      Eeprom_WriteByte(ADD_ZONE_8_TYPE, 0);
      serial1_string("$ZONE8:FIRE");
      }
      //slice(rcv_buffer,7,3);
      if(rcv_buffer[7]=='D'){
      Eeprom_WriteByte(ADD_ZONE_8_TYPE, 1); 
      serial1_string("$ZONE8:DAYS");
      }
      //slice(rcv_buffer,7,5);
      if(rcv_buffer[7]=='N'){
      Eeprom_WriteByte(ADD_ZONE_8_TYPE, 2);  
      serial1_string("$ZONE8:NIGT");
      }
      //slice(rcv_buffer,7,7);
      if(rcv_buffer[7]=='I'){
      Eeprom_WriteByte(ADD_ZONE_8_TYPE, 3);
      serial1_string("$ZONE8:ISOL");
      }
      }   
      
    }else if (strncmp((const char*)rcv_buffer, "DaysT", 5) == 0) {
        Eeprom_WriteByte(ADD_DAY_HOUR, (rcv_buffer[5] - '0') * 10 + (rcv_buffer[6] - '0'));
        Eeprom_WriteByte(ADD_DAY_MINUTE, (rcv_buffer[8] - '0') * 10 + (rcv_buffer[9] - '0'));
    } else if (strncmp((const char*)rcv_buffer, "NghtT", 5) == 0) {
        Eeprom_WriteByte(ADD_NIGHT_HOUR, (rcv_buffer[5] - '0') * 10 + (rcv_buffer[6] - '0'));
        Eeprom_WriteByte(ADD_NIGHT_MINUTE, (rcv_buffer[8] - '0') * 10 + (rcv_buffer[9] - '0'));
    } else if (strncmp((const char*)rcv_buffer, "Pmode", 5) == 0) {
        if (strncmp((const char*)rcv_buffer + 5, "MANUAL", 6) == 0) Eeprom_WriteByte(ADD_DAY_NIGHT_TYPE, 1);
        else if (strncmp((const char*)rcv_buffer + 5, "AUTOMT", 6) == 0) Eeprom_WriteByte(ADD_DAY_NIGHT_TYPE, 0);
    } else if (strncmp((const char*)rcv_buffer, "AD_MD", 5) == 0) {
        if (strncmp((const char*)rcv_buffer + 5, "MANUAL", 6) == 0) Eeprom_WriteByte(ADD_ARM_DISARM_TYPE, 1);
        else if (strncmp((const char*)rcv_buffer + 5, "AUTOMT", 6) == 0) Eeprom_WriteByte(ADD_ARM_DISARM_TYPE, 0);
    } else if (strncmp((const char*)rcv_buffer, "Arm_T", 5) == 0) {
        Eeprom_WriteByte(ADD_ARM_HOUR, (rcv_buffer[5] - '0') * 10 + (rcv_buffer[6] - '0'));
        Eeprom_WriteByte(ADD_ARM_MINUTE, (rcv_buffer[8] - '0') * 10 + (rcv_buffer[9] - '0'));
    } else if (strncmp((const char*)rcv_buffer, "DarmT", 5) == 0) {
        Eeprom_WriteByte(ADD_DISARM_HOUR, (rcv_buffer[5] - '0') * 10 + (rcv_buffer[6] - '0'));
        Eeprom_WriteByte(ADD_DISARM_MINUTE, (rcv_buffer[8] - '0') * 10 + (rcv_buffer[9] - '0'));
    } else if (strncmp((const char*)rcv_buffer, "NTcut", 5) == 0) {
        if (strncmp((const char*)rcv_buffer + 5, "ENABLE", 6) == 0) Eeprom_WriteByte(ADD_NIGHT_TRIG, 1);
        else if (strncmp((const char*)rcv_buffer + 5, "DISABLE", 7) == 0) Eeprom_WriteByte(ADD_NIGHT_TRIG, 0);
    } else if (strncmp((const char*)rcv_buffer, "SndrT", 5) == 0) {
        Eeprom_WriteByte(ADD_SOUNDER_TIME, (rcv_buffer[8] - '0') * 10 + (rcv_buffer[9] - '0'));
    } else if (strncmp((const char*)rcv_buffer, "A_Drm", 5) == 0) {
        if (strncmp((const char*)rcv_buffer + 5, "ARM", 3) == 0) Eeprom_WriteByte(ADD_ARM_DISARM, 1);
        else if (strncmp((const char*)rcv_buffer + 5, "DISARM", 6) == 0) Eeprom_WriteByte(ADD_ARM_DISARM, 0);
    } else if (strncmp((const char*)rcv_buffer, "Bname,", 6) == 0) {
        int i = 0;
        char *ptr = rcv_buffer + 6;
        // Clear previous data
        for (i = 0; i < 32; i++) {
            Eeprom_WriteByte(ADD_NAME_BASE + i, '\0');
        }
        i = 0;
        while (*ptr != '\0' && i < 31) {
            Eeprom_WriteByte(ADD_NAME_BASE + i, *ptr);
            ptr++;
            i++;
        }
        Eeprom_WriteByte(ADD_NAME_BASE + i, '\0');
    } else if (strncmp((const char*)rcv_buffer, "Baddr,", 6) == 0) {
        int i = 0;
        char *ptr = rcv_buffer + 6;
        // Clear previous data
        for (i = 0; i < 32; i++) {
            Eeprom_WriteByte(ADD_ADDRESS_BASE + i, '\0');
        }
        i = 0;
        while (*ptr != '\0' && i < 31) {
            Eeprom_WriteByte(ADD_ADDRESS_BASE + i, *ptr);
            ptr++;
            i++;
        }
        Eeprom_WriteByte(ADD_ADDRESS_BASE + i, '\0');
    } else if(rcv_buffer[0] == '$') {
        // Handle all $-prefixed commands here to avoid conflicts with other handlers
        // This check must come BEFORE the DATETIME handler which uses strstr("20")
        if(strncmp((const char*)rcv_buffer, "$HTH,", 5)==0){
            // Send all device info via UARTs if phone number matches
            unsigned char i, val, ph_idx, m, phone_matched;
            char info_buf[40];
            char rcv_phone[16];
            char stored_phone[16];
            unsigned int ph_addr;
            
            // Extract phone number from command (after "$HTH,")
            for(i = 0; i < 15 && rcv_buffer[5 + i] != '\0' && rcv_buffer[5 + i] != '\r' && rcv_buffer[5 + i] != '\n'; i++) {
                rcv_phone[i] = rcv_buffer[5 + i];
            }
            rcv_phone[i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            phone_matched = 0;
            for(ph_idx = 0; ph_idx < 10 && !phone_matched; ph_idx++) {
                ph_addr = ADD_FIRE_NUM_BASE + (ph_idx * 16);
                
                // Read stored phone number
                for(m = 0; m < 16; m++) {
                    val = Eeprom_ReadByte(ph_addr + m);
                    if(val == 0xFF || val == 0x00) val = '\0';
                    stored_phone[m] = val;
                }
                stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(m = 14; m > 0; m--) {
                    if(stored_phone[m] == ' ' || stored_phone[m] == '\0') {
                        stored_phone[m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp((const char*)rcv_phone, stored_phone) == 0) {
                    phone_matched = 1;
                }
            }
            
            // Only send health status if phone number matched
            if(!phone_matched) {
                serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                // Building Name
                info_buf[0] = 'N'; info_buf[1] = 'A'; info_buf[2] = 'M'; info_buf[3] = 'E'; info_buf[4] = ':';
                for(i = 0; i < 32; i++) {
                    val = Eeprom_ReadByte(ADD_NAME_BASE + i);
                    if(val == 0xFF || val == 0x00) val = ' ';
                    info_buf[5 + i] = val;
                }
                info_buf[37] = '\0';
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Building Address
                info_buf[0] = 'A'; info_buf[1] = 'D'; info_buf[2] = 'D'; info_buf[3] = 'R'; info_buf[4] = ':';
                for(i = 0; i < 32; i++) {
                    val = Eeprom_ReadByte(ADD_ADDRESS_BASE + i);
                    if(val == 0xFF || val == 0x00) val = ' ';
                    info_buf[5 + i] = val;
                }
                info_buf[37] = '\0';
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Day Time
                sprintf(info_buf, "DAYT:%02d:%02d", Eeprom_ReadByte(ADD_DAY_HOUR), Eeprom_ReadByte(ADD_DAY_MINUTE));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Night Time
                sprintf(info_buf, "NGTT:%02d:%02d", Eeprom_ReadByte(ADD_NIGHT_HOUR), Eeprom_ReadByte(ADD_NIGHT_MINUTE));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Day/Night Mode
                val = Eeprom_ReadByte(ADD_DAY_NIGHT_TYPE);
                sprintf(info_buf, "DNMD:%s", (val == 1) ? "MANUAL" : "AUTO");
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Arm/Disarm Status
                val = Eeprom_ReadByte(ADD_ARM_DISARM);
                sprintf(info_buf, "ARMS:%s", (val == 1) ? "ARM" : "DISARM");
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Arm/Disarm Mode
                val = Eeprom_ReadByte(ADD_ARM_DISARM_TYPE);
                sprintf(info_buf, "ADMD:%s", (val == 1) ? "MANUAL" : "AUTO");
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Arm Time
                sprintf(info_buf, "ARMT:%02d:%02d", Eeprom_ReadByte(ADD_ARM_HOUR), Eeprom_ReadByte(ADD_ARM_MINUTE));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Disarm Time
                sprintf(info_buf, "DSMT:%02d:%02d", Eeprom_ReadByte(ADD_DISARM_HOUR), Eeprom_ReadByte(ADD_DISARM_MINUTE));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Sounder Time
                sprintf(info_buf, "SNDT:%02d", Eeprom_ReadByte(ADD_SOUNDER_TIME));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Night Cut Trigger
                val = Eeprom_ReadByte(ADD_NIGHT_TRIG);
                sprintf(info_buf, "NCUT:%s", (val == 1) ? "ENABLE" : "DISABLE");
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Zone Types (Z1-Z8)
                for(i = 0; i < 8; i++) {
                    val = Eeprom_ReadByte(ADD_ZONE_1_TYPE + i);
                    info_buf[0] = 'Z';
                    info_buf[1] = '1' + i;
                    info_buf[2] = ':';
                    if(val == 0) { info_buf[3] = 'F'; info_buf[4] = 'I'; info_buf[5] = 'R'; info_buf[6] = 'E'; info_buf[7] = '\0'; }
                    else if(val == 1) { info_buf[3] = 'D'; info_buf[4] = 'A'; info_buf[5] = 'Y'; info_buf[6] = '\0'; }
                    else if(val == 2) { info_buf[3] = 'N'; info_buf[4] = 'I'; info_buf[5] = 'G'; info_buf[6] = 'H'; info_buf[7] = 'T'; info_buf[8] = '\0'; }
                    else { info_buf[3] = 'I'; info_buf[4] = 'S'; info_buf[5] = 'O'; info_buf[6] = '\0'; }
                    serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                    serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                    delay(10);
                }
                
                // Entry Delay
                sprintf(info_buf, "EDEL:%02d", Eeprom_ReadByte(ADD_ENTRY_DELAY));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // Exit Delay
                sprintf(info_buf, "XDEL:%02d", Eeprom_ReadByte(ADD_EXIT_DELAY));
                serial_string(info_buf); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string(info_buf); serial1_putc(0x0D); serial1_putc(0x0A);
                delay(10);
                
                // End marker
                serial_string("$END"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("$END"); serial1_putc(0x0D); serial1_putc(0x0A);
            }
        } else if(strncmp((const char*)rcv_buffer, "$SLT,", 5)==0){
            // Silent system if phone number matches
            unsigned char slt_i, slt_val, slt_ph_idx, slt_m, slt_phone_matched;
            char slt_rcv_phone[16];
            char slt_stored_phone[16];
            unsigned int slt_ph_addr;
            
            // Extract phone number from command (after "$SLT,")
            for(slt_i = 0; slt_i < 15 && rcv_buffer[5 + slt_i] != '\0' && rcv_buffer[5 + slt_i] != '\r' && rcv_buffer[5 + slt_i] != '\n'; slt_i++) {
                slt_rcv_phone[slt_i] = rcv_buffer[5 + slt_i];
            }
            slt_rcv_phone[slt_i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            slt_phone_matched = 0;
            for(slt_ph_idx = 0; slt_ph_idx < 10 && !slt_phone_matched; slt_ph_idx++) {
                slt_ph_addr = ADD_FIRE_NUM_BASE + (slt_ph_idx * 16);
                
                // Read stored phone number
                for(slt_m = 0; slt_m < 16; slt_m++) {
                    slt_val = Eeprom_ReadByte(slt_ph_addr + slt_m);
                    if(slt_val == 0xFF || slt_val == 0x00) slt_val = '\0';
                    slt_stored_phone[slt_m] = slt_val;
                }
                slt_stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(slt_m = 14; slt_m > 0; slt_m--) {
                    if(slt_stored_phone[slt_m] == ' ' || slt_stored_phone[slt_m] == '\0') {
                        slt_stored_phone[slt_m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp(slt_rcv_phone, slt_stored_phone) == 0) {
                    slt_phone_matched = 1;
                }
            }
            
            // Only silent if phone number matched
            if(!slt_phone_matched) {
                serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                Sounder_Silent();
                serial_string("OK_SLT"); serial_putc(0x0D); serial_putc(0x0A); 
                serial_string("$SILENT"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("OK_SLT"); serial1_putc(0x0D); serial1_putc(0x0A);
                serial1_string("$SILENT"); serial1_putc(0x0D); serial1_putc(0x0A);
            }
        } else if(strncmp((const char*)rcv_buffer, "$RST,", 5)==0){
            // Reset system if phone number matches
            unsigned char rst_i, rst_val, rst_ph_idx, rst_m, rst_phone_matched;
            char rst_rcv_phone[16];
            char rst_stored_phone[16];
            unsigned int rst_ph_addr;
            
            // Extract phone number from command (after "$RST,")
            for(rst_i = 0; rst_i < 15 && rcv_buffer[5 + rst_i] != '\0' && rcv_buffer[5 + rst_i] != '\r' && rcv_buffer[5 + rst_i] != '\n'; rst_i++) {
                rst_rcv_phone[rst_i] = rcv_buffer[5 + rst_i];
            }
            rst_rcv_phone[rst_i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            rst_phone_matched = 0;
            for(rst_ph_idx = 0; rst_ph_idx < 10 && !rst_phone_matched; rst_ph_idx++) {
                rst_ph_addr = ADD_FIRE_NUM_BASE + (rst_ph_idx * 16);
                
                // Read stored phone number
                for(rst_m = 0; rst_m < 16; rst_m++) {
                    rst_val = Eeprom_ReadByte(rst_ph_addr + rst_m);
                    if(rst_val == 0xFF || rst_val == 0x00) rst_val = '\0';
                    rst_stored_phone[rst_m] = rst_val;
                }
                rst_stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(rst_m = 14; rst_m > 0; rst_m--) {
                    if(rst_stored_phone[rst_m] == ' ' || rst_stored_phone[rst_m] == '\0') {
                        rst_stored_phone[rst_m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp((const char*)rst_rcv_phone, rst_stored_phone) == 0) {
                    rst_phone_matched = 1;
                }
            }
            
            // Only reset if phone number matched
            if(!rst_phone_matched) {
                //serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                //serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                serial_string("OK_RST"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("OK_RST"); serial1_putc(0x0D); serial1_putc(0x0A);
                Reset();
            }
        } else if(strncmp((const char*)rcv_buffer, "$TRIG,", 6)==0){
            // Trigger system if phone number matches
            unsigned char trig_i, trig_val, trig_ph_idx, trig_m, trig_phone_matched;
            char trig_rcv_phone[16];
            char trig_stored_phone[16];
            unsigned int trig_ph_addr;
            
            // Extract phone number from command (after "$TRIG,")
            for(trig_i = 0; trig_i < 15 && rcv_buffer[6 + trig_i] != '\0' && rcv_buffer[6 + trig_i] != '\r' && rcv_buffer[6 + trig_i] != '\n'; trig_i++) {
                trig_rcv_phone[trig_i] = rcv_buffer[6 + trig_i];
            }
            trig_rcv_phone[trig_i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            trig_phone_matched = 0;
            for(trig_ph_idx = 0; trig_ph_idx < 10 && !trig_phone_matched; trig_ph_idx++) {
                trig_ph_addr = ADD_FIRE_NUM_BASE + (trig_ph_idx * 16);
                
                // Read stored phone number
                for(trig_m = 0; trig_m < 16; trig_m++) {
                    trig_val = Eeprom_ReadByte(trig_ph_addr + trig_m);
                    if(trig_val == 0xFF || trig_val == 0x00) trig_val = '\0';
                    trig_stored_phone[trig_m] = trig_val;
                }
                trig_stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(trig_m = 14; trig_m > 0; trig_m--) {
                    if(trig_stored_phone[trig_m] == ' ' || trig_stored_phone[trig_m] == '\0') {
                        trig_stored_phone[trig_m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp((const char*)trig_rcv_phone, trig_stored_phone) == 0) {
                    trig_phone_matched = 1;
                }
            }
            
            // Only trigger if phone number matched
            if(!trig_phone_matched) {
                //serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                //serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                trigger_flag = 1;
                sounder_flag = 0;
                int_hoot(1);
                hooter_trig(1);
                serial_string("OK_TRIG"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("OK_TRIG"); serial1_putc(0x0D); serial1_putc(0x0A);
            }
        } else if(strncmp((const char*)rcv_buffer, "$A_DrmDISARM,", 13)==0){
            // Disarm system if phone number matches (check DISARM before ARM to avoid partial match)
            unsigned char dsm_i, dsm_val, dsm_ph_idx, dsm_m, dsm_phone_matched;
            char dsm_rcv_phone[16];
            char dsm_stored_phone[16];
            unsigned int dsm_ph_addr;
            
            // Extract phone number from command (after "$A_DrmDISARM,")
            for(dsm_i = 0; dsm_i < 15 && rcv_buffer[13 + dsm_i] != '\0' && rcv_buffer[13 + dsm_i] != '\r' && rcv_buffer[13 + dsm_i] != '\n'; dsm_i++) {
                dsm_rcv_phone[dsm_i] = rcv_buffer[13 + dsm_i];
            }
            dsm_rcv_phone[dsm_i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            dsm_phone_matched = 0;
            for(dsm_ph_idx = 0; dsm_ph_idx < 10 && !dsm_phone_matched; dsm_ph_idx++) {
                dsm_ph_addr = ADD_FIRE_NUM_BASE + (dsm_ph_idx * 16);
                
                // Read stored phone number
                for(dsm_m = 0; dsm_m < 16; dsm_m++) {
                    dsm_val = Eeprom_ReadByte(dsm_ph_addr + dsm_m);
                    if(dsm_val == 0xFF || dsm_val == 0x00) dsm_val = '\0';
                    dsm_stored_phone[dsm_m] = dsm_val;
                }
                dsm_stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(dsm_m = 14; dsm_m > 0; dsm_m--) {
                    if(dsm_stored_phone[dsm_m] == ' ' || dsm_stored_phone[dsm_m] == '\0') {
                        dsm_stored_phone[dsm_m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp((const char*)dsm_rcv_phone, dsm_stored_phone) == 0) {
                    dsm_phone_matched = 1;
                }
            }
            
            // Only disarm if phone number matched
            if(!dsm_phone_matched) {
                //serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                //serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                Eeprom_WriteByte(ADD_ARM_DISARM, 0);
                l_armed = 0;
                l_disarmed = 1;
                serial_string("OK_DISARM"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("OK_DISARM"); serial1_putc(0x0D); serial1_putc(0x0A);
            }
        } else if(strncmp((const char*)rcv_buffer, "$A_DrmARM,", 10)==0){
            // Arm system if phone number matches
            unsigned char arm_i, arm_val, arm_ph_idx, arm_m, arm_phone_matched;
            char arm_rcv_phone[16];
            char arm_stored_phone[16];
            unsigned int arm_ph_addr;
            
            // Extract phone number from command (after "$A_DrmARM,")
            for(arm_i = 0; arm_i < 15 && rcv_buffer[10 + arm_i] != '\0' && rcv_buffer[10 + arm_i] != '\r' && rcv_buffer[10 + arm_i] != '\n'; arm_i++) {
                arm_rcv_phone[arm_i] = rcv_buffer[10 + arm_i];
            }
            arm_rcv_phone[arm_i] = '\0';
            
            // Check if phone number matches any of the 10 stored numbers
            arm_phone_matched = 0;
            for(arm_ph_idx = 0; arm_ph_idx < 10 && !arm_phone_matched; arm_ph_idx++) {
                arm_ph_addr = ADD_FIRE_NUM_BASE + (arm_ph_idx * 16);
                
                // Read stored phone number
                for(arm_m = 0; arm_m < 16; arm_m++) {
                    arm_val = Eeprom_ReadByte(arm_ph_addr + arm_m);
                    if(arm_val == 0xFF || arm_val == 0x00) arm_val = '\0';
                    arm_stored_phone[arm_m] = arm_val;
                }
                arm_stored_phone[15] = '\0';
                
                // Trim trailing spaces from stored phone
                for(arm_m = 14; arm_m > 0; arm_m--) {
                    if(arm_stored_phone[arm_m] == ' ' || arm_stored_phone[arm_m] == '\0') {
                        arm_stored_phone[arm_m] = '\0';
                    } else {
                        break;
                    }
                }
                
                // Compare phone numbers
                if(strcmp((const char*)arm_rcv_phone, arm_stored_phone) == 0) {
                    arm_phone_matched = 1;
                }
            }
            
            // Only arm if phone number matched
            if(!arm_phone_matched) {
                //serial_string("NO_NOT_MATCHED"); serial_putc(0x0D); serial_putc(0x0A);
                //serial1_string("NO_NOT_MATCHED"); serial1_putc(0x0D); serial1_putc(0x0A);
            } else {
                Eeprom_WriteByte(ADD_ARM_DISARM, 1);
                l_armed = 1;
                l_disarmed = 0;
                serial_string("OK_ARM"); serial_putc(0x0D); serial_putc(0x0A);
                serial1_string("OK_ARM"); serial1_putc(0x0D); serial1_putc(0x0A);
            }
        }
    } else if (strstr((const char*)rcv_buffer, "DATETIME_") != NULL || strstr((const char*)rcv_buffer, "20") != NULL) {
        char *dt_ptr = rcv_buffer;
        unsigned char found = 0;
        
        // Search for the YYYY-MM-DDTHH:MM pattern in the buffer
        while (*dt_ptr) {
            if (strlen(dt_ptr) >= 16 && 
                dt_ptr[0] == '2' && dt_ptr[1] == '0' && 
                dt_ptr[4] == '-' && dt_ptr[7] == '-' && 
                dt_ptr[10] == 'T' && dt_ptr[13] == ':') {
                found = 1;
                break;
            }
            dt_ptr++;
        }

        if (found) {
            unsigned char year, month, day, hour, min;
            year = (dt_ptr[2] - '0') * 10 + (dt_ptr[3] - '0');
            month = (dt_ptr[5] - '0') * 10 + (dt_ptr[6] - '0');
            day = (dt_ptr[8] - '0') * 10 + (dt_ptr[9] - '0');
            hour = (dt_ptr[11] - '0') * 10 + (dt_ptr[12] - '0');
            min = (dt_ptr[14] - '0') * 10 + (dt_ptr[15] - '0');
            
            if (month >= 1 && month <= 12 && day >= 1 && day <= 31 && hour <= 23 && min <= 59) {
                RTC_SetYear(year);
                RTC_SetMonth(month);
                RTC_SetDay(day);
                RTC_SetHour(hour);
                RTC_SetMinute(min);
                RTC_SetSecond(0);
                
                // Update global variables for immediate home screen refresh
                c_year = year;
                c_month = month;
                c_day = day;
                c_hour = hour;
                c_min = min;
                c_sec = 0;
            }
        }
    } else if (strncmp((const char*)rcv_buffer, "LLT", 3) == 0) {
        unsigned int log_idx;
        unsigned int log_addr;
        unsigned char zone, type, hour, minute, day, month, year;
        unsigned char h12, k;
        char log_temp[40];

        for (log_idx = 1; log_idx <= 1000; log_idx++) {
            log_addr = ADD_LOG_BASE + (log_idx - 1) * 8;
            zone = Eeprom_ReadByte(log_addr);
            if (zone == 0xFF) continue;

            type = Eeprom_ReadByte(log_addr + 1);
            hour = Eeprom_ReadByte(log_addr + 2);
            minute = Eeprom_ReadByte(log_addr + 3);
            day = Eeprom_ReadByte(log_addr + 5);
            month = Eeprom_ReadByte(log_addr + 6);
            year = Eeprom_ReadByte(log_addr + 7);

            h12 = (hour > 12) ? (hour - 12) : ((hour == 0) ? 12 : hour);

            log_temp[0] = 'L';
            log_temp[1] = '0' + (log_idx / 100);
            log_temp[2] = '0' + ((log_idx / 10) % 10);
            log_temp[3] = '0' + (log_idx % 10);
            log_temp[4] = '_';
            log_temp[5] = 'Z';
            log_temp[6] = '0' + (zone / 10);
            log_temp[7] = '0' + (zone % 10);
            log_temp[8] = '_';
            if (type == 0) {
                log_temp[9] = 'F'; log_temp[10] = 'I'; log_temp[11] = 'R'; log_temp[12] = 'E';
                k = 13;
            } else {
                log_temp[9] = 'I'; log_temp[10] = 'N'; log_temp[11] = 'T'; log_temp[12] = 'R';
                k = 13;
            }
            log_temp[k++] = '_';
            log_temp[k++] = '0' + (h12 / 10);
            log_temp[k++] = '0' + (h12 % 10);
            log_temp[k++] = '-';
            log_temp[k++] = '0' + (minute / 10);
            log_temp[k++] = '0' + (minute % 10);
            log_temp[k++] = '_';
            if (hour >= 12) {
                log_temp[k++] = 'P'; log_temp[k++] = 'M';
            } else {
                log_temp[k++] = 'A'; log_temp[k++] = 'M';
            }
            log_temp[k++] = '_';
            log_temp[k++] = '0' + (day / 10);
            log_temp[k++] = '0' + (day % 10);
            log_temp[k++] = '/';
            log_temp[k++] = '0' + (month / 10);
            log_temp[k++] = '0' + (month % 10);
            log_temp[k++] = '/';
            log_temp[k++] = '0' + (year / 10);
            log_temp[k++] = '0' + (year % 10);
            log_temp[k++] = '\0';

            serial_string(log_temp);
            serial1_string(log_temp);
            serial1_putc(0x0D);
            serial1_putc(0x0A);
            delay(10);
        }
    } else if (strncmp((const char*)rcv_buffer, "HLT", 3) == 0) {
        unsigned int hol_idx;
        unsigned int hol_addr;
        unsigned char day, month, year;
        char hol_temp[20];

        for (hol_idx = 1; hol_idx <= 30; hol_idx++) {
            hol_addr = ADD_HOLIDAY_BASE + (hol_idx - 1) * 3;
            day = Eeprom_ReadByte(hol_addr);
            if (day == 0xFF) continue;

            month = Eeprom_ReadByte(hol_addr + 1);
            year = Eeprom_ReadByte(hol_addr + 2);

            hol_temp[0] = 'H';
            hol_temp[1] = '0' + (hol_idx / 100);
            hol_temp[2] = '0' + ((hol_idx / 10) % 10);
            hol_temp[3] = '0' + (hol_idx % 10);
            hol_temp[4] = '_';
            hol_temp[5] = '0' + (day / 10);
            hol_temp[6] = '0' + (day % 10);
            hol_temp[7] = '/';
            hol_temp[8] = '0' + (month / 10);
            hol_temp[9] = '0' + (month % 10);
            hol_temp[10] = '/';
            hol_temp[11] = '0' + (year / 10);
            hol_temp[12] = '0' + (year % 10);
            hol_temp[13] = '\0';

            serial_string(hol_temp);
            serial1_string(hol_temp);
            serial1_putc(0x0D);
            serial1_putc(0x0A);
            delay(10);
        }
    } else if (strncmp((const char*)rcv_buffer, "GetFireNo", 9) == 0) {
        unsigned int ph_idx;
        unsigned int ph_addr;
        unsigned char k, m, is_blank;
        char ph_temp[30];

        for (ph_idx = 0; ph_idx < 10; ph_idx++) {
            ph_addr = ADD_FIRE_NUM_BASE + (ph_idx * 16);
            
            ph_temp[0] = '$';
            ph_temp[1] = 'P';
            ph_temp[2] = '0' + ph_idx;
            ph_temp[3] = ':';
            
            is_blank = 1;
            for (m = 0; m < 16; m++) {
                k = Eeprom_ReadByte(ph_addr + m);
                if (k != 0xFF && k != 0x00 && k != ' ') {
                    is_blank = 0;
                    break;
                }
            }

            if (is_blank) {
               // ph_temp[4] = '\\';
              //  ph_temp[5] = '0';
                ph_temp[4] = '\0';
            } else {
                for (m = 0; m < 16; m++) {
                    k = Eeprom_ReadByte(ph_addr + m);
                    if (k == 0xFF || k == 0x00) k = ' ';
                    ph_temp[4 + m] = k;
                }
                ph_temp[20] = '\0';
                // Trim trailing spaces
                for (m = 19; m >= 4; m--) {
                    if (ph_temp[m] == ' ') {
                        ph_temp[m] = '\0';
                    } else {
                        break;
                    }
                }
            }

            serial1_string(ph_temp);
            //serial1_putc(0x0D);
            //serial1_putc(0x0A);
            delay(200);
        }
    } else if (strncmp((const char*)rcv_buffer, "GetIntrNo", 9) == 0) {
        unsigned int ph_idx;
        unsigned int ph_addr;
        unsigned char k, m, is_blank;
        char ph_temp[30];

        for (ph_idx = 0; ph_idx < 10; ph_idx++) {
            ph_addr = ADD_INTR_NUM_BASE + (ph_idx * 16);
            
            ph_temp[0] = '$';
            ph_temp[1] = 'P';
            ph_temp[2] = '0' + ph_idx;
            ph_temp[3] = ':';
            
            is_blank = 1;
            for (m = 0; m < 16; m++) {
                k = Eeprom_ReadByte(ph_addr + m);
                if (k != 0xFF && k != 0x00 && k != ' ') {
                    is_blank = 0;
                    break;
                }
            }

            if (is_blank) {
               // ph_temp[4] = '\\';
              //  ph_temp[5] = '0';
                ph_temp[4] = '\0';
            } else {
                for (m = 0; m < 16; m++) {
                    k = Eeprom_ReadByte(ph_addr + m);
                    if (k == 0xFF || k == 0x00) k = ' ';
                    ph_temp[4 + m] = k;
                }
                ph_temp[20] = '\0';
                // Trim trailing spaces
                for (m = 19; m >= 4; m--) {
                    if (ph_temp[m] == ' ') {
                        ph_temp[m] = '\0';
                    } else {
                        break;
                    }
                }
            }

            serial1_string(ph_temp);
            //serial1_putc(0x0D);
            //serial1_putc(0x0A);
            delay(200);
        }
    }else if (rcv_buffer[0] == 'Z' && (rcv_buffer[1] >= '1' && rcv_buffer[1] <= '8')) {
        SetZoneMode(rcv_buffer[1] - '1', rcv_buffer[2], 1);
    }
    rcv_buffer[0] = '\0';
}
#endif
//#endif