/*****************************************************
  This program was produced by the
  CodeWizardAVR V2.04.4a Advanced
  Automatic Program Generator
  � Copyright 1998-2009 Pavel Haiduc, HP InfoTech s.r.l.
  http://www.hpinfotech.com
  Project :
  Version :
  Date    : 18-02-2021
  Author  : Debajyoti Roy
  Company : Security Engineers Pvt Ltd.
  Comments:
  Chip type               : ATmega32
  Program type            : Application
  AVR Core Clock frequency: 8.000000 MHz
  Memory model            : Small
  External RAM size       : 0
  Data Stack size         : 1024
*****************************************************/

/*****************************************************
  This program was produced by the
  CodeWizardAVR V2.04.4a Advanced
  Automatic Program Generator
  � Copyright 1998-2009 Pavel Haiduc, HP InfoTech s.r.l.
  http://www.hpinfotech.com
  Project :
  Version : 1.5T 4G
  Date    : 18-01-2021
  Author  : Claude 3.5 Sonnet
  Company : Security Engineers Pvt Ltd.
  Comments:
  Chip type               : ATmega1284P
  Program type            : Application
  AVR Core Clock frequency: 8.000000 MHz
  Memory model            : Small
  External RAM size       : 0
  Data Stack size         : 1024
*****************************************************/
#define F_CPU 8000000UL
#include <util/delay.h>
#include "main.h"
//#include <avr/interrupt.h>
//#include <avr/wdt.h>


unsigned char zone_led_state[24] = {0};  // Track LED state for each zone
unsigned char zone_prev_elapsed[24] = {0};  // Track previous elapsed time
unsigned char zone_activated_time[24] = {0};  // Store the activation start time
unsigned int zone_total_elapsed[24] = {0};  // Track TOTAL elapsed seconds across minute boundaries
unsigned char zone_init_flag[24] = {0};  // NEW: Flag to track if already initialized (1 = yes, 0 = no)
unsigned char current_elapsed_in_minute;

unsigned char c_hour, c_min, c_sec, c_day, c_month, c_year;
unsigned char c_arm_status;
unsigned char c_name[17];
unsigned char name_refresh_flag = 1;
int zone_mode_change=0;
char DataForSend[16];
char rcv[50];
//==========================================================


void send_hooter_cut_status(void) {
  static unsigned char prev_hooter_state = 2;  // Initialize with invalid state
  unsigned char current_hooter_state;
  char status_message[32];
  unsigned char i;

  current_hooter_state = (PINA & (1 << PA1)) ? 1 : 0; // Read HOOTER_CUT pin

  // Only send if hooter state has changed
  if (current_hooter_state != prev_hooter_state) {
    if (current_hooter_state) { // HOOTER_CUT pin is high
      strcpy(status_message, "HOOTER:CUT_\r\n");
      l_siren_cut = 0;
    } else {
      strcpy(status_message, "HOOTER:NORM\r\n");
      l_siren_cut = 1;
    }
    RS485_EN();
    // Send the status through serial port
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();
    prev_hooter_state = current_hooter_state; // Update previous state
  }

}


//==========================================================

void send_m_hooter_cut_status(void) {

  static unsigned char prev_m_hooter_state = 2;  // Initialize with invalid state
  unsigned char current_m_hooter_state;
  char status_message[32];
  unsigned char i;

  current_m_hooter_state = (PINA & (1 << PA4)) ? 1 : 0;// Read HOOTER_CUT pin

  // Only send if hooter state has changed
  if (current_m_hooter_state != prev_m_hooter_state) {
    if (current_m_hooter_state) { // M. HOOTER_CUT pin is high
      strcpy(status_message, "M.SIREN:NORM\n");
      l_m_siren_cut = 0;
    } else {
      strcpy(status_message, "M.SIREN:CUT_\n");
      l_m_siren_cut = 1;
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();
    prev_m_hooter_state = current_m_hooter_state; // Update previous state
  }
}


/*=========================================================*/

void send_power_status(void) {
  static unsigned char prev_power_state = 2;  // Using unsigned char instead of uint8_t
  unsigned char current_power_state;
  char status_message[32];
  unsigned char i;  // Declare loop variable at start

  //current_power_state = PINA.3; // Read MAIN_ON pin
  current_power_state = (PINA & (1 << PA3)) ? 1 : 0;

  // Only send if power state has changed
  if (current_power_state != prev_power_state) {
    if (current_power_state) {                      // MAIN_ON pin is low = AC power lost
      strcpy(status_message, "$POWER:BATT\n");
      l_mains = 0;
      l_battery = 1;
    } else {                                        // MAIN_ON pin is high = AC power present
      strcpy(status_message, "$POWER:MAIN\n");
      l_mains = 1;
      l_battery = 0;
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();

    prev_power_state = current_power_state; // Update previous state
  }
}

//===========================================================
void send_battery_cut_status(void) {
  static unsigned char prev_battery_cut_state = 2;  // Initialize with invalid state
  char status_message[32];
  unsigned char i;

  // Check if battery cut LED state has changed
  if (l_battery_cut != prev_battery_cut_state) {
    if (l_battery_cut) {
      strcpy(status_message, "$BAT:CUT_\n");
    } else {
      strcpy(status_message, "$BAT:CONN\n");
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();
    prev_battery_cut_state = l_battery_cut;
  }
}
/*=========================================================*/

void send_battery_low_status(void) {
  static unsigned char prev_battery_low_state = 2;  // Initialize with invalid state
  static unsigned char init_done = 0;  // Flag for initialization
  char status_message[32];
  unsigned char i;

  // On first run, just update previous state without sending message
  if (!init_done) {
    prev_battery_low_state = l_battery_low;
    init_done = 1;
    return;
  }

  if (!l_battery_cut && (l_battery_low != prev_battery_low_state)) {
    if (l_battery_low) { // Battery low
      strcpy(status_message, "$BATSTAT:LOW_\n");
    } else {  // Battery normal
      strcpy(status_message, "$BATSTAT:NORM\n");
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();

    prev_battery_low_state = l_battery_low;
  }
}

//=========================================================
void send_arm_status(void) {

  static unsigned char prev_arm_state = 2;  // Initialize with invalid state
  unsigned char current_arm_state;
  char status_message[32];
  unsigned char i;

  current_arm_state = l_armed;  // Using l_armed status variable

  // Only send if arm state has changed
  if (current_arm_state != prev_arm_state) {
    if (current_arm_state) { // System is armed
      strcpy(status_message, "$SYS:ARM_\n");
      l_armed = 1;
      l_disarmed = 0;
    } else {  // System is disarmed
      strcpy(status_message, "$SYS:DARM\n");
      l_armed = 0;
      l_disarmed = 1;
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();

    prev_arm_state = current_arm_state; // Update previous state
  }
}

//==========================================================


void send_daynight_status(void) {
  static unsigned char prev_daynight_state = 2;  // Initialize with invalid state
  unsigned char current_daynight_state;
  char status_message[32];
  unsigned char i;

  current_daynight_state = l_day_mode;  // Using l_day_mode as current state

  // Only send if day/night state has changed
  if (current_daynight_state != prev_daynight_state) {
    if (current_daynight_state) { // Day mode
      strcpy(status_message, "$MODE:NIGT\n");
      l_day_mode = 1;
      l_night_mode = 0;
    } else {  // Night mode
      strcpy(status_message, "$MODE:DAYS\n");
      l_day_mode = 0;
      l_night_mode = 1;
    }

    // Send the status through serial port
    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();
    prev_daynight_state = current_daynight_state; // Update previous state
  }
}


//==========================================================

void send_battery_voltage(void) {
  static unsigned char last_day_bat = 0;
  static unsigned char power_on_sent = 0;
  char status_message[32];
  unsigned int bat_adc;
  unsigned long bat_v;
  unsigned char i;

  // Send if first time after power on OR if day has changed
  if (!power_on_sent || (c_day != last_day_bat)) {
    bat_adc = read_bat_volt();
    // Voltage = ADC * (5.0 / 1024) * 4 (divider) = ADC * 0.01953125
    // To get xx.xx format: Voltage * 100 = ADC * 1.953125
    // 1.953125 approx 2000 / 1024
    bat_v = ( (unsigned long)bat_adc * 2000L ) / 1024L;

    sprintf(status_message, "$BAT_VOLT:%02u.%02uV\n", (unsigned int)(bat_v / 100), (unsigned int)(bat_v % 100));

    RS485_EN();
    for (i = 0; status_message[i] != '\0'; i++) {
      serial_putc(status_message[i]);
      serial1_putc(status_message[i]);
    }
    RS485_DN();

    last_day_bat = c_day;
    power_on_sent = 1;
  }
}

//==========================================================

void send_tamper_status(void) {
  static unsigned char prev_tamper_state = 2;  // Initialize with invalid state
  unsigned char current_tamper_state;
  char status_message[32];
  unsigned char i;

  current_tamper_state = tamper_flag;  // Using tamper_flag as current state
  _delay_ms(1000);
  // Only send if tamper state has changed
  if (current_tamper_state != prev_tamper_state) {
    if (current_tamper_state) { // Tamper detected
      strcpy(status_message, "$TAMP:ACTV\n");
    } else {  // Tamper normal
      strcpy(status_message, "$TAMP:NORM\n");
    }

    // Send the status through serial port
    RS485_EN();
//    for (i = 0; status_message[i] != '\0'; i++) {
//      serial_putc(status_message[i]);
//      serial1_putc(status_message[i]);
//    }
      serial_string(status_message);
      serial1_string(status_message);
      _delay_ms(1000);
    RS485_DN();
    prev_tamper_state = current_tamper_state; // Update previous state
  }
}

//==========================================================

void send_initial_zone_status(void) {
  unsigned char i;
  unsigned char type;
  RS485_EN();

  for (i = 0; i < 8; i++) {
    // Send zone header
    serial1_string("$ZONE");
    serial1_putc('1' + i);
    serial1_putc(':');

    // Get zone type from EEPROM and send appropriate type
    type = Eeprom_ReadByte(ADD_ZONE_1_TYPE + i);
    switch (type) {
      case TYPE_FIRE:
        serial1_string_f("FIRE");
        break;
      case TYPE_DAY:
        serial1_string_f("DAYS");
        break;
      case TYPE_NIGHT:
        serial1_string_f("NIGT");
        break;
      default:
        serial1_string_f("ISOL");
        break;
    }

    // Send line ending
    serial1_putc('\r');
    serial1_putc('\n');
    RS485_DN();
    // Add a small delay between messages
    delay(1);
  }
}

                                                                            
void send_zone_status(void) {
  static unsigned char prev_zone_states[8] = {255, 255, 255, 255, 255, 255, 255, 255};
  unsigned char zone;
  char msg[20];  // Enough room for messages
  RS485_EN();

  for (zone = 0; zone < 8; zone++) {
    // Send message ONLY if state actually changed
    if (zonestates[zone] != prev_zone_states[zone]) {
      RS485_EN();
      // Clear buffer
      msg[0] = 'Z';
      msg[1] = '1' + zone;   // Zone number (Z1..Z8)
      msg[2] = ':';

      // Choose state text manually to avoid strcat/string overlap problems
      switch (zonestates[zone]) {
        case STATE_NORMAL:
          msg[3] = 'N';
          msg[4] = 'O';
          msg[5] = 'R';
          msg[6] = 'M';
          msg[7] = '\r';
          msg[8] = '\0';
          break;

        case STATE_ACTIVATED:
          msg[3] = 'A';
          msg[4] = 'C';
          msg[5] = 'T';
          msg[6] = 'V';
          msg[7] = '\r';
          msg[8] = '\0';
          break;

        case STATE_TRIG:
          msg[3] = 'T';
          msg[4] = 'R';
          msg[5] = 'I';
          msg[6] = 'G';
          msg[7] = '\r';
          msg[8] = '\0';
          break;

        case STATE_OPEN:
          msg[3] = 'O';
          msg[4] = 'P';
          msg[5] = 'E';
          msg[6] = 'N';
          msg[7] = '\r';
          msg[8] = '\0';
          break;

        case STATE_SHORT:
          msg[3] = 'S';
          msg[4] = 'H';
          msg[5] = 'R';
          msg[6] = 'T';
          msg[7] = '\r';
          msg[8] = '\0';
          break;

        case STATE_ISO:
          msg[3] = 'I';
          msg[4] = 'S';
          msg[5] = 'O';
          msg[6] = 'L';
          msg[7] = '\r';
          msg[8] = '\0';
          break;
      }

      // Send string safely
      //LcdSetCursor(1,0);LcdSetCursor(1,0);125937203serial_string(msg);
      serial1_string(msg);
      RS485_DN();
      delay(10);  // Small delay to ensure message is sent before next one
      
      // Update previous state
      prev_zone_states[zone] = zonestates[zone];
    }
  }
  RS485_DN();
}


// ==========================================================================

void DefaultSettings()
{
  unsigned int i;
  Eeprom_WriteByte(ADD_NIGHT_TRIG, 0);

  // Zone type settings
  Eeprom_WriteByte(ADD_ZONE_1_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_2_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_3_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_4_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_5_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_6_TYPE, TYPE_NIGHT);
  Eeprom_WriteByte(ADD_ZONE_7_TYPE, TYPE_NIGHT);
  Eeprom_WriteByte(ADD_ZONE_8_TYPE, TYPE_FIRE);
  Eeprom_WriteByte(ADD_ZONE_9_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_10_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_11_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_12_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_13_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_14_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_15_TYPE, TYPE_DAY);
  Eeprom_WriteByte(ADD_ZONE_16_TYPE, TYPE_DAY);

  // Zone delay settings
  Eeprom_WriteByte(ADD_ZONE_1_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_2_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_3_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_4_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_5_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_6_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_7_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_8_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_9_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_10_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_11_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_12_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_13_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_14_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_15_DELAY, 0);
  Eeprom_WriteByte(ADD_ZONE_16_DELAY, 0);

  // Time settings
  Eeprom_WriteByte(ADD_ENTRY_DELAY, 20);
  Eeprom_WriteByte(ADD_EXIT_DELAY, 20);
  Eeprom_WriteByte(ADD_DAY_HOUR, 10);
  Eeprom_WriteByte(ADD_DAY_MINUTE, 0);
  Eeprom_WriteByte(ADD_NIGHT_HOUR, 18);
  Eeprom_WriteByte(ADD_NIGHT_MINUTE, 0);
  Eeprom_WriteByte(ADD_ARM_HOUR, 10);
  Eeprom_WriteByte(ADD_ARM_MINUTE, 0);
  Eeprom_WriteByte(ADD_DISARM_HOUR, 18);
  Eeprom_WriteByte(ADD_DISARM_MINUTE, 0);

  // System settings
  Eeprom_WriteByte(ADD_DAY_NIGHT_TYPE, 0);    // Auto mode
  Eeprom_WriteByte(ADD_ARM_DISARM, 0);        // Disarmed
  Eeprom_WriteByte(ADD_CMS_EN, 0);            // CMS disabled
  Eeprom_WriteByte(ADD_RPTR_EN, 0);           // Repeater disabled

  for (i = 0; i < 160; i++) {
    Eeprom_WriteByte(ADD_FIRE_NUM_BASE + i, ' ');
  }
  for (i = 0; i < 32; i++) {
    Eeprom_WriteByte(ADD_EMAIL_ID_BASE + i, ' ');
  }
}

//==========================================================
void Reset()
{
  //RS485_EN();
  //serial_string_f("RESET");
  //serial1_string_f("RESET");
  //RS485_DN();
  //WDTCSR = 0x18; /// enable watchdog
  //while (1); /// loop until reset
  RS485_EN();

  serial_string_f("RESET");

  //while (!(UCSR0A & (1 << TXC0)));

  _delay_ms(20);

  RS485_DN();

  wdt_enable(WDTO_15MS);

  while(1);
}

// ================================================= Factory Reset ======================================================

void FactoryReset(void)
{
  // Clear LCD and show message
  lcd_clear();
  lcd_string_f("  FACTORY RESET  ");
  lcd_cmd(0xC0);  // Move to second line
  lcd_string_f("   IN PROGRESS   ");
  _delay_ms(3000);

  DefaultSettings();
  // Show completion message
  lcd_clear();
  lcd_string_f("  FACTORY RESET  ");
  lcd_cmd(0xC0);  // Move to second line
  lcd_string_f("    COMPLETE     ");

  delay(2000);  // Show message for 2 seconds

  // System reset after factory reset
  Reset();
}

// =============================================================================================
//void ProcessDayNightTime(char *cmd) {
//    unsigned char hour, minute;
//
//    // Extract hours and minutes from the string
//    hour = ((cmd[1] - '0') * 10) + (cmd[2] - '0');
//    minute = ((cmd[3] - '0') * 10) + (cmd[4] - '0');
//
//    // Validate time values
//    if(hour > 23 || minute > 59) {
//        return; // Invalid time
//    }
//
//    // Check if it's Day or Night setting
//    if(cmd[0] == 'D') {
//        Eeprom_WriteByte(ADD_DAY_HOUR, hour);
//        Eeprom_WriteByte(ADD_DAY_MINUTE, minute);
//    }
//    else if(cmd[0] == 'N') {
//        Eeprom_WriteByte(ADD_NIGHT_HOUR, hour);
//        Eeprom_WriteByte(ADD_NIGHT_MINUTE, minute);
//    }
//
//}

void SetZoneMode(unsigned char zone, char mode_char, unsigned char uart_num) {
  unsigned char zoneAddr;
  if (zone > 7) return; // Only zones 1-8 supported (index 0-7)
  
  zoneAddr = ADD_ZONE_1_TYPE + zone;

  switch (mode_char) {
    case 'F': // Fire Zone
      Eeprom_WriteByte(zoneAddr, TYPE_FIRE);
      break;

    case 'D': // Day Zone
      Eeprom_WriteByte(zoneAddr, TYPE_DAY);
      break;

    case 'N': // Night Zone
      Eeprom_WriteByte(zoneAddr, TYPE_NIGHT);
      break;

    case 'I': // Isolated Zone
      Eeprom_WriteByte(zoneAddr, TYPE_ISO);
      zonestates[zone] = STATE_ISO;
      break;
    default:
      return; // Invalid mode
  }
  
  // Return the same zone mode status to the same UART
  RS485_EN();
  if (uart_num == 0) {
      serial_putc('Z');
      serial_putc('1' + zone);
      serial_putc(mode_char);
      serial_putc('\r');
      serial_putc('\n');
  } else {
      serial1_putc('Z');
      serial1_putc('1' + zone);
      serial1_putc(mode_char);
      serial1_putc('\r');
      serial1_putc('\n');
  }
  RS485_DN();

  send_ok(uart_num);
}

//==========================================================

void send_ok(unsigned char uart_num) {
  RS485_EN();
  if (uart_num == 0) {
      serial_putc('O');
      serial_putc('K');
      serial_putc('\r');
      serial_putc('\n');
  } else {
      serial1_putc('O');
      serial1_putc('K');
      serial1_putc('\r');
      serial1_putc('\n');
  }
  RS485_DN();
  delay(10);  // Keep delay after sending response
}
// ========================================= UART0 Manual Data Receive Without Interrupt ===================================================

/*void check_serial(void) {
  static char cmd_buffer[4]; 
  static char rcv[50] ;
  static unsigned char cmd_index = 0;

  // Check if data is available
  if (UCSR0A & (1 << RXC0)) {
    char c = read_serial_char();
    
    // Buffer for multi-character commands like Z1F
//    cmd_buffer[cmd_index++] = c;
     rcv[cmd_index++]=c;
    if (cmd_index >= 3) {
      if (cmd_buffer[0] == 'Z' && (cmd_buffer[1] >= '1' && cmd_buffer[1] <= '8')) {
         SetZoneMode(cmd_buffer[1] - '1', cmd_buffer[2], 0);
         cmd_index = 0;
         return;
      }
    }
    if (GetRPTREnabled()) {
      switch (c) {
        case '1':
          send_ok(0);
          Reset();
          cmd_index = 0;
          break;

        case '2':
          // ARM the system
          Eeprom_WriteByte(ADD_ARM_DISARM, 1);  // Set armed state
          l_armed = 1;
          l_disarmed = 0;
          send_ok(0);
          cmd_index = 0;
          break;

        case '3':
          // DISARM the system
          Eeprom_WriteByte(ADD_ARM_DISARM, 0);  // Set disarmed state
          l_armed = 0;
          l_disarmed = 1;
          Sounder_Silent();
          trigger_reset_flag = 0;
          sounder_time_flag = 0;
          send_ok(0);
          cmd_index = 0;
          break;

        case '4':
          // Silent the system
          Sounder_Silent();
          send_ok(0);
          cmd_index = 0;
          break;
        
        default:
          if (cmd_index >= 3) cmd_index = 0; // Reset if no match
          break;
      }
    } else {
        // RPTR disabled, still reset cmd_index if it exceeds 3 to prevent overflow/stuck
        if (cmd_index >= 3) cmd_index = 0;
    }
  }
}*/

void check_serial(void) {

        // ? static important
    static unsigned char cmd_index = 0;

    RS485_DN();

    while (UCSR0A & (1 << RXC0)) {

        char c = UDR0;

        // End of string
        if (c == '\r' || c == '\n' || c == '\0') {

            if (cmd_index > 0) {

                rcv[cmd_index] = '\0';

                // ? process here
                
            }

            cmd_index = 0;   // reset AFTER processing
        }

        else {

            if (cmd_index < sizeof(rcv) - 1) {

                rcv[cmd_index++] = c;
            }
            else {

                cmd_index = 0;  // overflow reset
            }
        }
    }
}


// ====================================================== Serial Interrupts ========================================================

//interrupt [USART0_RXC] void usart0_rx_isr(void)
//{
//    char received_byte = 0;
//
//    // Read received data
//    received_byte = UDR0;
//
//    // Wait until transmit buffer empty
//    while (!(UCSR0A & (1 << UDRE0)));
//}
//

// =================================================================================================================================

void delay(unsigned int a)
{
  unsigned int i, j;

  for (i = 0; i < 1250; i++) {
    for (j = 0; j < a; j++);
  }
}

void Sounder_Silent()
{
  trigger_flag = 0;
  sounder_flag = 0;
  int_hoot(0);
  hooter_trig(0);
  relay1_trig(0);
  relay2_trig(0);
}

unsigned char GetKey(void)
{
  unsigned char rawKey = Keypad_GetKey();

  if (rawKey != 0xFF) {
    switch (rawKey) {
      case '0': return '0';
      case '1': return '1';
      case '2': return '2';
      case '3': return '3';
      case '4': return '4';
      case '5': return '5';
      case '6': return '6';
      case '7': return '7';
      case '8': return '8';
      case '9': return '9';
      case '*': return '*';
      case '#': return '#';
      case 'A': return 'A';
      case 'B': return 'B';
      case 'C': return 'C';
      case 'D': return 'D';
    }
  }
  return KEY_NONE;
}


unsigned char ReadDayNightKey(void)
{
  if (DAY_NIGHT_KEY == 0) {
    _delay_ms(100);
    return 1;
  } else
    return 0;
}

unsigned char Read_Tamper_key()
{
  if (TAMP_SWITCH == 0)
  {
    return 1;
  }
  else
    return 0;
}///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//unsigned char SearchNumber(char *number )
//{
//    unsigned char i,j;
//  char buff[17];
//
//  unsigned char len1 = 0, a = 0, len2 = 0, b = 0;
//  len1 = strlen(number);
//
//  if(len1<10){
//    return 0;
//  }
//  a = len1 - 10;
//  for( i = 0; i < 30; i++ ){
////    GetNumber(i,buff);
//    buff[16] = '\0';
//    for( j = 0; j < 16; j++ ){
//      if( !isdigit(buff[j] ) ){
//        buff[j]='\0';
//        break;
//      }
//    }
//    len2 = strlen(buff);
//    if( len2 < 10 ){
//      continue;
//    }
//    b = len2-10;
//    if(strcmp(&number[a],&buff[b])==0){
//      return 1;
//    }
//  }
//  return 0;
//}

unsigned char GetZoneSettings(unsigned char zone) // 1->8
{
  unsigned char type;
  type = Eeprom_ReadByte( ADD_ZONE_1_TYPE + zone - 1 );
  if ( type > 3 ) {
    type = 0;
  }
  return type;
}

void lcd_back(unsigned char);

void NightTriggerDlg()
{
  unsigned char a, change = 1;
  a = Eeprom_ReadByte(ADD_NIGHT_TRIG);
  if (a != 0) {
    a = 1;
  }
  lcd_clear();
  lcd_string_f(" NIGHT CUT TRIG ");

  while (1) {
    if (change == 1) {
      lcd_cmd(0xC0);
      lcd_clear_line();
      lcd_cmd(0xC0);
      lcd_string_f(a == 0 ? "    DISABLED" : "    ENABLED");
      change = 0;
    }
    if (GetKey() == KEY_UP && a == 1) {
      a = 0;
      change = 1;
    }
    if (GetKey() == KEY_DOWN && a == 0) {
      a = 1;
      change = 1;
    }
    if (GetKey() == KEY_MENU) {
      break;
    }
    if (GetKey() == KEY_ENTER) {
      Eeprom_WriteByte(ADD_NIGHT_TRIG, a);
      break;
    }
  }
  lcd_clear();
}


void SetTimeDlg(unsigned char type) //1 = data/time, 2 = day time, 3 =night time,4 = arm time, 5= disarm time
{
  unsigned int hour = 0, minute = 0, day = 0, month = 0, year = 0;
  lcd_clear();
  lcd_cmd(0x84);

  switch (type)
  {
    case 1:
      lcd_string_f("DATE/TIME");
      break;
    case 2:
      lcd_string_f("DAY TIME");
      break;
    case 3:
      lcd_string_f("NIGHT TIME");
      break;
    case 4:
      lcd_string_f("ARM TIME");
      break;
    case 5:
      lcd_string_f("DISARM TIME");
      break;
    default:
      return;
  }
  //delay(500);
  ////////////////////////////////////////////////////////////////////////// HOUR /////////////////////////////////////////////////////////////////
  lcd_cmd(0xC0);
  if ( type == 1 ) {
    hour = 0;
    minute = 0;
  } else if ( type == 2 ) {
    hour = Eeprom_ReadByte(ADD_DAY_HOUR);
    minute = Eeprom_ReadByte(ADD_DAY_MINUTE);
  } else if ( type == 3 ) {
    hour = Eeprom_ReadByte(ADD_NIGHT_HOUR);
    minute = Eeprom_ReadByte(ADD_NIGHT_MINUTE);
  } else if ( type == 4 ) {
    hour = Eeprom_ReadByte(ADD_ARM_HOUR);
    minute = Eeprom_ReadByte(ADD_ARM_MINUTE);
  } else if ( type == 5 ) {
    hour = Eeprom_ReadByte(ADD_DISARM_HOUR);
    minute = Eeprom_ReadByte(ADD_DISARM_MINUTE);
  }
  if ( hour == 0xFF || minute == 0xFF ) {
    hour = 0;
    minute = 0;
  }
  lcd_string("HOUR : ");  
  LcdSetCursor(1, 7);
  lcd_num(hour);
  lcd_clear_line();

  while (1) {
    if ( GetKey() == KEY_UP ) {
      if ( hour < 23 ) {
        hour++;
        lcd_cmd(0xC7);
        lcd_num(hour);
      }
    }

    if ( GetKey() == KEY_DOWN ) {
      if ( hour > 0 ) {
        hour--;
        lcd_cmd(0xC7);
        lcd_num(hour);
      }
    }

    if ( GetKey() == KEY_MENU ) {
      delay(50);
      lcd_clear();
      return;
    }

    if ( GetKey() == KEY_ENTER ) {
      break;
    }
    delay(20);
  }
  /////////////////////////////////////////////////////////// MINUTE ///////////////////////////////////////////////////////
  delay(100);
  lcd_cmd(0xC0);
  lcd_string_f("MINUTE : ");
  LcdSetCursor(1, 9);
  lcd_num(minute);
  lcd_clear_line();

  while (1) {
    if ( GetKey() == KEY_UP ) {
      if ( minute < 59 ) {
        minute++;
        lcd_cmd(0xC9);
        lcd_num(minute);
      }
    }

    if ( GetKey() == KEY_DOWN ) {
      if ( minute > 0 ) {
        minute--;
        lcd_cmd(0xC9);
        lcd_num(minute);
      }
    }

    if ( GetKey() == KEY_MENU ) {
      delay(50);
      lcd_clear();
      return;
    }

    if ( GetKey() == KEY_ENTER ) {
      break;
    }

    delay(20);
  }

  if ( type == 1 ) {
    ///////////////////////////////////////////////////////// DAY ///////////////////////////////////////////////////////////////////
    delay(100);
    lcd_cmd(0xC0);
    day = 1;
    lcd_string_f("DAY : ");
    lcd_num(day);
    lcd_clear_line();

    while (1) {
      if ( GetKey() == KEY_UP ) {
        if (day < 31) {
          day++;
          lcd_cmd(0xC6);
          lcd_num(day);
        }
      }
      if ( GetKey() == KEY_DOWN ) {
        if (day > 1) {
          day--;
          lcd_cmd(0xC6);
          lcd_num(day);
        }
      }
      if ( GetKey() == KEY_MENU ) {
        delay(50);
        lcd_clear();
        return;
      }
      if ( GetKey() == KEY_ENTER ) {
        break;
      }
      delay(20);
    }
    /////////////////////////////////////////////////////////////////////// MONTH /////////////////////////////////////////////////////////////////////
    delay(100);
    lcd_cmd(0xC0);

    month = 1;
    lcd_string_f("MONTH : ");
    lcd_num(month);
    lcd_clear_line();

    while (1) {
      if ( GetKey() == KEY_UP ) {
        if ( month < 12 ) {
          month++;
          lcd_cmd(0xC8);
          lcd_num(month);
        }
      }

      if ( GetKey() == KEY_DOWN ) {
        if ( month > 1 ) {
          month--;
          lcd_cmd(0xC8);
          lcd_num(month);
        }
      }

      if ( GetKey() == KEY_MENU ) {
        delay(50);
        lcd_clear();
        return;
      }

      if ( GetKey() == KEY_ENTER ) {
        break;
      }
      delay(20);
    }

    ///////////////////////////////////////////////// YEAR //////////////////////////////////////////////
    delay(100);
    lcd_cmd(0xC0);
    year = 18;
    lcd_string_f("YEAR : 20");
    lcd_num(year);
    lcd_clear_line();

    while (1) {
      if ( GetKey() == KEY_UP ) {
        if (year < 99) {
          year++;
          lcd_cmd(0xC9);
          lcd_num(year);
        }
      }
      if ( GetKey() == KEY_DOWN ) {
        if ( year > 12 ) {
          year--;
          lcd_cmd(0xC9);
          lcd_num(year);
        }
      }

      if ( GetKey() == KEY_MENU ) {
        delay(50);
        lcd_clear();
        return;
      }

      if ( GetKey() == KEY_ENTER ) {
        break;
      }
      delay(20);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  }

  if ( type == 1 ) { /// set time
    RTC_SetHour(hour);
    RTC_SetMinute(minute);
    RTC_SetSecond(0x00);
    RTC_SetDay(day);
    RTC_SetMonth(month);
    RTC_SetYear(year);
  } else if ( type == 2 ) { //// set day time
    Eeprom_WriteByte(ADD_DAY_HOUR, hour);
    Eeprom_WriteByte(ADD_DAY_MINUTE, minute);
  } else if ( type == 3 ) { /// set night time
    Eeprom_WriteByte(ADD_NIGHT_HOUR, hour);
    Eeprom_WriteByte(ADD_NIGHT_MINUTE, minute);
  } else if ( type == 4 ) { /// set arm time
    Eeprom_WriteByte(ADD_ARM_HOUR, hour);
    Eeprom_WriteByte(ADD_ARM_MINUTE, minute);
  } else if ( type == 5 ) { /// set disarm time
    Eeprom_WriteByte(ADD_DISARM_HOUR, hour);
    Eeprom_WriteByte(ADD_DISARM_MINUTE, minute);
  }
  delay(500);
  lcd_clear();
}

// Shift Register 74HC595 Driver
void SendLatchData(unsigned char latch, unsigned char data) // 1,2,3,4
{
  unsigned char i;
  for ( i = 0; i < 8; i++ ) {
    /*DS = ( data & 0x80 ) >> 7;
    SHCP = 0;
    //delay(1);
    SHCP = 1;
    //delay(1);
    data <<= 1;*/
	if((data & 0x80) >> 7)
	PORTB |= (1 << DS);
	else
	PORTB &= ~(1 << DS);

	PORTB &= ~(1 << SHCP);
	PORTB |= (1 << SHCP);

	data <<= 1;
  }

  switch (latch) {
    case 1:
      //LATCH_1 = 0;
	  PORTC &= ~(1<<LATCH_1);
      //delay(1);
      //LATCH_1 = 1;
	  PORTC |= (1<<LATCH_1);
      //delay(1);
      break;
    case 2:
      //LATCH_2 = 0;
	  PORTC &= ~(1<<LATCH_2);
      //delay(1);
      //LATCH_2 = 1;
       PORTC |= (1<<LATCH_2);
      //delay(1);
      break;
    case 3:
      //LATCH_3 = 0;
      PORTC &= ~(1<<LATCH_3);
	  //delay(1);
     // LATCH_3 = 1;
      PORTC |= (1<<LATCH_3);
	  //delay(1);
      break;
    case 4:
      //LATCH_4 = 0;
      PORTC &= ~(1<<LATCH_4);
	  //delay(1);
      //LATCH_4 = 1;
      PORTC |= (1<<LATCH_4);
	  //delay(1);
      break;
    default:
      break;
  }
}


void UpdateLatch()
{
  SendLatchData(1, latch1_data);
  SendLatchData(2, latch2_data);
  SendLatchData(3, latch3_data);
  SendLatchData(4, latch4_data);
}


void led_syson(unsigned char a)
{
  l_sys_on = a;
  if ( a == 0 ) {
    latch1_data &= 0xFE;
  } else {
    latch1_data |= 0x01;
  }
  UpdateLatch();
}

void led_mains(unsigned char a)
{
  l_mains = a;
  if ( a == 0 ) {
    latch1_data &= 0xFD;
  } else {
    latch1_data |= 0x02;
  }
  UpdateLatch();
}

void led_battery(unsigned char a)
{
  l_battery = a;
  if ( a == 0 ) {
    latch1_data &= 0xFB;
  } else {
    latch1_data |= 0x04;
  }
  UpdateLatch();
}

void led_battery_cut(unsigned char a)
{
  l_battery_cut = a;
  if ( a == 0 ) {
    latch1_data &= 0xF7;
  } else {
    latch1_data |= 0x08;
  }
  UpdateLatch();
}


void led_battery_low(unsigned char a)
{
  l_battery_low = a;
  if ( a == 0 ) {
    latch1_data &= 0xEF;
  } else {
    latch1_data |= 0x10;
  }
  UpdateLatch();
}


void led_battery_charging(unsigned char a)
{
  l_battery_charging = a;
  l_battery_charging = a;
  if ( a == 0 ) {
    latch1_data &= 0xDF;
  } else {
    latch1_data |= 0x20;
  }
  UpdateLatch();
}


void led_hoot_cut(unsigned char a)
{
  l_siren_cut = a;
  if ( a == 0 ) {
    latch1_data &= 0xBF;
    send_hooter_cut_status();
  } else {
    latch1_data |= 0x40;
    send_hooter_cut_status();
  }
  UpdateLatch();
}


void led_mot_sir_cut(unsigned char a)
{
  l_m_siren_cut = a;
  if (a == 0) {
    latch1_data &= 0x7F;
    send_m_hooter_cut_status();
  } else {
    latch1_data |= 0x80;
    send_m_hooter_cut_status();
  }
  UpdateLatch();
}


void led_zone(unsigned char zone, unsigned char a)
{
  if ( zone >= 1 && zone <= 8 ) {
    if ( a == 0 ) {
      latch3_data &= ~( 1 << ( zone - 1 ) );
    } else {
      latch3_data |= ( 1 << ( zone - 1 ) );
    }
  }
  UpdateLatch();
}

void led_day(unsigned char a)
{
  l_day_mode = a;
  if ( a == 0 ) {
    latch2_data &= 0xFE;
  } else {
    latch2_data |= 0x01;
  }
  UpdateLatch();
}


void led_night(unsigned char a)
{
  l_night_mode = a;
  if ( a == 0 ) {
    latch2_data &= 0xFD;
  } else {
    latch2_data |= 0x02;
  }
  UpdateLatch();
}


void led_armed(unsigned char a)
{
  l_armed = a;
  if ( a == 0 ) {
    latch2_data &= 0xFB;
  } else {
    latch2_data |= 0x04;
  }
  UpdateLatch();
}


void led_disarmed(unsigned char a)
{
  l_disarmed = a;
  if ( a == 0 ) {
    latch2_data &= 0xF7;
  } else {
    latch2_data |= 0x08;
  }
  UpdateLatch();
}


void led_short(unsigned char a)
{
  if ( a == 0 ) {
    latch2_data &= 0xEF;
  } else {
    latch2_data |= 0x10;
  }
  UpdateLatch();
}


void led_open(unsigned char a)
{
  if ( a == 0 ) {
    latch2_data &= 0xDF;
  } else {
    latch2_data |= 0x20;
  }
  UpdateLatch();
}

void lcd_back(unsigned char a)
{
  if ( a == 0 ) {
   // LCD_BACK = 0;
   PORTD &= ~(1<<LCD_BACK);
  } else {
    //LCD_BACK = 1;
	PORTD |= (1<<LCD_BACK);
  }
}

void all_led_off()   //including lcd back light
{
  lcd_back(0);
  led_open(0);
  led_short(0);
  led_disarmed(0);
  led_armed(0);
  led_night(0);
  led_day(1);
  led_mot_sir_cut(0);
  led_hoot_cut(0);
  led_battery_charging(0);
  led_battery_cut(0);
  led_battery(0);
  led_syson(0);
  led_mains(0);
}

void CommonTest()
{
  unsigned char i;
  lcd_clear();
  lcd_string_f("    LAMP TEST   ");
  lcd_back(1);
  led_open(1);
  led_short(1);
  led_disarmed(1);
  led_armed(1);
  led_night(1);
  led_day(0);
  led_mot_sir_cut(1);
  led_hoot_cut(1);
  led_battery_charging(1);
  led_battery_low(0);
  led_battery_cut(1);
  led_battery(1);
  led_syson(1);
  led_mains(1);
  for (i = 1; i <= 8; i++) {
    led_zone(i, 1);
  }
  delay(1000);
  lcd_back(0);
  led_open(0);
  led_short(0);
  led_disarmed(0);
  led_armed(0);
  led_night(0);
  led_day(1);
  led_mot_sir_cut(0);
  led_hoot_cut(0);
  led_battery_charging(0);
  led_battery_low(1);
  led_battery_cut(0);
  led_battery(0);
  led_syson(0);
  led_mains(0);
  for (i = 1; i <= 8; i++) {
    led_zone(i, 0);
  }
  delay(1000);
  lcd_clear();
}

/*
interrupt [EXT_INT2] void ext_int2_isr(void)
{
  if ( menu_flag == 0 ) {
    menu_flag = 1;
    if ( GetKey() == KEY_RIGHT ) {
      //fun_reset();
      //lcd_init();
      key_index = 1;

      //===============================SENDING RESET COMMAND TO REMOTE KEYPAD=========================
      Reset();
    }
    else if (GetKey() == KEY_MENU) {
      //key_index = 1;
      key_index = 3;
    }
    else if (GetKey() == KEY_LEFT) {
      //fun_silent();
      //silent_action();
      //lcd_init();
      key_index = 2;
      RS485_EN();
      serial_string("$SILENT");
      serial1_string("$SILENT");
      RS485_DN();
      Sounder_Silent();
    }
    menu_flag = 0;
  }
}
*/

void relay1_trig(unsigned char a) //0/1
{
  if (a == 0) {
    latch4_data &= 0xBF;
  } else {
    latch4_data |= 0x40;
  }
  UpdateLatch();
}

void relay2_trig(unsigned char a) //0/1
{
  if (a == 0) {
    latch4_data &= 0xDF;
  } else {
    latch4_data |= 0x20;
  }
  UpdateLatch();
}

// TAMPER RELAY
void relay3_trig(unsigned char a) //0/1      0 == off,1==on
{
  if (a == 0) {
    latch4_data &= 0xEF;
  } else {
    latch4_data |= 0x10;
  }
  UpdateLatch();
}

void relay4_trig(unsigned char a) //0/1
{
  if (a == 0) {
    latch4_data &= 0xF7;
  } else {
    latch4_data |= 0x08;
  }
  UpdateLatch();
}

void night_power(unsigned char a)
{
  a = (a != 0 ? 0 : 1);
  if (a == 0) {
    latch4_data &= 0xFB;
  } else {
    latch4_data |= 0x04;
  }
  UpdateLatch();
}

void int_hoot(unsigned char a)
{
  if ( a == 0 ) {
    latch4_data &= 0xFD;
  } else {
    latch4_data |= 0x02;
  }
  UpdateLatch();
}

void hooter_trig(unsigned char a)
{
  if ( a == 0 ) {
    latch4_data &= 0xFE;
  } else {
    latch4_data |= 0x01;
  }
  UpdateLatch();
}

void BatteryTrip()
{
  SendLatchData(1, latch1_data);
  SendLatchData(2, latch2_data);
  SendLatchData(3, latch3_data);
  SendLatchData(4, latch4_data);
  ADCSRA = 0x00;
  all_led_off();
  lcd_cmd(0x01);
  delay(10);
  while (MAIN_ON != 0)
  {
    led_battery_low(0);
    int_hoot(1);
    delay(500);
    led_battery_low(1);
    int_hoot(0);
    delay(500);
  }
  Reset();
}


/* 29-06-2021
  #define ADC_VREF_TYPE 0x40
*/
// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
  ADMUX = adc_input | (ADC_VREF_TYPE & 0xff);
  // Delay needed for the stabilization of the ADC input voltage
  // Start the AD conversion
  ADCSRA |= 0x40;
  // Wait for the AD conversion to complete
  while ((ADCSRA & 0x10) == 0);
  ADCSRA |= 0x10;
  return ADCW;
}/// battery cut volt 690
/* 29-06-2021
  #define BAT_CUT_VOLT    700
  #define BAT_FULL_VOLT   672
  #define BAT_TRIP_VOLT    570
  #define BAT_LOW_VOLT     600
*/

unsigned int read_bat_volt()
{
  return read_adc(7);
}


unsigned int read_avg()
{
  unsigned char i;
  unsigned int adc_val = 0;
  for ( i = 0; i < 50; i++ ) {
    adc_val += read_adc(5);
  }
  adc_val = adc_val / 50;
  return adc_val;
}


unsigned int GetZoneStatus(unsigned char a)
{
  unsigned int adc_valu = 0;
  unsigned char buffer[20];
  switch (a)
  {
    case 1:
     /* SEL_ZONE_A = 1;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 0;*/
	 PORTB |= (1<<SEL_ZONE_A);
	 PORTB &= ~(1<<SEL_ZONE_B);
	 PORTB &= ~(1<<SEL_ZONE_C);
      break;

    case 2:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 0;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB &= ~(1<<SEL_ZONE_C);
      break;

    case 3:
     /* SEL_ZONE_A = 1;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 0;*/
	 PORTB |= (1<<SEL_ZONE_A);
	 PORTB |= (1<<SEL_ZONE_B);
	 PORTB &= ~(1<<SEL_ZONE_C);
      break;

    case 4:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 0;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB |=  (1<<SEL_ZONE_B);
	  PORTB &= ~(1<<SEL_ZONE_C);
      break;

    case 5:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 1;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;

    case 6:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 1;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;

    case 7:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 1;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;

    case 8:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 1;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;

    default:
      return remotezonestatus[a - 9] ? STATE_TRIG : STATE_NORMAL;
      break;
  }
  delay(1);
  //adc_val= read_adc(5);
  //    for(i=0;i<50;i++)
  //    {
  //        adc_val+=read_adc(5);
  //    }
  //adc_val=adc_val/50;
  adc_valu = read_avg(); 
  //sprintf(buffer, "%u", adc_valu);
  //serial1_string(buffer);  
  //serial1_putc('\n');
  if ( adc_valu > REF_3 ) {
    return STATE_OPEN;
  } else if ( adc_valu > REF_2 ) {
    return STATE_NORMAL;
  } else if ( adc_valu > REF_1 ) {
    _delay_ms(50);
    adc_valu = read_avg();
    if ( adc_valu > REF_1 ) {
      return STATE_TRIG;
    }
  } else {
    return STATE_SHORT;
  }
}


unsigned int GetZoneStatus_dedicated_nightzone(unsigned char a)
{
  unsigned int adc_valu = 0;
  unsigned char buffer[20];
  //  unsigned char i;
  switch (a)
  {
    case 1:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 0;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB &= ~(1<<SEL_ZONE_C);
      break;
    case 2:
     /* SEL_ZONE_A = 0;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 0;*/
	 PORTB &= ~(1<<SEL_ZONE_A);
	 PORTB &= ~(1<<SEL_ZONE_B);
	 PORTB &= ~(1<<SEL_ZONE_C);
      break;
    case 3:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 0;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB &= ~(1<<SEL_ZONE_C);
      break;
    case 4:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 0;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB &= ~(1<<SEL_ZONE_C);
      break;
    case 5:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 1;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;
    case 6:
      /*SEL_ZONE_A = 1;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 1;*/
	  PORTB |= (1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;
    case 7:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 1;
      SEL_ZONE_C = 1;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB |= (1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;
    case 8:
      /*SEL_ZONE_A = 0;
      SEL_ZONE_B = 0;
      SEL_ZONE_C = 1;*/
	  PORTB &= ~(1<<SEL_ZONE_A);
	  PORTB &= ~(1<<SEL_ZONE_B);
	  PORTB |= (1<<SEL_ZONE_C);
      break;
    default:
      return remotezonestatus[a - 9] ? STATE_TRIG : STATE_NORMAL;
      break;

  }
  delay(1);
  //adc_val= read_adc(5);
  adc_valu = read_avg(); 
  //sprintf(buffer, "%u", adc_valu);
  //serial1_string(buffer);  
  //serial1_putc('\n');
  if (adc_valu < 100) {
    return STATE_NORMAL;
  } else if (adc_valu > 900) {
    return STATE_OPEN;
  }
  else if (adc_valu > 450)     //previous value is 600
  {
    _delay_ms(50);
    adc_valu = read_avg();
    if (adc_valu > 450) {
      return STATE_TRIG;
    }
  }
}


//void autodialer_en()
//{
//    AUTODIALER_EN = 1;
//}


//void extra_UART_en()
//{
////    AUTODIALER_EN = 0;
//}



//void Select_GSM()
//{
//    autodialer_en();
//  GSM_EN=1;
//}


unsigned char memory_test()
{
  if (Eeprom_ReadByte(0) != 0x00) {
    Eeprom_WriteByte(0, 0x00);
    if (Eeprom_ReadByte(0) == 0x00) {
      DefaultSettings();
      return 1;
    } else {
      return 0;
    }
  } else {
    return 1;
  }
}


unsigned char clock_test()
{
  RTC_WriteByte(0x07, 0x10);
  if (RTC_ReadByte(0x00) & 0x80) {
    return 0;
  } else {
    return 1;
  }
}

/*
  unsigned char mains_on=2;
  unsigned int last_sec=60;
*/

void ShowTime()
{
  unsigned char display_hour;int i;
  
  //char Data[16];
  // Uses global cached values c_hour, c_min, etc.
//  RS485_EN=1;
//  _delay_ms(10);
  display_hour = (c_hour > 12) ? (c_hour - 12) : ((c_hour == 0) ? 12 : c_hour);  
  DataForSend[0] = (display_hour/10)+48;
  DataForSend[1] = (display_hour%10)+48;
  LcdSetCursor(1, 0);
  lcd_num(display_hour);
  LcdSetCursor(1, 2);
  lcd_data(':'); 
  DataForSend[2]=':';  
  LcdSetCursor(1, 3);
  lcd_num(c_min);
  DataForSend[3]=(c_min/10)+48;
  DataForSend[4]=(c_min%10)+48;
  LcdSetCursor(1, 5);
  //lcd_data(' ');   
  //LcdSetCursor(1, 6);
  lcd_string_f(c_hour >= 12 ? "PM" : "AM"); 
  DataForSend[5]=(c_hour >= 12 ? 'P' : 'A');
  DataForSend[6]='M';
  LcdSetCursor(1, 7);
  lcd_data(' ');
  DataForSend[7]=' ';
  //lcd_data(' ');
  LcdSetCursor(1, 8);
  lcd_num(c_day);
  DataForSend[8]=(c_day/10)+48;
  DataForSend[9]=(c_day%10)+48;
  DataForSend[10]=' ';
  
  //lcd_data(' ');
  if (c_month >= 1 && c_month <= 12) {
  
    LcdSetCursor(1, 11);
    lcd_string_f(months[c_month - 1]);
    for(i=0;i<3;i++){
      DataForSend[11+i] = months[c_month - 1][i];
      }
  } else {
    lcd_string_f("   ");
  } 
//  serial_string(DataForSend);
//  _delay_ms(10);
//  RS485_EN=0;
}

void EnableSerialInt(unsigned char a)
{
  unsigned int i;
  if (a == 0) {
    UCSR0B = 0x18;
    b_direct_buffer = 0;
  } else {
    UCSR0B = 0x98;
    buff_index = 0;
    for (i = 0; i < 512; i++)
    {
      serial_buff[i] = '\0';
    }
  }
}




//unsigned char IsPrintable(char *s)
//{
//    while(*s){
//    if((*s>='A' && *s<='Z')||(*s>='a' && *s<='z')){
//      return 1;
//    }
//    s++;
//  }
//  return 0;
//}

//
//void SendLedStatus()
//{
//    serial_putc(l_sys_on==1?82:81);
//  serial_putc(l_mains==1?84:83);
//  serial_putc(l_battery==1?86:85);
//  serial_putc(l_battery_cut==1?88:87);
//  serial_putc(l_battery_low==1?90:89);
//  serial_putc(l_battery_charging==1?92:91);
//  serial_putc(l_siren_cut==1?94:93);
//  serial_putc(l_m_siren_cut==1?96:95);
//  serial_putc(l_day_mode==1?98:97);
//  serial_putc(l_night_mode==1?100:99);
//  serial_putc(l_armed==1?102:101);
//  serial_putc(l_armed==1?102:101);
//  serial_putc(l_disarmed==1?104:103);
//}

//void SendZoneStat(unsigned char zone,unsigned char status)
//{
//  unsigned char val = ( zone - 1 ) * 5 + status + 1;
//  delay(1);
//  serial_putc(val);
//  delay(1);
//}/////////////////////////////////////////////////////////////////

void SendNameToUARTs(void)
{
  unsigned char i;
  char buffer[45];
  strcpy(buffer, "$NAM:");
  for (i = 0; i < 16; i++) {
    buffer[5 + i] = Eeprom_ReadByte(ADD_NAME_BASE + i);
    if (buffer[5 + i] == 0xFF || buffer[5 + i] == 0x00) {
      buffer[5 + i] = ' ';
    }
  }
  buffer[21] = '\0';
  serial_println(buffer);
  serial1_string(buffer);
  serial1_putc('\r');
  serial1_putc('\n');
}

void SendAddressToUARTs(void)
{
  unsigned char i;
  char buffer[45];
  strcpy(buffer, "$ADD:");
  for (i = 0; i < 16; i++) {
    buffer[5 + i] = Eeprom_ReadByte(ADD_ADDRESS_BASE + i);
    if (buffer[5 + i] == 0xFF || buffer[5 + i] == 0x00) {
      buffer[5 + i] = ' ';
    }
  }
  buffer[21] = '\0';
  serial_println(buffer);
  serial1_string(buffer);
  serial1_putc('\r');
  serial1_putc('\n');
}

void SendPhoneNumbersToUARTs(void)
{
  unsigned char i, j;
  char buffer[45];
  for (j = 0; j < 10; j++) {
    buffer[0] = '$';
    buffer[1] = 'P';
    buffer[2] = j + '0';
    buffer[3] = ':';
    for (i = 0; i < 16; i++) {
      buffer[4 + i] = Eeprom_ReadByte(ADD_FIRE_NUM_BASE + (j * 16) + i);
      if (buffer[4 + i] == 0xFF || buffer[4 + i] == 0x00) {
        buffer[4 + i] = ' ';
      }
    }
    buffer[20] = '\0';
    serial_println(buffer);
    serial1_string(buffer);
    serial1_putc('\r');
    serial1_putc('\n');
  }
}

void SendEmailIDToUARTs(void)
{
  unsigned char i;
  char buffer[45];
  strcpy(buffer, "$EID:");
  for (i = 0; i < 32; i++) {
    buffer[5 + i] = Eeprom_ReadByte(ADD_EMAIL_ID_BASE + i);
    if (buffer[5 + i] == 0xFF || buffer[5 + i] == 0x00) {
      buffer[5 + i] = ' ';
    }
  }
  buffer[37] = '\0';
  serial_println(buffer);
  serial1_string(buffer);
  serial1_putc('\r');
  serial1_putc('\n');
}

void SendNameAddressToUARTs(void)
{
  SendNameToUARTs();
  SendAddressToUARTs();
  SendPhoneNumbersToUARTs();
  SendEmailIDToUARTs();
}


void ShowCenteredName(void)
{
  unsigned char i, len = 0, start_pos;

  if (name_refresh_flag == 1) {
    // Read name from EEPROM once
    for (i = 0; i < 16; i++) {
      c_name[i] = Eeprom_ReadByte(ADD_NAME_BASE + i);
      if (c_name[i] == 0xFF || c_name[i] == 0x00) {
        c_name[i] = ' ';
      }
    }
    c_name[16] = '\0';
    name_refresh_flag = 0;
  } else {
    // Skip if no refresh needed and we're not in a state that needs it
    // But since the loop normally overwrites LCD, we might still need to write it
    // if we are in the home screen mode (display_counter <= 4)
  }

  // Trim trailing spaces to find effective length
  for (i = 16; i > 0; i--) {
    if (c_name[i - 1] != ' ') {
      len = i;
      break;
    }
  }

  // Trim leading spaces
  start_pos = 0;
  while (start_pos < len && c_name[start_pos] == ' ') {
    start_pos++;
  }

  if (len > start_pos) {
    unsigned char effective_len = len - start_pos;
    unsigned char padding = (14 - effective_len) / 2; // 14 available chars (16 - 2 for network)

//    lcd_cmd(0x80);
    LcdSetCursor(0, 0);
    lcd_data(CUSTOM_CHAR_NETWORK);
    LcdSetCursor(0, 1);
    lcd_data(CUSTOM_CHAR_NETWORK_2);
    for (i = 0; i < padding; i++) {
      lcd_data(' ');
    }
    for (i = 0; i < effective_len; i++) {
      lcd_data(c_name[start_pos + i]);
    }
    for (i = padding + effective_len + 2; i < 15; i++) { // Leave 0x8F for status icon
      lcd_data(' ');
    }
  } else {
//    lcd_cmd(0x80);
    LcdSetCursor(0, 0);
    lcd_data(CUSTOM_CHAR_NETWORK);  
    LcdSetCursor(0, 1);
    lcd_data(CUSTOM_CHAR_NETWORK_2);
    //lcd_string_f("              "); // 14 spaces
  }
}

void EraseSettingsDlg(unsigned char a) /// a=0 = erase logs, a=1 =factory settings
{
  unsigned int i, j;
  lcd_clear();
  lcd_string_f(a == 0 ? "   ERASE LOGS" : " FACTORY RESET");
  lcd_cmd(0xC0);
  lcd_string_f("PRESS ENTER.");
  delay(500);
  while (1)
  {
    if (GetKey() == KEY_ENTER)
    {
      lcd_cmd(0xC0);
      lcd_string_f("000% COMPLETE");
      if (a == 0)
      {
        for (i = 1000; i <= 3000; i++)
        {
          Eeprom_WriteByte(i - 1, 0xFF);
          lcd_cmd(0xC0);
          j = ((i / 20) - 50);
          lcd_data(j / 100 + 48);
          lcd_num(j % 100);

        }
      }
      else
      {

        for (i = 1; i <= 8000; i++)
        {
          Eeprom_WriteByte(i - 1, 0xFF);
          lcd_cmd(0xC0);
          j = i / 80;
          lcd_data(j / 100 + 48);
          lcd_num(j % 100);
        }
        Eeprom_WriteByte(ADD_ZONE_1_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_2_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_3_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_4_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_5_TYPE, 2); // night
        Eeprom_WriteByte(ADD_ZONE_6_TYPE, 2); // night
        Eeprom_WriteByte(ADD_ZONE_7_TYPE, 2); // night
        Eeprom_WriteByte(ADD_ZONE_8_TYPE, 0); // fire
        Eeprom_WriteByte(ADD_ZONE_9_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_10_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_11_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_12_TYPE, 1); // day
        Eeprom_WriteByte(ADD_ZONE_13_TYPE, 2); // day
        Eeprom_WriteByte(ADD_ZONE_14_TYPE, 2); // day
        Eeprom_WriteByte(ADD_ZONE_15_TYPE, 2); // day
        Eeprom_WriteByte(ADD_ZONE_16_TYPE, 0); // day
        Reset();
      }

    }

    if ( GetKey() == KEY_MENU ) {
      lcd_clear();
      return;
    }
  }
}///////////////////////////////////////////////////////////////////

//
//unsigned int _pow(unsigned char a, unsigned char b) /// calculates a to the power b
//{ unsigned char i;
//  unsigned int x=1;
//  if(b==0)
//  {
//    return 1;
//  }
//  else
//  {
//    for(i=1;i<=b;i++)
//    {
//      x*=a;
//    }
//  }
//  return x;
//}

void SounderTimeDlg()
{
  unsigned char a;
  a = Eeprom_ReadByte(ADD_SOUNDER_TIME);

  if ( a > 120 ) {
    a = 3;
  }

  lcd_string_f("  SOUNDER TIME  ");
  lcd_cmd(0xC0);
  lcd_string_f("DELAY : ");
  LcdSetCursor(1, 8);
  lcd_data( a / 100 + 48 );
  LcdSetCursor(1, 9);
  lcd_num( a % 100 ); 
  LcdSetCursor(1, 12);
  lcd_string_f(" MIN ");

  while (1) {
    if ( GetKey() == KEY_UP && a < 120 ) {
      a++;
      lcd_cmd(0xC8);
      lcd_data( a / 100 + 48 );
      lcd_num( a % 100 );
    }
    if ( GetKey() == KEY_DOWN && a > 3 ) {
      a--;
      lcd_cmd(0xC8);
      lcd_data( a / 100 + 48 );
      lcd_num(a % 100);
    }
    if ( GetKey() == KEY_ENTER ) {
      Eeprom_WriteByte(ADD_SOUNDER_TIME, a);
      break;
    }
    if ( GetKey() == KEY_MENU ) {
      break;
    }
    delay(100);
  }
  delay(500);
  lcd_clear();
}

void DelayDlg(unsigned char entry)// entry =0 for exit, 1 for entry
{
  unsigned char entry_exit_delay;
  entry_exit_delay = Eeprom_ReadByte( entry == 0 ? ADD_EXIT_DELAY : ADD_ENTRY_DELAY );
  if ( entry_exit_delay > 120 || entry_exit_delay < 1 ) {
    entry_exit_delay = 20;
  }
  lcd_clear();

  lcd_string_f( entry == 0 ? "   EXIT DELAY   " : "   ENTRY DELAY  " );

  lcd_cmd(0xC0);
  lcd_string_f("DELAY : "); 
  LcdSetCursor(1, 8);
  lcd_data(entry_exit_delay / 100 + 48);
  LcdSetCursor(1, 9);
  lcd_num(entry_exit_delay % 100);
  LcdSetCursor(1, 11);
  lcd_string_f(" SEC");

  while (1)
  {
    if ( GetKey() == KEY_UP && entry_exit_delay < 240 ) {
      entry_exit_delay++;
      lcd_cmd(0xC8);
      lcd_data(entry_exit_delay / 100 + 48); 
      LcdSetCursor(1, 9);
      lcd_num(entry_exit_delay % 100);
    }

    if (GetKey() == KEY_DOWN && entry_exit_delay > 1)
    {
      entry_exit_delay--;
      lcd_cmd(0xC8);
      lcd_data(entry_exit_delay / 100 + 48);
      LcdSetCursor(1, 9);
      lcd_num(entry_exit_delay % 100);
    }
    if (GetKey() == KEY_ENTER)
    {
      Eeprom_WriteByte(entry == 0 ? ADD_EXIT_DELAY : ADD_ENTRY_DELAY, entry_exit_delay);
      break;
    }
    if (GetKey() == KEY_MENU)
    {
      break;
    }
    delay(100);
  }
  delay(500);
  lcd_clear();
}

void SetPassword(unsigned int password)
{ Eeprom_WriteByte(ADD_PASSWORD_D3, password / 1000);
  password = password % 1000;
  Eeprom_WriteByte(ADD_PASSWORD_D2, password / 100);
  password = password % 100;
  Eeprom_WriteByte(ADD_PASSWORD_D1, password / 10);
  password = password % 10;
  Eeprom_WriteByte(ADD_PASSWORD_D0, password);
}

void ZoneSettingsDlg()// 1->8
{ 
  const char *str_zone_types[] = {"      FIRE", "      DAY", "     NIGHT", "    ISOLATED"};
  unsigned char type;
  unsigned char zone = 1;
  bool change;
  delay(300);
  while (1)
  {
    change = 1;
    lcd_clear();
    lcd_string_f("   ZONE SETUP");

    while (1)
    {

      if (change == 1)
      {
        LcdSetCursor(1, 0);
        lcd_string_f("ZONE -> ");
        LcdSetCursor(1, 8);
        //lcd_data(zone+48);
        lcd_num(zone);
        change = 0;
      }
      if (GetKey() == KEY_UP && zone < 16)
      {
        zone++;
        change = 1;
      }
      if (GetKey() == KEY_DOWN && zone > 1)
      {
        zone--;
        change = 1;
      }
      if (GetKey() == KEY_ENTER)
      {
        break;
      }
      if (GetKey() == KEY_MENU)
      {
        delay(200);
        lcd_clear();
        return;
      }
      delay(100);
    }
    delay(500);

    lcd_clear();
    change = 1;
    type = Eeprom_ReadByte(ADD_ZONE_1_TYPE + zone - 1);
    if (type > 3)
    {
      type = 0;
    }
    LcdSetCursor(0, 0);
    lcd_string_f("     ZONE ");
    LcdSetCursor(0, 10);
    lcd_num(zone);
    while (1)
    {
      if (change == 1)
      {
//        lcd_cmd(0xC0);
        LcdSetCursor(1, 0);
        lcd_string_f(str_zone_types[type]);
        lcd_clear_line();
        change = 0;
      }

      if (GetKey() == KEY_UP && type < 3)
      { 
        zone_mode_change=1;
        type++;
        change = 1;
      }
      if (GetKey() == KEY_DOWN && type > 0)
      { 
        zone_mode_change=1;
        type--;
        change = 1;
      }
      if (GetKey() == KEY_ENTER)
      {
        Eeprom_WriteByte(ADD_ZONE_1_TYPE + zone - 1, type);
        break;
      }
      if (GetKey() == KEY_MENU)
      {
        break;
      }
      delay(100);
    }
    delay(500);
  }
  delay(200);
  lcd_clear();
}

void ChangePasswordDlg()
{ unsigned char a = 0;
  unsigned char buff[4];
  unsigned int password = 0;
  lcd_clear(); 
  LcdSetCursor(0, 0);
  lcd_string_f("CHANGE PASSWORD:");
  LcdSetCursor(1, 0);
  lcd_string_f(">");
  while (1)
  {
    if (GetKey() == KEY_1 && a < 4)
    {
      //password+=1*_pow(10,3-a);
      buff[a] = 1;
      LcdSetCursor(1, 1+a);
      lcd_data('*');
      a++;
    }
    if (GetKey() == KEY_2 && a < 4)
    {
      //password+=2*_pow(10,3-a);
      buff[a] = 2;  
      LcdSetCursor(1, 1+a);
      lcd_data('*');
      a++;
    }
    if (GetKey() == KEY_3 && a < 4)
    {
      //password+=3*_pow(10,3-a);
      buff[a] = 3; 
      LcdSetCursor(1, 1+a);
      lcd_data('*');
      a++;
    }
    if (GetKey() == KEY_4 && a < 4 )
    {
      //password+=4*_pow(10,3-a);
      buff[a] = 4;   
      LcdSetCursor(1, 1+a);
      lcd_data('*');
      a++;
    }

    if (a == 4)
    {
      password = buff[0] * 1000 + buff[1] * 100 + buff[2] * 10 + buff[3];
      SetPassword(password);
      break;

    }
    if (GetKey() == KEY_MENU)
    {
      break;
    }
    delay(50);
  }
  delay(200);
  lcd_clear();
}

void SetTypeDlg(unsigned char daynight) // daynighttype=1, armdisarmtype=0
{
  unsigned char a = 0;
  const char* automatic = "    AUTOMATIC";
  const char* manual = "     MANUAL";
  lcd_clear();
  lcd_string_f( daynight == 0 ? " ARM/DISARM MODE" : " DAY/NIGHT MODE" );
  lcd_cmd(0xC0);
  a = Eeprom_ReadByte( daynight == 0 ? ADD_ARM_DISARM_TYPE : ADD_DAY_NIGHT_TYPE );
  lcd_cmd(0xC0);
  lcd_string_f( a == 0 ? automatic : manual );
  lcd_clear_line();

  while (1) {
    if ( GetKey() == KEY_UP ) {
      a = 1;
      lcd_cmd(0xC0);
      lcd_string_f( a == 0 ? automatic : manual );
      lcd_clear_line();
    }

    if ( GetKey() == KEY_DOWN ) {
      a = 0;
      lcd_cmd(0xC0);
      lcd_string_f( a == 0 ? automatic : manual );
      lcd_clear_line();
    }

    if ( GetKey() == KEY_ENTER ) {
      Eeprom_WriteByte( daynight == 0 ? ADD_ARM_DISARM_TYPE : ADD_DAY_NIGHT_TYPE, a );
      if ( daynight == 0 ) {
        if ( a == 0 ) { // auto
          if ( Eeprom_ReadByte(ADD_ARM_HOUR) == 0xFF || Eeprom_ReadByte(ADD_ARM_MINUTE) == 0xFF || Eeprom_ReadByte(ADD_DISARM_HOUR) == 0xFF || Eeprom_ReadByte(ADD_DISARM_MINUTE) == 0xFF ) {
            Eeprom_WriteByte(ADD_ARM_HOUR, 10);
            Eeprom_WriteByte(ADD_ARM_MINUTE, 0);
            Eeprom_WriteByte(ADD_DISARM_HOUR, 18);
            Eeprom_WriteByte(ADD_DISARM_MINUTE, 0);
          }
        }
      } else {
        if ( a == 0 ) { // auto
          if (Eeprom_ReadByte(ADD_DAY_HOUR) == 0xFF || Eeprom_ReadByte(ADD_DAY_MINUTE) == 0xFF || Eeprom_ReadByte(ADD_NIGHT_HOUR) == 0xFF || Eeprom_ReadByte(ADD_NIGHT_MINUTE) == 0xFF ) {
            Eeprom_WriteByte(ADD_DAY_HOUR, 10);
            Eeprom_WriteByte(ADD_DAY_MINUTE, 0);
            Eeprom_WriteByte(ADD_NIGHT_HOUR, 18);
            Eeprom_WriteByte(ADD_NIGHT_MINUTE, 0);
          }
        }
      }
      //SelectKeypad();
      //serial_putc(CMD_RESET);
      Reset();
      break;
    }
    if ( GetKey() == KEY_MENU ) {
      break;
    }
    delay(50);
  }
  delay(100);
  lcd_clear();
}

//unsigned char ValidateChar(unsigned char c)
//{
//  if(c==' '||c=='.'||c=='-'||c=='/'||c=='@'||(c>='0' && c<='9')||(c>='A' && c<='Z')||(c>='a' && c<='z'))
//  {
//    return 1;
//  }
//  return 0;
//}

//flash unsigned char charset[]={' ','.','-','/','@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9'};
#define CHARSET_SIZE    41
/// A->Z,charset,0->9

//unsigned char FindInCharSet(unsigned char a)
//{ unsigned char i;
//  for(i=0;i<=40;i++)
//  {
//    if(a==charset[i])
//    {
//      return i;
//    }
//  }
//  return 0xFF;
//}

void ArmDisarmDlg()
{
  unsigned char val, val_A1, val_A2, val_A3, val_A4, area = 1;
  bool change = 1;
  lcd_string_f("  SYSTEM STATUS");
  val = Eeprom_ReadByte(ADD_ARM_DISARM);
  val_A1 = Eeprom_ReadByte(ADD_ARM_DISARM_P1);
  val_A2 = Eeprom_ReadByte(ADD_ARM_DISARM_P2);
  val_A3 = Eeprom_ReadByte(ADD_ARM_DISARM_P3);
  val_A4 = Eeprom_ReadByte(ADD_ARM_DISARM_P4);

  delay(100);

  if ( val > 1 ) {
    val = 1;
  }

  if ( val_A1 > 1 ) {
    val_A1 = 1;
  }

  if ( val_A2 > 1 ) {
    val_A2 = 1;
  }

  if ( val_A3 > 1 ) {
    val_A3 = 1;
  }

  if ( val_A4 > 1 ) {
    val_A4 = 1;
  }

  while (1) {
    if ( change == 1 ) {
      if ( area == 1 ) {
        lcd_cmd(0x80);
        lcd_string_f("   AREA: ");
        lcd_string_f("FULL   ");
        lcd_cmd(0xC0);
        lcd_string_f( val == 1 ? "     ARMED      " : "    DISARMED    ");
      }

      if ( area == 2 ) {
        lcd_cmd(0x80);
        lcd_string_f("   AREA: ");
        lcd_string_f("P1   ");
        lcd_cmd(0xC0);
        lcd_string_f( val_A1 == 1 ? "     ARMED      " : "    DISARMED    ");
      }

      if ( area == 3 ) {
        lcd_cmd(0x80);
        lcd_string_f("   AREA: ");
        lcd_string_f("P2   ");
        lcd_cmd(0xC0);
        lcd_string_f( val_A2 == 1 ? "     ARMED      " : "    DISARMED    ");
      }

      if ( area == 4 ) {
        lcd_cmd(0x80);
        lcd_string_f("   AREA: ");
        lcd_string_f("P3   ");
        lcd_cmd(0xC0);
        lcd_string_f( val_A3 == 1 ? "     ARMED      " : "    DISARMED    ");
      }

      if ( area == 5 ) {
        lcd_cmd(0x80);
        lcd_string_f("   AREA: ");
        lcd_string_f("P4   ");
        lcd_cmd(0xC0);
        lcd_string_f( val_A4 == 1 ? "     ARMED      " : "    DISARMED    ");
      }
      change = 0;
    }

    if ( GetKey() == KEY_RIGHT && area < 5 ) {
      area++;
      change = 1;
    }

    if ( GetKey() == KEY_LEFT && area > 1 ) {
      area--;
      change = 1;
    }

    if ( GetKey() == KEY_UP ) {
      if ( area == 1 ) {
        if ( val < 1 ) {
          val++;
        }
      }

      if ( area == 2 ) {
        if ( val_A1 < 1 ) {
          val_A1++;
        }
      }

      if ( area == 3 ) {
        if ( val_A2 < 1 ) {
          val_A2++;
        }
      }

      if ( area == 4 ) {
        if ( val_A3 < 1 ) {
          val_A3++;
        }
      }

      if ( area == 5 ) {
        if ( val_A4 < 1 ) {
          val_A4++;
        }
      }
      change = 1;
    }

    if ( GetKey() == KEY_DOWN ) {
      if ( area == 1 ) {
        if ( val > 0 ) {
          val--;
        }
      }

      if ( area == 2 ) {
        if ( val_A1 > 0 ) {
          val_A1--;
        }
      }

      if ( area == 3 ) {
        if ( val_A2 > 0 ) {
          val_A2--;
        }
      }

      if ( area == 4 ) {
        if ( val_A3 > 0 ) {
          val_A3--;
        }
      }

      if ( area == 5 ) {
        if ( val_A4 > 0 ) {
          val_A4--;
        }
      }
      change = 1;
    }

    if ( GetKey() == KEY_ENTER ) {
      Eeprom_WriteByte(ADD_ARM_DISARM, val);
      Eeprom_WriteByte(ADD_ARM_DISARM_P1, val_A1);
      Eeprom_WriteByte(ADD_ARM_DISARM_P2, val_A2);
      Eeprom_WriteByte(ADD_ARM_DISARM_P3, val_A3);
      Eeprom_WriteByte(ADD_ARM_DISARM_P4, val_A4);

      if (val == 0) {
        trigger_reset_flag = 0;
        sounder_time_flag = 0;
        Sounder_Silent();
      }

      break;
    }

    if ( GetKey() == KEY_MENU ) {
      break;
    }
    delay(100);
  }
  delay(100);
  lcd_clear();
}


///////////////////////////////////// LOGGING FUNCTIONS /////////////////////////
unsigned int FindFirstLog() /// finds the first available log memory
{
  unsigned int i;
  for (i = 1; i <= 500; i++) {
    if ( Eeprom_ReadByte(ADD_LOG_BASE + (i - 1) * 8) == 0xFF ) {
      return i;
    }
  }
  return 0;
}

#define LOG_TYPE_TRIG   0
#define LOG_TYPE_TAMP   0xFF

void AddLog(unsigned char zone, unsigned char type)
{
  unsigned int index;
  unsigned int address;
  index = FindFirstLog();
  if ( index == 0 || zone < 1 || zone > 16 ) {
    return;
  }
  address = ADD_LOG_BASE + (index - 1) * 8;
  Eeprom_WriteByte(address++, zone);
  Eeprom_WriteByte(address++, type);
  Eeprom_WriteByte(address++, RTC_GetHour());
  Eeprom_WriteByte(address++, RTC_GetMinute());
  Eeprom_WriteByte(address++, RTC_GetSecond());
  Eeprom_WriteByte(address++, RTC_GetDay());
  Eeprom_WriteByte(address++, RTC_GetMonth());
  Eeprom_WriteByte(address, RTC_GetYear());
}

void ShowLog(unsigned char index) /// index = 1->500
{
  unsigned char type, zone, hour, minute, day, month, year;
  unsigned int address;
  address = ADD_LOG_BASE + (index - 1) * 8;
  zone = Eeprom_ReadByte(address++);
  type = Eeprom_ReadByte(address++);
  hour = Eeprom_ReadByte(address++);
  minute = Eeprom_ReadByte(address++);
  address++; // Skip second
  day = Eeprom_ReadByte(address++);
  month = Eeprom_ReadByte(address++);
  year = Eeprom_ReadByte(address); 

  lcd_clear();
  // Line 1: Lxxx Zxx INTR/FIRE
  lcd_data('L');
  if (index < 100) lcd_data('0');
  if (index < 10) lcd_data('0');
  lcd_num(index);
  lcd_string_f(" Z");
  if (zone < 10) lcd_data('0');
  lcd_num(zone);
  lcd_data(' ');
  lcd_string_f(type == 0 ? "FIRE" : "INTR");

  // Line 2: HH:MMAM/PM  DD/MM
  lcd_cmd(0xC0);
  lcd_num(hour > 12 ? hour - 12 : (hour == 0 ? 12 : hour));
  lcd_data(':');
  if (minute < 10) lcd_data('0');
  lcd_num(minute);
  lcd_string_f(hour >= 12 ? "PM" : "AM");
  lcd_string_f(" ");
//  if (day < 10) lcd_data('0');
  lcd_num(day);
  lcd_data('/');
//  if (month < 10) lcd_data('0');
  lcd_num(month);
  lcd_data('/');    
  lcd_num(year);
}

void ViewLogDlg()
{
  unsigned int lognum, i = 1;
  bool change = 1;
  lcd_clear();
  lcd_string_f("    VIEW LOGS");
  lognum = FindFirstLog();
  if ( lognum == 0 ) {
    lognum = 500;
  } else {
    lognum -= 1;
  }
  lcd_cmd(0xC0);
  i =  lognum;
  if (lognum == 0) {
    lcd_string_f("LOG MEMORY EMPTY");
    delay(3000);
    lcd_clear();
    return;
  }

  while (1) {
    if (change == 1) {
      ShowLog(i);
      change = 0;
    }
    if (GetKey() == KEY_RIGHT && i < lognum) {
      i++;
      change = 1;
    }
    if (GetKey() == KEY_LEFT && i > 1) {
      i--;
      change = 1;
    }
    if (GetKey() == KEY_MENU) {
      break;
    }
    delay(50);
  }

  delay(300);
  lcd_clear();
}


///////////////////////////////////////////ZONE DELAY FUNCTION//////////////////////////////////////

void zone_delay()
{
  //    unsigned char index=0;
  unsigned char zone = 1;
  unsigned int delay_time = 0;
  bool change;
  //delay(300);
  while (1) {
    change = 1;
    lcd_clear();
    lcd_cmd(0x80);
    lcd_string_f("ZONE DELAY SETUP");
    while (1) {

      if (change == 1) {
        lcd_cmd(0xC0);

        lcd_string_f("ZONE : ");
        LcdSetCursor(1, 7);
        lcd_num(zone);

      }

      if (GetKey() == KEY_UP && zone < 16) {
        zone++;
        change = 1;
      }
      if (GetKey() == KEY_DOWN && zone > 1) {
        zone--;
        change = 1;
      }
      if (GetKey() == KEY_ENTER) {
        break;
      }
      if (GetKey() == KEY_MENU) {
        delay(200);
        lcd_clear();
        return;
      }
      delay(100);
    }
    delay(500);
    lcd_clear();
    change = 1;
    if (Eeprom_ReadByte(ADD_ZONE_1_DELAY + (zone - 1)) > 120) {
      delay_time =  0;
      Eeprom_WriteByte((ADD_ZONE_1_DELAY + (zone - 1)), delay_time);
    } else {
      delay_time = Eeprom_ReadByte(ADD_ZONE_1_DELAY + (zone - 1));
    }
    lcd_cmd(0x80);

    if (zone == 6 && GetZoneSettings(zone) == TYPE_NIGHT) {
      lcd_string_f("ENTRY TIME DELAY");
      delay_time = Eeprom_ReadByte(ADD_ENTRY_DELAY);
      lcd_cmd(0xC4);
      lcd_num(delay_time);
    } else {
      lcd_string_f("     DELAY ");
      lcd_cmd(0xC4);
      lcd_num(delay_time);
    }

    lcd_string_f(" SEC");
    while (1) {
      if (zone != 6) {
        if (change == 1) {
          lcd_cmd(0xC4);
          lcd_num(delay_time);
          lcd_string_f(" SEC");
          change = 0;
        }

        if (GetKey() == KEY_UP && delay_time < 90) {
          delay_time++;
          change = 1;
        }
        if (GetKey() == KEY_DOWN && delay_time > 0) {
          delay_time--;
          change = 1;
        }
        if (GetKey() == KEY_ENTER) {

          Eeprom_WriteByte(ADD_ZONE_1_DELAY + zone - 1, delay_time);
          break;
        }
      }
      else
      {
        if (GetKey() == KEY_ENTER) {
          break;
        }
      }
      if (GetKey() == KEY_MENU) {
        break;
      }
      delay(100);
    }
    delay(500);
  }
  delay(200);
  lcd_clear();
}


/////////////////////////////////////////////////////////////////////////////////////////////////
void ArmTamper()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void Walk_Test_Dlg()
{
  unsigned char i, key;
  //#asm("cli"); // disable interrupts
  lcd_clear();
  lcd_string_f("    WALK TEST   ");
  lcd_cmd(0xC0);
  lcd_string_f("ENTER -> START");

  while (1) {
    key = GetKey();
    if (key == KEY_MENU)
    {
      lcd_clear();
      return;
    }
    else if (key == KEY_ENTER)
    {
      break;
    }
    delay(50);
  }

  night_power(1);
  lcd_cmd(0xC0);
  lcd_string_f("WAIT    SEC...");

  for (i = 30; i >= 1; i--) {
    lcd_cmd(0xC5);
    lcd_num(i);
    delay(600);
  }
  lcd_cmd(0xC0);
  lcd_string_f("ANY KEY -> EXIT ");

  while (GetKey() == KEY_NONE) {
    for (i = 1; i <= 8; i++) {
      if (GetZoneSettings(i) != TYPE_ISO) {
        if (GetZoneStatus(i) == STATE_TRIG) {
          int_hoot(1);
          hooter_trig(1);
          delay(3000);
          int_hoot(0);
          hooter_trig(0);
          delay(1000);
        }
      }
    }
  }

  relay1_trig(1);
  relay2_trig(1);
  hooter_trig(1);
  CommonTest();
  relay1_trig(0);
  relay2_trig(0);
  hooter_trig(0);

  night_power(0);
  Reset();

}

// =========================================

// Character set for cycling through: A-Z, 0-9, and space
const char charset[] PROGMEM= "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
#define CHARSET_LENGTH  37

const char charset_num[] PROGMEM = "0123456789 ";
#define CHARSET_NUM_LENGTH 11

const char charset_email[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@._- ";
#define CHARSET_EMAIL_LENGTH 67

// Helper function to get next character
unsigned char GetNextChar(unsigned char current_char, unsigned char direction)
{
  unsigned char i;

  // Find current character position
  for (i = 0; i < CHARSET_LENGTH; i++) {
    if (charset[i] == current_char) {
      break;
    }
  }

  if (direction == 1) { // UP - next character
    i++;
    if (i >= CHARSET_LENGTH) {
      i = 0;  // Wrap around
    }
  } else { // DOWN - previous character
    if (i == 0) {
      i = CHARSET_LENGTH - 1;  // Wrap around
    } else {
      i--;
    }
  }

  return charset[i];
}

unsigned char GetNextCharFromSet(unsigned char current_char, unsigned char direction, const char *set, unsigned char set_len)
{
  unsigned char i;

  // Find current character position
  for (i = 0; i < set_len; i++) {
    if (set[i] == current_char) {
      break;
    }
  }

  if (direction == 1) { // UP - next character
    i++;
    if (i >= set_len) {
      i = 0;  // Wrap around
    }
  } else { // DOWN - previous character
    if (i == 0) {
      i = set_len - 1;  // Wrap around
    } else {
      i--;
    }
  }

  return set[i];
}

// Display 16 characters with current position blinking
void DisplayEditLine(unsigned char *data, unsigned char current_pos, unsigned char blink_state)
{
  unsigned char i;

  lcd_cmd(0xC0);

  // Display all 16 characters
  for (i = 0; i < 16; i++) {
    if (i == current_pos) {
      // Blink the current character
      if (blink_state) {
        lcd_data(data[i]);  // Show character
      } else {
        lcd_data(' ');      // Hide character (show space)
      }
    } else {
      lcd_data(data[i]);
    }
  }
}

// Set Name Function - 16 characters max (0-15)
void SetName(void)
{
  unsigned char name[17];
  unsigned char current_pos = 0;
  unsigned char i = 0;
  unsigned char key = 0;
  unsigned char timeout_counter = 0;
  unsigned char max_length = 16;
  unsigned char blink_counter = 0;
  unsigned char blink_state = 1;

  // Load current name from EEPROM
  for (i = 0; i < max_length; i++) {
    name[i] = Eeprom_ReadByte(ADD_NAME_BASE + i);
    if (name[i] == 0xFF || name[i] == 0x00) {
      name[i] = ' ';  // Replace empty with space
    }
  }
  name[max_length] = 0;  // Null terminate

  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f("   ENTER NAME");
  delay(500);
  lcd_clear();

  while (1) {
    // Display position counter
    lcd_cmd(0x80);
    if (current_pos < 10) {
      lcd_data(' ');
      lcd_data('0' + current_pos);
    } else {
      lcd_data('0' + (current_pos / 10));
      lcd_data('0' + (current_pos % 10));
    }
    lcd_string_f("/15   ");

    // Display name with blinking current character
    DisplayEditLine(name, current_pos, blink_state);

    key = GetKey();

    if (key != KEY_NONE) {
      timeout_counter = 0;
      blink_counter = 0;
      blink_state = 1;  // Reset blink state to show character

      switch (key) {
        case KEY_UP:  // '8' - Cycle to next character
          name[current_pos] = GetNextChar(name[current_pos], 1);
          break;

        case KEY_DOWN:  // '0' - Cycle to previous character
          name[current_pos] = GetNextChar(name[current_pos], 0);
          break;

        case KEY_LEFT:  // '*' - Move to previous position
          if (current_pos > 0) {
            current_pos--;
          }
          break;

        case KEY_RIGHT:  // '#' - Move to next position
          if (current_pos < max_length - 1) {
            current_pos++;
          }
          break;

        case KEY_ENTER:  // 'D' - Save and exit
          // Save name to EEPROM
          for (i = 0; i < max_length; i++) {
            Eeprom_WriteByte(ADD_NAME_BASE + i, name[i]);
            delay(5);
          }

          SendNameToUARTs();
          name_refresh_flag = 1;

          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("NAME SAVED");
          delay(1000);
          lcd_clear();
          return;

        case KEY_MENU:  // 'C' - Cancel without saving
          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("    CANCELLED");
          delay(1000);
          lcd_clear();
          return;
      }
      delay(200);
    } else {
      delay(100);
      blink_counter++;
      timeout_counter++;

      // Toggle blink every 5 cycles (500ms with 100ms delay)
      if (blink_counter >= 1) {
        blink_counter = 0;
        blink_state = (blink_state == 0) ? 1 : 0;  // Toggle
      }

      if (timeout_counter > 150) { // 15 seconds timeout
        lcd_clear();
        lcd_cmd(0x80);
        lcd_string_f("   TIMEOUT");
        delay(1000);
        lcd_clear();
        return;
      }
    }
  }
}

void DisplayEditLineScrolled(unsigned char *data, unsigned char current_pos, unsigned char blink_state, unsigned char total_len)
{
  unsigned char i;
  unsigned char start_pos = 0;

  if (current_pos >= 16) {
    start_pos = current_pos - 15;
  }

  if (start_pos + 16 > total_len) {
    start_pos = total_len - 16;
  }

  lcd_cmd(0xC0);

  for (i = 0; i < 16; i++) {
    unsigned char abs_pos = start_pos + i;
    if (abs_pos == current_pos) {
      if (blink_state) {
        lcd_data(data[abs_pos]);
      } else {
        lcd_data(' ');
      }
    } else {
      lcd_data(data[abs_pos]);
    }
  }
}

void REC_PLY_MSG(const char *type, const char *action){
   serial1_string_f(type);
   serial1_putc(' ');
   serial1_string_f(action);
   serial1_putc('\n'); 
   lcd_clear();
}

void call_set(const char *type, unsigned int index1, unsigned int index2){
  unsigned char c,key ;
  int loop=1,sent=0;
  unsigned char seq=0;
  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f(type);
  LcdSetCursor(0, 5);
  lcd_string("CALL SETTING");
    while(loop==1){
    c=Eeprom_ReadByte(index1);
    if(c==1){
      lcd_cmd(0xC0);
      lcd_string_f("     ENABLE     "); 
      if(sent == 1){  
      sent = 0;
      serial1_string_f(type);
      serial1_string_f("_Call:");
      serial1_string_f("TRUE");
      serial1_putc('\n'); 
      }
    }else{
      lcd_cmd(0xC0);
      lcd_string_f("     DISABLE    "); 
      if(sent == 1){ 
      sent = 0;
      serial1_string_f(type);
      serial1_string_f("_Call:");
      serial1_string_f("FLSH");
      serial1_putc('\n');   
      }
    }
    key = GetKey();
    if (key == KEY_UP) {
     sent = 1; 
     Eeprom_WriteByte(index1,1); 
    }else if (key == KEY_DOWN) { 
     sent = 1;
     Eeprom_WriteByte(index1,0);
    }else if (key == KEY_MENU || key == KEY_ENTER) {   
     loop=2;
    }
  }
  while(loop==2 && c==1){
    seq = Eeprom_ReadByte(index2);
    ///if(c==1){
      lcd_cmd(0xC0);
      lcd_string_f("CALL SEQ:"); 
      lcd_num(seq);
    //}else{
      //lcd_cmd(0xC0);
      //lcd_string_f("     DISABLE    ");
    //}
    key = GetKey();
    if (key == KEY_UP) { 
     seq++;
     if(seq>5){
     seq=5;
     }
     Eeprom_WriteByte(index2,seq);  
     /*serial1_string_f(type);
     serial1_string_f("_CallSq:");
     serial1_putc(seq+48);
     serial1_putc('\n'); */
    }else if (key == KEY_DOWN) { 
      
    if(seq>=1){
    seq--;
    }
    Eeprom_WriteByte(index2,seq);
   /* serial1_string_f(type);
    serial1_string_f("_CallSq:");
    serial1_putc(seq+48);
    serial1_putc('\n');  */
    }else if (key == KEY_MENU || key == KEY_ENTER) {   
    serial1_string_f(type);
    serial1_string_f("_CallSq:");
    serial1_putc(seq+48);
    serial1_putc('\n');
     lcd_clear();
     return;
    }
  } 
  lcd_clear();
}

void msg_set(const char *type, unsigned int index1, unsigned int index2){
  unsigned char c,key ; 
  int loop=1,sent=0;
  unsigned char seq=0;
  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f(type);
  LcdSetCursor(0, 5);
  lcd_string("MSG SETTING");
  while(loop==1){
    c=Eeprom_ReadByte(index1);
    if(c==1){
      lcd_cmd(0xC0);
      lcd_string_f("     ENABLE     "); 
      if(sent == 1){  
      sent = 0;
      serial1_string_f(type);
      serial1_string_f("_Sms:");
      serial1_string_f("TRUE");
      serial1_putc('\n'); 
      }
    }else{
      lcd_cmd(0xC0);
      lcd_string_f("     DISABLE    "); 
      if(sent == 1){ 
      sent = 0;
      serial1_string_f(type);
      serial1_string_f("_Sms:");
      serial1_string_f("FLSH");
      serial1_putc('\n');   
      }
    }
    key = GetKey();
    if (key == KEY_UP) {
    sent = 1;
     Eeprom_WriteByte(index1,1); 
    }else if (key == KEY_DOWN) { 
    sent = 1;
    Eeprom_WriteByte(index1,0);
    }else if (key == KEY_MENU || key == KEY_ENTER) {   
     loop=2;
    }
  }
  while(loop==2 && c==1){
    seq = Eeprom_ReadByte(index2);
    ///if(c==1){
      lcd_cmd(0xC0);
      lcd_string_f("SMS SEQ:"); 
      lcd_num(seq);
      lcd_data(' ');
    //}else{
      //lcd_cmd(0xC0);
      //lcd_string_f("     DISABLE    ");
    //}
    key = GetKey();
    if (key == KEY_UP) { 
     seq++;
     if(seq>5){
     seq=5;
     }
     Eeprom_WriteByte(index2,seq);   
     /*serial1_string_f(type);
     serial1_string_f("_SmsSq:");
     serial1_putc(seq+48);
     serial1_putc('\n');   */
    }else if (key == KEY_DOWN) { 
      
    if(seq>=1){
    seq--;
    }
    Eeprom_WriteByte(index2,seq);
    /*serial1_string_f(type);
    serial1_string_f("_SmsSq:");
    serial1_putc(seq+48);
    serial1_putc('\n'); */
    }else if (key == KEY_MENU || key == KEY_ENTER) {  
    serial1_string_f(type);
    serial1_string_f("_SmsSq:");
    serial1_putc(seq+48);
    serial1_putc('\n'); 
     lcd_clear();
     return;
    }
  }
  lcd_clear();
}

void EditPhoneNumber(unsigned char index, unsigned int NUM_BASE)
{
  unsigned char phone[17];
  unsigned char current_pos = 0;
  unsigned char i = 0;
  unsigned char key = 0;
  unsigned char timeout_counter = 0;
  unsigned char max_length = 16;
  unsigned char blink_counter = 0;
  unsigned char blink_state = 1;
  unsigned int base_addr = NUM_BASE + (index * 16);

  // Load current phone number from EEPROM
  for (i = 0; i < max_length; i++) {
    phone[i] = Eeprom_ReadByte(base_addr + i);
    if (phone[i] == 0xFF || phone[i] == 0x00) {
      phone[i] = ' ';  // Replace empty with space
    }
  }
  phone[max_length] = 0;  // Null terminate

  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f("EDIT PHONE #");
  lcd_num(index + 1);
  delay(500);
  lcd_clear();

  while (1) {
    lcd_cmd(0x80);
    lcd_string_f("PH ");
    lcd_num(index + 1);
    lcd_string_f(" POS:");
    lcd_num(current_pos);
    lcd_string_f("   ");

    DisplayEditLine(phone, current_pos, blink_state);

    key = GetKey();
    if (key != KEY_NONE) {
      timeout_counter = 0;
      blink_counter = 0;
      blink_state = 1;

      switch (key) {
        case KEY_UP:
          phone[current_pos] = GetNextCharFromSet(phone[current_pos], 1, charset_num, CHARSET_NUM_LENGTH);
          break;
        case KEY_DOWN:
          phone[current_pos] = GetNextCharFromSet(phone[current_pos], 0, charset_num, CHARSET_NUM_LENGTH);
          break;
        case KEY_LEFT:
          if (current_pos > 0) current_pos--;
          break;
        case KEY_RIGHT:
          if (current_pos < max_length - 1) current_pos++;
          break;
        case KEY_ENTER:
          for (i = 0; i < max_length; i++) {
            Eeprom_WriteByte(base_addr + i, phone[i]);
            delay(5);
          }
          SendPhoneNumbersToUARTs();
          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("PHONE SAVED");
          delay(1000);
          return;
        case KEY_MENU:
          return;
      }
      delay(200);
    } else {
      delay(100);
      blink_counter++;
      timeout_counter++;
      if (blink_counter >= 1) {
        blink_counter = 0;
        blink_state = (blink_state == 0) ? 1 : 0;
      }
      if (timeout_counter > 150) return;
    }
  }
}



void AddPhoneNumbersDlg(unsigned int NUM_BASE)
{
  unsigned char num_index = 0;
  unsigned char key;

  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f("SELECT PHONE NO");

  while (1) {
    lcd_cmd(0xC0);
    lcd_string_f("NUMBER : ");
    LcdSetCursor(1, 9);
    lcd_num(num_index + 1);
    lcd_string_f("     ");

    key = GetKey();
    if (key == KEY_UP) {
      if (num_index < 9) num_index++;
    } else if (key == KEY_DOWN) {
      if (num_index > 0) num_index--;
    } else if (key == KEY_ENTER) {
      EditPhoneNumber(num_index, NUM_BASE);
      lcd_clear();
      lcd_cmd(0x80);
      lcd_string_f("SELECT PHONE NO");
    } else if (key == KEY_MENU) {
      return;
    }
    delay(200);
  }
}

void AddEmailIDDlg(void)
{
  unsigned char email[33];
  unsigned char current_pos = 0;
  unsigned char i = 0;
  unsigned char key = 0;
  unsigned char timeout_counter = 0;
  unsigned char max_length = 32;
  unsigned char blink_counter = 0;
  unsigned char blink_state = 1;

  for (i = 0; i < max_length; i++) {
    email[i] = Eeprom_ReadByte(ADD_EMAIL_ID_BASE + i);
    if (email[i] == 0xFF || email[i] == 0x00) {
      email[i] = ' ';
    }
  }
  email[max_length] = 0;

  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f("  ENTER EMAIL");
  delay(500);
  lcd_clear();

  while (1) {
    lcd_cmd(0x80);
    lcd_string_f("POS:");
    LcdSetCursor(0, 4);
    lcd_num(current_pos);
    LcdSetCursor(0, 6);
    lcd_string_f("/31      ");

    DisplayEditLineScrolled(email, current_pos, blink_state, max_length);

    key = GetKey();
    if (key != KEY_NONE) {
      timeout_counter = 0;
      blink_counter = 0;
      blink_state = 1;

      switch (key) {
        case KEY_UP:
          email[current_pos] = GetNextCharFromSet(email[current_pos], 1, charset_email, CHARSET_EMAIL_LENGTH);
          break;
        case KEY_DOWN:
          email[current_pos] = GetNextCharFromSet(email[current_pos], 0, charset_email, CHARSET_EMAIL_LENGTH);
          break;
        case KEY_LEFT:
          if (current_pos > 0) current_pos--;
          break;
        case KEY_RIGHT:
          if (current_pos < max_length - 1) current_pos++;
          break;
        case KEY_ENTER:
          for (i = 0; i < max_length; i++) {
            Eeprom_WriteByte(ADD_EMAIL_ID_BASE + i, email[i]);
            delay(5);
          }
          SendEmailIDToUARTs();
          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("EMAIL SAVED");
          delay(1000);
          return;
        case KEY_MENU:
          return;
      }
      delay(200);
    } else {
      delay(100);
      blink_counter++;
      timeout_counter++;
      if (blink_counter >= 1) {
        blink_counter = 0;
        blink_state = (blink_state == 0) ? 1 : 0;
      }
      if (timeout_counter > 150) return;
    }
  }
}

// Set Address Function - 16 characters max (0-15)
void SetAddress(void)
{
  unsigned char address[17];
  unsigned char current_pos = 0;
  unsigned char i = 0;
  unsigned char key = 0;
  unsigned char timeout_counter = 0;
  unsigned char max_length = 16;
  unsigned char blink_counter = 0;
  unsigned char blink_state = 1;

  // Load current address from EEPROM
  for (i = 0; i < max_length; i++) {
    address[i] = Eeprom_ReadByte(ADD_ADDRESS_BASE + i);
    if (address[i] == 0xFF || address[i] == 0x00) {
      address[i] = ' ';  // Replace empty with space
    }
  }
  address[max_length] = 0;  // Null terminate

  lcd_clear();
  lcd_cmd(0x80);
  lcd_string_f(" ENTER ADDRESS");
  delay(500);
  lcd_clear();

  while (1) {
    // Display position counter
    lcd_cmd(0x80);
    if (current_pos < 10) {
      lcd_data(' ');
      lcd_data('0' + current_pos);
    } else {
      lcd_data('0' + (current_pos / 10));
      lcd_data('0' + (current_pos % 10));
    }
    lcd_string_f("/15   ");

    // Display address with blinking current character
    DisplayEditLine(address, current_pos, blink_state);

    key = GetKey();

    if (key != KEY_NONE) {
      timeout_counter = 0;
      blink_counter = 0;
      blink_state = 1;  // Reset blink state to show character

      switch (key) {
        case KEY_UP:  // '8' - Cycle to next character
          address[current_pos] = GetNextChar(address[current_pos], 1);
          break;

        case KEY_DOWN:  // '0' - Cycle to previous character
          address[current_pos] = GetNextChar(address[current_pos], 0);
          break;

        case KEY_LEFT:  // '*' - Move to previous position
          if (current_pos > 0) {
            current_pos--;
          }
          break;

        case KEY_RIGHT:  // '#' - Move to next position
          if (current_pos < max_length - 1) {
            current_pos++;
          }
          break;

        case KEY_ENTER:  // 'D' - Save and exit
          // Save address to EEPROM
          for (i = 0; i < max_length; i++) {
            Eeprom_WriteByte(ADD_ADDRESS_BASE + i, address[i]);
            delay(5);
          }

          SendAddressToUARTs();

          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("ADDRESS SAVED");
          delay(1000);
          lcd_clear();
          return;

        case KEY_MENU:  // 'C' - Cancel without saving
          lcd_clear();
          lcd_cmd(0x80);
          lcd_string_f("    CANCELLED");
          delay(1000);
          lcd_clear();
          return;
      }
      delay(200);
    } else {
      delay(100);
      blink_counter++;
      timeout_counter++;

      // Toggle blink every 5 cycles (500ms with 100ms delay)
      if (blink_counter >= 1) {
        blink_counter = 0;
        blink_state = (blink_state == 0) ? 1 : 0;  // Toggle
      }

      if (timeout_counter > 150) { // 15 seconds timeout
        lcd_clear();
        lcd_cmd(0x80);
        lcd_string_f("   TIMEOUT");
        delay(1000);
        lcd_clear();
        return;
      }
    }
  }
}

// =========================================

void display(unsigned int a, unsigned int b, unsigned int c)
{ lcd_cmd(0xC0);
  lcd_num(a);
  lcd_string_f("-");
  lcd_num(b);
  lcd_string_f("-");
  lcd_num(c);
  lcd_cmd(0xD0);
}

void set_holiday(unsigned int address, unsigned int a, unsigned int b, unsigned int c)
{
  Eeprom_WriteByte(address, a);
  delay(50);
  Eeprom_WriteByte((address + 1), b);
  delay(50);
  Eeprom_WriteByte((address + 2), c);
  delay(50);
}

void holidaylistdlg()
{
  volatile unsigned int index = 1;
  unsigned int day = 0, month = 0, year = 18, address;
  unsigned int pos = 1;
  unsigned int ret = 0;
  bool update = 0;
  lcd_clear();
  //delay(100);
  //lcd_string_f("ADD LIST");
  //lcd_clear();
  lcd_cmd(0x0E);   // cursor on
  while (1) {
    //time_out++;
    lcd_cmd(0x80);
    lcd_string_f("  HOLIDAY:");
    lcd_num(index);
    address = (ADD_HOLIDAY_BASE + ((index - 1) * 3));
    if (Eeprom_ReadByte(address) == 0xFF) {
      lcd_cmd(0xC0);
      lcd_string_f("00-00-18");
      day = 0;
      month = 0;
    } else {
      day = Eeprom_ReadByte(address);
      month = Eeprom_ReadByte(address + 1);
      year = Eeprom_ReadByte(address + 2);
    }

    display(day, month, year);
    while (1) {
      if (GetKey() == KEY_RIGHT && (pos > 0 || pos < 4)) {
        delay(50);
        pos++;
        if (pos == 4) {
          pos = 1;
        }
      }
      if (GetKey() == KEY_LEFT && (pos > 0 || pos < 4)) {
        delay(50);
        pos--;
        if (pos == 0) {
          pos = 1;
        }
      }

      if (pos == 1) {
        lcd_cmd(0xC1);
        if (GetKey() == KEY_UP) {
          if (day < 31) {
            day++;
          } else {
            day = 0;
          }
          update = 1;
        }

        if (GetKey() == KEY_DOWN) {
          if (day > 0) {
            day--;
          } else {
            day = 31;
          }
          update = 1;
        }
        day_arr[index] = day;
      }

      if (pos == 2) {
        lcd_cmd(0xC4);
        if (GetKey() == KEY_UP) {
          if (month < 12) {
            month++;
          } else {
            month = 0;
          }
          update = 1;
        }
        if (GetKey() == KEY_DOWN) {
          if (month > 0) {
            month--;
          } else {
            month = 12;
          }
          update = 1;
        }
        month_arr[index] = month;
      }

      if (pos == 3) {
        lcd_cmd(0xC7);
        if (GetKey() == KEY_UP) {
          year++;
          update = 1;
        }
        if (GetKey() == KEY_DOWN) {
          year--;
          if (year < 1) {
            year = 18;
          }
          update = 1;
        }
        year_arr[index] = year;
      }
      if (update == 1) {
        display(day, month, year);
        update = 0;
      }

      if (GetKey() == KEY_ENTER) {
        set_holiday(address, day, month, year);
        index++;
        if (index == 31) {
          index = 1;
        }
        pos = 1;
        break;
      }

      if (GetKey() == KEY_MENU) {
        delay(300);
        ret = 1;
        lcd_clear();
        break;
      }
    }

    if (ret == 1) {
      break;
    }
  }
  lcd_cmd(0x0C);  //cursour off
}

unsigned int Get_holiday()
{
  unsigned int index;
  unsigned int day_now, month_now, year_now, address;
  bool set;
  day_now = RTC_GetDay();
  month_now = RTC_GetMonth();
  year_now = RTC_GetYear();
  for (index = 1; index < 31; index++) {
    address = (ADD_HOLIDAY_BASE + ((index - 1) * 3));
    if (day_now == Eeprom_ReadByte(address)) {
      delay(100);
      if (month_now == Eeprom_ReadByte(address + 1)) {
        delay(100);
        if (year_now == Eeprom_ReadByte(address + 2)) {
          set = 1;
          break;
        }
      }
    } else
      set = 0;
  }
  if (set == 1) {
    return 1;
  }
  else
    return 0;
}

void clear_holiday()
{
  unsigned int address, j;
  lcd_cmd(0x01);
  lcd_cmd(0x80);
  lcd_string_f("CLEARING HOLIDAY");
  for (j = 1; j < 31; j++)
  {
    lcd_cmd(0xC0);
    lcd_string_f("PLEASE WAIT..");
    address = (ADD_HOLIDAY_BASE + ((j - 1) * 3));
    Eeprom_WriteByte(address, 0xFF);
    delay(50);
    Eeprom_WriteByte((address + 1), 0xFF);
    delay(50);
    Eeprom_WriteByte((address + 2), 0xFF);
    delay(50);
  }
  lcd_cmd(0x01);
}

void UserCreationDlg(unsigned char user_number)
{
  unsigned char a = 0;
  unsigned char buff[4];
  //  unsigned int password=0;
  unsigned char count = 0;
  unsigned int passadderbase = 0;
  passadderbase = 675 + ((user_number - 1) * 4);
  lcd_cmd(0x01);
  lcd_cmd(0x80);
  lcd_string_f("USER : ");
  LcdSetCursor(0, 7);
  lcd_num(user_number);
  lcd_cmd(0xC0);
  lcd_string_f("SET PASS: ");

  while (1) {
  LcdSetCursor(1, 10+a);
    if (GetKey() == KEY_LEFT && a < 4) {
      //password+=1*_pow(10,3-a);
      buff[a] = 1;
      lcd_data('*');
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      _delay_ms(500);
      a++;
      count = 0;
    }

    if (GetKey() == KEY_UP && a < 4) {
      //password+=2*_pow(10,3-a);
      buff[a] = 2;
      lcd_data('*');
      _delay_ms(500);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }

    if (GetKey() == KEY_DOWN && a < 4) {
      //password+=3*_pow(10,3-a);
      buff[a] = 3;
      lcd_data('*');
      _delay_ms(500);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }

    if (GetKey() == KEY_RIGHT && a < 4 ) {
      //password+=4*_pow(10,3-a);
      buff[a] = 4;
      lcd_data('*');
      _delay_ms(500);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }

    if (GetKey() == KEY_ENTER) {
      Eeprom_WriteByte(passadderbase, buff[3]);
      Eeprom_WriteByte(passadderbase + 1, buff[2]);
      Eeprom_WriteByte(passadderbase + 2, buff[1]);
      Eeprom_WriteByte(passadderbase + 3, buff[0]);
      break;
    }

    //    if(count==1000){
    //      //result=2; // timeout
    //      break;
    //    }
    else {
      count++;
    }
  }
}

void PartSelectionDlg(unsigned char user_number)
{
  unsigned char a = 0, i = 0, change = 0;
  unsigned char count = 0;
  unsigned char buff[4] = {0, 0, 0, 0};
  unsigned int partaddressbase = 0;
  partaddressbase = ADD_PART1_U1 + ((user_number - 1) * 4);
  lcd_cmd(0x01);
  lcd_cmd(0x80);
  lcd_string_f("USER : "); 
  LcdSetCursor(0, 7);
  lcd_num(user_number);
  lcd_cmd(0xC0);
  lcd_string_f("PART:");
  //lcd_cmd(0xC5); 
  LcdSetCursor(1, 5);
  for (i = 0; i < 4; i++) {
    if (Eeprom_ReadByte(partaddressbase + i) == 1) {
      LcdSetCursor(1, 5);
      lcd_string_f("P1 ");
      buff[i] = 1;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 2) {
      LcdSetCursor(1, 8);
      lcd_string_f("P2 ");
      buff[i] = 2;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 3) {
      LcdSetCursor(1, 11);
      lcd_string_f("P3 ");
      buff[i] = 3;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 4) {
      LcdSetCursor(1, 14);
      lcd_string_f("P4 ");
      buff[i] = 4;
    } else {
      lcd_string_f("   ");
      buff[i] = 0;
    }
  }
  lcd_cmd(0xC5);
  while (1) {
    if ( GetKey() == KEY_LEFT && a < 4 ) {
      //password+=1*_pow(10,3-a);
      if (change == 0) {
        buff[0] = 0;
        buff[1] = 0;
        buff[2] = 0;
        buff[3] = 0;
        LcdSetCursor(1, 5);
        lcd_string_f("           ");
        LcdSetCursor(1, 5);
        //change = 1;
      }
      buff[a] = 1;
      lcd_string_f("P1 ");
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      //RemotekepadData(22,0);
      //RemotekepadData(11,0);
      _delay_ms(200);
      a++;
      count = 0;
    }
    if ( GetKey() == KEY_UP && a < 4 ) {
      if (change == 0) {
        buff[0] = 0;
        buff[1] = 0;
        buff[2] = 0;
        buff[3] = 0;
        LcdSetCursor(1, 5);
        lcd_string_f("           ");
        LcdSetCursor(1, 8);
        //change = 1;

      }
      buff[a] = 2;
      lcd_string_f("P2 ");
      //RemotekepadData(23,0);
      //RemotekepadData(11,0);
      _delay_ms(200);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }
    if (GetKey() == KEY_DOWN && a < 4) {
      if (change == 0) {
        buff[0] = 0;
        buff[1] = 0;
        buff[2] = 0;
        buff[3] = 0;
        lcd_cmd(0xC5);
        lcd_string_f("           ");
        LcdSetCursor(1, 11);
        //change = 1;

      }
      buff[a] = 3;
      lcd_string_f("P3 ");
      //RemotekepadData(24,0);
      //RemotekepadData(11,0);
      _delay_ms(200);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }
    if (GetKey() == KEY_RIGHT && a < 4 ) {
      if (change == 0) {
        buff[0] = 0;
        buff[1] = 0;
        buff[2] = 0;
        buff[3] = 0;
        lcd_cmd(0xC5);
        lcd_string_f("           ");
        LcdSetCursor(1, 14);
        //change = 1;

      }
      buff[a] = 4;
      lcd_string_f("P4 ");
      //RemotekepadData(25,0);
      //RemotekepadData(11,0);
      _delay_ms(200);
      /*while(GetKey()!=KEY_NONE)
        {
        delay(1);
        }*/
      a++;
      count = 0;
    }
    if (GetKey() == KEY_ENTER) {
      Eeprom_WriteByte(partaddressbase, buff[0]);
      Eeprom_WriteByte(partaddressbase + 1, buff[1]);
      Eeprom_WriteByte(partaddressbase + 2, buff[2]);
      Eeprom_WriteByte(partaddressbase + 3, buff[3]);
      break;
    }
    if (GetKey() == KEY_MENU) {
      break;
    }

    _delay_ms(200);
    //RemotekepadData(user_number+11,0);
    //RemotekepadData(11,0);
    //if(user_number == 1){
    //        RemotekepadData( 8, 0 );  // Type =8 for hooter cut, zone nuber irrelevent
    // }
    //if(user_number == 2){
    //     RemotekepadData( 9, 0 );  // Type =8 for hooter cut, zone nuber irrelevent
    //}
    //RemotekepadData(24,0);
  }
}

void CreateUser()
{ unsigned char a = 1;
  lcd_cmd(0x80);
  lcd_string_f("  CREATE USER   ");
  lcd_cmd(0xC0);
  lcd_clear_line();
  lcd_cmd(0xC0);
  lcd_string_f("   USER : 01");
  while (1) {
    if (GetKey() == KEY_UP && a < 40) {
      a++;
      lcd_cmd(0xCA);
      lcd_num(a);
      lcd_clear_line();
    }
    if (GetKey() == KEY_DOWN && a > 1) {
      a--;
      lcd_cmd(0xCA);
      lcd_num(a);
      lcd_clear_line();
    }
    if (GetKey() == KEY_ENTER) {
      UserCreationDlg(a);
      PartSelectionDlg(a);
      break;
    }
    if (GetKey() == KEY_MENU) {
      break;
    }
    _delay_ms(500);
  }
  delay(500);
  lcd_clear();
}

void ArmDisarmPart(unsigned char user_number)
{
  unsigned char i, index = 0, val, assigned_part = 0;
  unsigned char buff[4] = {0, 0, 0, 0};
  unsigned int partaddressbase;
  partaddressbase = ADD_PART1_U1 + ((user_number - 1) * 4);
  for (i = 0; i < 4; i++) {
    if (Eeprom_ReadByte(partaddressbase + i) == 1) {
      buff[i] = 1;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 2) {
      buff[i] = 2;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 3) {
      buff[i] = 3;
    } else if (Eeprom_ReadByte(partaddressbase + i) == 4) {
      buff[i] = 4;
    } else {
      buff[i] = 0;
    }
  }

  for (i = 0; i < 4; i++) {
    if (buff[i] != 0) {
      index = buff[i];
      break;
    }
  }

  if (index != 0) {
    val = Eeprom_ReadByte((ADD_ARM_DISARM_P1) + (index - 1));
    if (val != 0) {
      val = 1;
    } else {
      val = 0;
    }
    lcd_cmd(0x01);
    lcd_cmd(0x80);
    lcd_string_f("      PART ");
    lcd_num(index);
    lcd_cmd(0xC0);

    if (val == 0) {
      lcd_string_f("<     DISARM    ");
    } else {
      lcd_string_f("       ARM     >");
    }

    while (1) {
      for (i = 0; i < 4; i++) {
        if (buff[i] == index) {
          assigned_part = 1;
        }
      }

      lcd_cmd(0x80);
      lcd_string_f("      PART ");
      lcd_num(index);
      if (assigned_part == 1) {
        lcd_cmd(0xC0);
        if (index)
          if (val == 0) {
            lcd_string_f("<     DISARM    ");
          } else {
            lcd_string_f("       ARM     >");
          }
      } else {
        lcd_cmd(0xC0);
        lcd_string_f("USER NOT ALLOWED");
      }

      if (GetKey() == KEY_LEFT) {
        lcd_cmd(0xC0);
        lcd_string_f("       ARM     >");
        val = 1;
      }

      if (GetKey() == KEY_RIGHT) {
        lcd_cmd(0xC0);
        lcd_string_f("<     DISARM    ");
        val = 0;
      }

      if (GetKey() == KEY_UP && index < 4) {
        index++;
        assigned_part = 0;
        val = Eeprom_ReadByte((ADD_ARM_DISARM_P1) + (index - 1));
      }

      if (GetKey() == KEY_DOWN && index > 1) {
        index--;
        assigned_part = 0;
        val = Eeprom_ReadByte((ADD_ARM_DISARM_P1) + (index - 1));
      }

      if (GetKey() == KEY_MENU) {
        delay(500);
        lcd_clear();
        return;
      }

      if (GetKey() == KEY_ENTER) {
        if (val == 1) {
          Eeprom_WriteByte(((ADD_ARM_DISARM_P1) + (index - 1)), 1);
        } else {
          Eeprom_WriteByte(((ADD_ARM_DISARM_P1) + (index - 1)), 0);
        }

        if (index < 4) {
          index++;
        } else if (index == 4) {
          index = 1;
        }
        //break;
      }
    }
  } else {
    lcd_cmd(0x01);
    lcd_cmd(0x80);
    lcd_string_f("NO PART SELECTED");
    _delay_ms(1000);
  }
}



void CMS_Setting()
{
  unsigned char a = 0;
  bool change = 1;
  lcd_clear();
  _delay_ms(100);
  lcd_cmd(0x80);
  lcd_string_f("  CMS SETTINGS  ");
  a = GetCMSEnabled();

  while (1) {
    if ( change == 1 ) {
      change = 0;
      lcd_cmd(0xC0);
      lcd_string_f( a == 0 ? "    DISABLED    " : "     ENABLED    ");
      change = 0;
    }
    if ( GetKey() == KEY_UP ) {
      a = 1;
      change = 1;
    }
    if ( GetKey() == KEY_DOWN ) {
      a = 0;
      change = 1;
    }
    if ( GetKey() == KEY_ENTER ) {
      Eeprom_WriteByte(ADD_CMS_EN, a);
      _delay_ms(100);
      //SetGSMEnabled(a);
      //Reset();
      break;
    }
    if (GetKey() == KEY_MENU) {
      break;
    }
    _delay_ms(50);
  }
  _delay_ms(100);
  lcd_clear();
}

void SetupDlg()
{
  unsigned char index = 0, a, count = 0;
  bool change = 1;
  lcd_clear();
  a = PasswordDlg(1);

  if ( a == 0 ) {
    lcd_string_f(" ACCESS DENIED");
    delay(1000);
    lcd_clear();
    return;
  } else if ( a == 1 ) {
    lcd_clear();
    lcd_cmd(0x80);
    lcd_string_f("  WELLCOME BACK ");
    lcd_cmd(0xC0);
    lcd_string_f("     MASTER    ");
    _delay_ms(1000);
    lcd_cmd(0x01);

    while (1)
    {
      if ( change == 1 ) {
        LcdSetCursor(0, 0);
        lcd_string_f("      SETUP    ");
        LcdSetCursor(1, 1);
        if ( index > 0 ) { 
        LcdSetCursor(1, 0);
          lcd_data('<');
        } else {
          lcd_data(' ');
        }
        lcd_string_f(str_menu[index]);
        if ( index < MENU_NUM - 1) {
          LcdSetCursor(1, 15);
          lcd_data('>');
        } else {
          lcd_data(' ');
        }
        change = 0;
      }
      if ( GetKey() == KEY_LEFT && index > 0 ) {
        index--;
        change = 1;
        delay(100);
        count = 0;
        continue;
      }
      if ( GetKey() == KEY_RIGHT && index < MENU_NUM - 1 ) {
        index++;
        change = 1;
        delay(100);
        count = 0;
        continue;
      }
      if ( GetKey() == KEY_MENU ) {
        delay(500);
        lcd_clear();
        return;
      }

      if ( GetKey() == KEY_ENTER ) {
        change = 1;
        switch (index)
        {
          case MENU_USER_CREATE:
            /// set time
            CreateUser();
            break;

          case MENU_SET_TIME:
            /// set time
            SetTimeDlg(1);
            break;

          case MENU_SET_DAY_TIME:
            /// set day time
            SetTimeDlg(2);
            break;

          case MENU_SET_NIGHT_TIME:
            /// set night time
            SetTimeDlg(3);
            break;

          case MENU_SET_DAY_NIGHT_MODE:
            /// set day/night mode
            SetTypeDlg(1);
            break;

          case MENU_ARM_DISARM:
            /// armed/disarmed
            ArmDisarmDlg();
            break;

          case MENU_ARM_DISARM_MODE:
            // set arm/disarm mode
            SetTypeDlg(0);
            break;

          case MENU_ARM_TIME:
            /// set arm time
            SetTimeDlg(4);
            break;

          case MENU_DISARM_TIME:
            /// set disarm time
            SetTimeDlg(5);
            break;

          case MENU_ZONE_SETTINGS:
            /// zone settings
            ZoneSettingsDlg();
            break;

          case MENU_ZONE_DELAY:
            zone_delay();
            break;

          case MENU_SET_ENTRY_TIME:
            // ENTRY DELAY
            DelayDlg(1);
            break;

          case MENU_SET_EXIT_TIME:
            // EXIT DELAY
            DelayDlg(0);
            break;

          case MENU_SOUNDER_TIME:
            SounderTimeDlg();
            break;

          case MENU_NIGHT_CUT_TRIG:
            // night cut trigger
            NightTriggerDlg();
            break;

          case MENU_WALK_TEST:
            Walk_Test_Dlg();
            break;

          case NAME_:
            SetName();
            break;

          case ADDRESS_:
            SetAddress();
            break;

          case MENU_VIEW_LOGS:
            ViewLogDlg();
            break;

          case MENU_ERASE_LOGS:
            // ERASE LOGS
            EraseSettingsDlg(0);
            break;

          case MENU_HOLIDAY_LIST:
            holidaylistdlg();
            break;

          case MENU_HOLIDAY_CLEAR:
            clear_holiday();
            break;

          case MENU_CHANGE_PASSWORD:
            // change password
            ChangePasswordDlg();
            break;

          case MENU_FACTORY_RESET:

            FactoryReset();
            break;

          case MENU_KEYPAD_ON_OFF:
            Repeater_Setting();
            break;

          case MENU_CMS_SETTING:
            CMS_Setting();
            break;

          case MENU_ADD_FIRE_NUM:
            AddPhoneNumbersDlg(ADD_FIRE_NUM_BASE);
            break;
            
          case MENU_ADD_INTR_NUM:
            AddPhoneNumbersDlg(ADD_INTR_NUM_BASE);
            break;
           
          case MENU_FIRE_CALL_SET:
            call_set("FIRE",ADD_FIRE_CALL_SET,ADD_FIRE_CALL_SEQ );
            break;
          
          case MENU_INTR_CALL_SET:
            call_set("INTR",ADD_INTR_CALL_SET, ADD_INTR_CALL_SEQ );
            break;      
          
          case MENU_TMPR_CALL_SET:
            call_set("TMPR",ADD_TMPR_CALL_SET, ADD_TMPR_CALL_SEQ );
            break;
              
          case MENU_FIRE_SMS_SET:
            msg_set("FIRE ",ADD_FIRE_SMS_SET, ADD_FIRE_SMS_SEQ );
            break;
          
          case MENU_INTR_SMS_SET:
            msg_set("INTR ",ADD_INTR_SMS_SET, ADD_INTR_SMS_SEQ );
            break;
          
          case MENU_TMPR_SMS_SET:
            msg_set("TMPR ",ADD_TMPR_SMS_SET, ADD_TMPR_SMS_SEQ );
            break;
                          
          case MENU_ADD_EMAIL_ID:
            AddEmailIDDlg();
            break; 
            
          case MENU_FIRE_MSG_REC:
            REC_PLY_MSG("FIRE","REC");
            break;
          
          case MENU_FIRE_MSG_PLY:
            REC_PLY_MSG("FIRE","PLY");
            break;
          
          case MENU_INTR_MSG_REC:
            REC_PLY_MSG("INTR","REC");
            break;
          
          case MENU_INTR_MSG_PLY:
            REC_PLY_MSG("INTR","PLY");
            break;
          
          case MENU_TMPR_MSG_REC:
            REC_PLY_MSG("TMPR","REC");
            break;
          
          case MENU_TMPR_MSG_PLY:
            REC_PLY_MSG("TMPR","PLY");
            break;
          
          case MENU_MSG4_REC:
            REC_PLY_MSG("MSG4","REC");
            break;
          
          case MENU_MSG4_PLY:
            REC_PLY_MSG("MSG4","PLY");
            break;
          default:
            change = 0;
            break;
        }
      }
      delay(50);
      count++;
      if ( count == 150 ) {
        count = 0;
        break;
      }
    }
  } else if ( a > 1 && a < 11 ) {
    lcd_clear();
    lcd_cmd(0x80);
    lcd_string_f("  WELLCOME BACK ");
    lcd_cmd(0xC0);
    lcd_string_f("     USER ");
    lcd_num(a - 1);
    _delay_ms(2000);
    ArmDisarmPart(a - 1);
    //return;
  } else if ( a == 12 ) {
    return;
    /// ok, lets proceeed
  }
}


void port_init(void)
{
  // -------- PORT A --------
  PORTA = 0x5F;     // Set pull-ups / default states FIRST
  DDRA  = 0x44;     // Set directions

  // Explicit overrides (safe)
  /*DDRA.2 = 1;
  PORTA.2 = 0;
  DDRA.0 = 1;
  PORTA.0 = 0;*/
  DDRA |= (1 << PA2);      // PA2 output
  PORTA &= ~(1 << PA2);    // PA2 LOW

  DDRA |= (1 << PA0);      // PA0 output
  PORTA &= ~(1 << PA0);    // PA0 LOW

  // -------- PORT B --------
  PORTB = 0x04;
  DDRB  = 0xFB;

  //DDRB.3 = 1;
  //PORTB.3 = 0;
  DDRB |= (1 << PB3);      // PA2 output
  PORTB &= ~(1 << PB3);    // PA2 LOW
  // -------- PORT C --------
  PORTC = 0x00;
  DDRC  = 0xFF;

  // -------- PORT D --------
  PORTD = 0xFE;     // Pull-ups enabled first
  DDRD  = 0x02;     // Base configuration

  //DDRD.5 = 1;       // Output
  //PORTD.5 = 0;
  DDRD |= (1 << PD5);      
  PORTD &= ~(1 << PD5);    

  //DDRD.7 = 1;       // Output
  //PORTD.7 = 0;
  DDRD |= (1 << PD7);
  PORTD &= ~(1 << PD7);

  //DDRD.4 = 0;       // Tamper input
  //PORTD.4 = 1;      // Pull-up ON
  DDRD |= (1 << PD4);
  PORTD &= ~(1 << PD4);

  //DDRD.6 = 0;       // Day/Night key
  //PORTD.6 = 1;      // Pull-up ON
  DDRD |= (1 << PD6);
  PORTD &= ~(1 << PD6);
  // -------- ADC --------
  ADMUX  = ADC_VREF_TYPE & 0xFF;
  ADCSRA = 0x87;

  // -------- TIMER 0 --------
  TCCR0A = 0x00;
  TCCR0B = 0x05;
  TCNT0  = 0x00;

  // -------- TIMER 1 --------
  TCCR1A = 0x10;
  TCCR1B = 0x03;
  TCNT1H = 0x00;
  TCNT1L = 0x00;
  ICR1H  = 0x00;
  ICR1L  = 0x00;
  OCR1AH = 0x00;
  OCR1AL = 0x00;
  OCR1BH = 0x00;
  OCR1BL = 0xFF;
}



unsigned char int_hoot_val = 0;
unsigned char per_sec = 0;
unsigned char arm_disarm  = 0;
// Timer 0 overflow interrupt service routine

ISR(TIMER0_OVF_vect)
{
  // Place your code here
  if ( int_hoot_val == 16 ) {
    TCNT0 = 0x00;
    int_hoot_bit = ! int_hoot_bit;
    per_sec = ! per_sec;
    int_hoot_val = 0;
    TCNT0 = 0x01;
  } else {
    int_hoot_val++;
  }
}

unsigned char rev_nibble(unsigned char a)
{
  unsigned char val = 0;
  //  x = a;
  val += 8 * ( a & 0x01 );
  a >>= 1;
  val += 4 * ( a & 0x01 );
  a >>= 1;
  val += 2 * ( a & 0x01 );
  a >>= 1;
  val += ( a & 0x01 );
  return val;
}


// External Interrupt 0 service routine
ISR(INT2_vect)
{
  unsigned char remote_data = 0x0F, bus_value, i, bit_value = 3;
	//remote_data=rev_nibble((PIND&0xF0)>>4);   
	bus_value = PCF8574_ex_ReadByte(); 
	remote_data = rev_nibble( bus_value & 0x0F );   
	/*for(i=0;i<=3;i++)
	{ 
	if(which_key(bus_value,(i)) == 0)
	{ 
		remote_data &= ~(1<<bit_value);
	} 
	else
	{
		remote_data|= (1<<bit_value);
	}
	bit_value--;  
	} */
	if( remote_data == 0x00 ){
		// break;
	}else if( remote_data == 0x0F ){        
		//arm
		//Eeprom_WriteByte(ADD_ARM_DISARM,1);
		
	}else if( remote_data == 0x0B ){            
		//disarm 
		//Eeprom_WriteByte(ADD_ARM_DISARM,0);   
		Sounder_Silent();
		
	}else if( remote_data == 0x0D ){   
	//else if(remote_data==0x03)
		//reset
		Reset();
	}else if( remote_data == 0x0E ){
		arm_disarm = ~arm_disarm;
		if( arm_disarm == 0){
			Eeprom_WriteByte( ADD_ARM_DISARM, 0 );     //disarm
		}else{
			Eeprom_WriteByte( ADD_ARM_DISARM, 1 );   	//arm
		}
	}else{
		remotezonestatus[remote_data-1] = 1;
	}
	
	PCF8574_ex_WriteByte(0xFF);
}


ISR(INT1_vect)
{ // Place your code here
  //  sms_flag = 1;
}

/////////////////////////////////////////////////////////////////////////////////
void EnableRemoteInt(unsigned char bEnabled)
{
  if ( bEnabled == 0 ) {
    EICRA = 0x00;
  } else {
    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Falling Edge
    // INT1: On
    // INT1 Mode: Falling Edge
    // INT2: On
    EICRA = 0x38;
    EIMSK = 0x04; // Enable INT2 (0x04 = 0000 0100)
    EIFR = 0x04;
    PCICR = 0x00;
    sei(); // Enable global interrupts
    /*GICR|=0x40;
      MCUCR=0x02;
      MCUCSR=0x00;
      GIFR=0x40;*/

  }
}

// Timer2 overflow interrupt service routine
//extern volatile unsigned int pir_counter=0;

unsigned char PasswordDlg_on_off(unsigned char x)
{
  unsigned char a = 0;
  unsigned char buff[4];
  unsigned int password = 0;
  unsigned char result = 0;
  unsigned char count = 0;
  unsigned char key = 0;

  lcd_clear();
  backlight = 1;

  if (x == 0) {
    lcd_string_f(" SYSTEM OFF CODE");
  } else {
    lcd_string_f(" SYSTEM ON CODE ");
  }

  lcd_cmd(0xC0);
  lcd_string_f(">");
  delay(625);

  while (1) {
    key = GetKey();

    // Accept numeric keys: '1' through '9' and '0'
    if ((key >= '0' && key <= '9')) {
      if (a < 4) {
        // Convert ASCII character to digit value
        buff[a] = key - '0';

        // Display asterisk for each digit
        lcd_data('*');
        a++;
        count = 0;
      }
    }

    // MENU key (C) = Cancel
    if (key == KEY_MENU) {
      result = 2;  // Cancel
      break;
    }

    // Check if 4 digits entered
    if (a == 4) {
      password = buff[0] * 1000 + buff[1] * 100 + buff[2] * 10 + buff[3];

      // Check password: 3241
      if (password == 3241) {
        result = 1;  // Correct
      } else {
        result = 0;  // Wrong
      }
      break;
    }

    // Timeout after 50 iterations (5 seconds at 100ms delay)
    if (count == 50) {
      result = 2;  // Timeout
      break;
    } else {
      count++;
    }

    delay(200);
  }

  delay(200);
  lcd_clear();
  return result;
}


/*==========================FOR DATA SYNC========================*/
void DataSync()
{
  serial_putc(Eeprom_ReadByte(ADD_SOUNDER_TIME));
  serial_putc(Eeprom_ReadByte(ADD_EXIT_DELAY));
  serial_putc(Eeprom_ReadByte(ADD_ENTRY_DELAY));
  serial_putc(Eeprom_ReadByte(ADD_ARM_DISARM));
  serial_putc(Eeprom_ReadByte(ADD_ZONE_1_DELAY));
  serial_putc(Eeprom_ReadByte(ADD_ZONE_1_TYPE));
}




void main(void)
{

  unsigned char a = 0, i, zone_counter = 0, password_match, per_sec_in = 1;
  static unsigned char low_bat_flag = 0;
  static unsigned char power_last = 0xFF; // 0=mains, 1= battery,else none (first time switch on)
  static unsigned int zone_open_isolate = 0;
  unsigned int zoneval, d;
  static unsigned char trigger_zone_number = 0, ArmTamper_delay_count = 0;
  unsigned char display_counter = 0;
  static unsigned int sounder_count = 0;
  static bool tamp_switch = 0, tapmer_zone_trigger = 0, led = 0;
  static bool open_iso = 0;
  static unsigned char batt_switch_count = 0;
  static unsigned int batvolt = 0, lcd_re_init_count = 0;
  static char zone_delay_array_present[16];
  static int zone_delay_iso_open_array[16];

  static unsigned int timenow = 0, daytime = 0, nighttime = 0, armtime = 0, disarmtime = 0;
  static unsigned int pir_count = 0;
  static unsigned int mag_count = 0;
  static unsigned char firetrig = 0;
  static unsigned char intrtrig = 0;
  static unsigned char Healthy_Zone = 0;
  static unsigned char display_tamper_flag = 0;
  static unsigned char Part_area = 0;
  static unsigned int CmsSetReset = 0x00;
  static unsigned int CmsBitChk = 0x00;
  static unsigned int HeartBeatTime = 0;
  static unsigned int NextBeat = 0;
  static unsigned char TamperFlagRemoteKeypad = 0, hooter_cut_flag = 0;

  MCUSR = 0;
  wdt_disable();
  // daynight=1-> day, 0->night
  port_init();
  UpdateLatch();
  //OUT_EN = 0;
  PORTA |= (1<<OUT_EN);
  night_power(1);
  lcd_init();
  sei();
  _delay_ms(500);
  lcd_build(CUSTOM_CHAR_BATTERY, lcd_battery);
  lcd_build(CUSTOM_CHAR_MAINS, lcd_mains);
  lcd_build(CUSTOM_CHAR_NETWORK, lcd_network1);
  lcd_build(CUSTOM_CHAR_NETWORK_2, lcd_network2);
  //lcd_build(CUSTOM_CHAR_PSTN,lcd_pstn);
  lcd_back(1);
  lcd_clear();
  lcd_cmd(0x90);
  lcd_string_f("JARVIS");
  _delay_ms(5000);
  serial_init();
  serial1_init();
  //debug_uart();
  Keypad_Init();

  // Enable global interrupts
  for ( i = 0; i < 11; i++ ) {
    lcd_cmd(0x18);
    _delay_ms(100);
  }
  _delay_ms(500);
  //serial_string("S0:Hello World");
  //serial1_string("S1:Hello World");
  lcd_cmd(0x01);
  lcd_cmd(0x80);
  lcd_string_f("     JARVIS     ");
  lcd_cmd(0xC0);
  lcd_string_f("  VERSION 2.0T");
  _delay_ms(5000);
  lcd_clear();
  lcd_string_f("   ComIP Init");
  lcd_cmd(0xC0);
  lcd_string_f("Please Wait...");
  _delay_ms(20000);
  lcd_cmd(0x80);
  EnableSerialInt(0);
  //lcd_data(CUSTOM_CHAR_PSTN);
  //while(1);
  //*** serial_putc(CMD_RESET);
  lcd_clear();
  CommonTest();
  lcd_back(1);
  //////////////////////////////////////////
  lcd_clear();
  lcd_string_f("Checking Memory");
  lcd_cmd(0xC0);
  lcd_string_f("Please Wait...");
  _delay_ms(10000);
  lcd_clear();
  if ( memory_test() == 1 ) {
    lcd_string_f("   MEMORY OK");
  } else {
    lcd_string_f("  MEMORY ERROR");
  }
  delay(2000);

  SendNameAddressToUARTs();
 //serial_string("ALL ZONE HEALTHY");
///////////////////////////////////////
  lcd_clear();
  lcd_string_f("Checking RTC");
  lcd_cmd(0xC0);
  lcd_string_f("Please Wait...");
  _delay_ms(5000);  
  lcd_clear();
  if ( clock_test() == 1 ) {
    lcd_string_f("    CLOCK OK");
    delay(1000);
  } else {
    lcd_string_f("  CLOCK ERROR");
    delay(1000);
    SetTimeDlg(1);
  }
  lcd_clear();
  lcd_string_f("   INTELLIGENT");
  lcd_cmd(0xC0);
  lcd_string_f("  ALARM SYSTEM");
  
  _delay_ms(3000);
  PCF8574_ex_WriteByte(0xFF);

  //TIMSK=0x41; // timers start
  TIMSK0 = 0x01;
  // Timer/Counter 1 Interrupt(s) initialization
  TIMSK1 = 0x00;
  // Timer/Counter 2 Interrupt(s) initialization
  TIMSK2 = 0x01;
  // Timer/Counter 3 Interrupt(s) initialization
  TIMSK3 = 0x00;
  delay(1000);
  led_syson(1);

  // REMOTE KEYPAD COMMUNICATION
  //if( GetRPTREnabled() == 1 ){                          // Added on 18-01-2022
  EnableRemoteInt(1);
  //}

  if ( Eeprom_ReadByte(ADD_ARM_DISARM) == 0 ) {       // TOKENNO0005
    led_armed(0);
    led_disarmed(1);
  } else if ( Eeprom_ReadByte(ADD_ARM_DISARM) == 1 ) {
    led_armed(1);
    led_disarmed(0);
  }

  for ( i = 1; i < 17; i++ ) {
    if (Eeprom_ReadByte( ADD_ZONE_1_DELAY + (i - 1) ) > 120 ) {
      zone_delay_array_present[i - 1] = 0;
      Eeprom_WriteByte((ADD_ZONE_1_DELAY + (i - 1)), 0);
    } else {
      zone_delay_array_present[i - 1] = Eeprom_ReadByte(ADD_ZONE_1_DELAY + (i - 1)); //for double the delay time
    }
  }

  //send_initial_zone_status();
  //RS485_EN();
  //serial_string("ALL ZONE HEALTHY");
  //RS485_DN();
  // ******************************************************************************************************
  // ******************************************* MAIN LOOP ************************************************
  // ******************************************************************************************************
  while (1)
  { 
   check_serial();
    c_hour = RTC_GetHour();
    c_min = RTC_GetMinute();
    c_sec = RTC_GetSecond();
    c_day = RTC_GetDay();
    c_month = RTC_GetMonth();
    c_year = RTC_GetYear();
    c_arm_status = Eeprom_ReadByte(ADD_ARM_DISARM);
    if (c_arm_status == 0) {
      Sounder_Silent();
      relay3_trig(0);
      relay4_trig(0);
      trigger_reset_flag = 0;
      sounder_time_flag = 0;
    }
    sei();              // Enable global interrupts
    //        UCSR1B |= (1 << RXCIE1); // Re-enable UART receive interrupt

    //RS485_EN(); 
        //serial_string_f("ALL ZONE HEALTHY");
    //RS485_DN();

    // ****************************************** REMOTE KEYPAD *********************************************
    if ( menu_flag == 0 ) {
      unsigned char k = GetKey();
      if (k != KEY_NONE) {
        menu_flag = 1;
        if ( k == KEY_RIGHT ) {
          key_index = 1;
          Reset();
        } else if ( k == KEY_MENU ) {
          key_index = 3;
        } else if ( k == 'B' ) {
          CommonTest();
        } else if ( k == KEY_LEFT ) {
          key_index = 2;
          RS485_EN();
          serial_string("$SILENT");
          serial1_string("$SILENT");
          RS485_DN();
          Sounder_Silent();
        } else if ( k == '9' ) {
          ViewLogDlg();
        }
        menu_flag = 0;
      }
    }

    //===============================================================================
    rcv_action();
    check_serial();
    // read_full_string();

    if ( power_last == 0 ) { // mains
      if ( MAIN_ON == 1 ) { // battery

        power_last = 1;
      }
    } else if (power_last == 1) { // battery
      if (MAIN_ON == 0) { // main

        power_last = 0;
      }
    } else {
      power_last = MAIN_ON;
    }

    if ( MAIN_ON == 1 && read_bat_volt() < BAT_LOW_VOLT) {
      if ( low_bat_flag == 0 ) {
        low_bat_flag = 1;
      }
    }

    if ( MAIN_ON == 0 ) { // mains
      led_battery_low(1);
      batt_switch_count++;
      if ( batt_switch_count == 1 ) {
        //BAT_SWITCH = 1;
        //PCF8574_ex_WriteByte(7,0);
        //BAT_CUT_SWITCH = 0;
		PORTD &= ~(1<<BAT_CUT_SWITCH);
        _delay_ms(10);
        batvolt = read_bat_volt();
        //BAT_CUT_SWITCH = 1;
		PORTD |= (1<<BAT_CUT_SWITCH);
        //BAT_SWITCH = 0;
        //PCF8574_ex_WriteByte(7,1);
      }

      if ( batt_switch_count == 50 ) {
        batt_switch_count = 0;
      }

      if ( batvolt < 50 ) {
        led_battery_cut(1);
        led_battery_charging(0);
      } else {
        led_battery_cut(0);
        led_battery_charging(1);
      }

      /*if(read_bat_volt()<BAT_FULL_VOLT)
        {
        led_battery_charging(1);
        }
        else
        {
        led_battery_charging(0);
        }*/
    } else {
      led_battery_charging(0);
      led_battery_cut(0);
      if ( read_bat_volt() <= BAT_TRIP_VOLT ) {
        // trip

        //SelectKeypad();
        //*** serial_putc(CMD_RESET);
        BatteryTrip();

      } else if ( read_bat_volt() < BAT_LOW_VOLT ) {
        led_battery_low(0);
      } else {
        led_battery_low(1);
      }
    }

    if ( Eeprom_ReadByte(ADD_ARM_DISARM_TYPE) == 0 ) { // automatic
      //here
      timenow = RTC_GetHour() * 60 + RTC_GetMinute();
      armtime = Eeprom_ReadByte(ADD_ARM_HOUR) * 60 + Eeprom_ReadByte(ADD_ARM_MINUTE);
      disarmtime = Eeprom_ReadByte(ADD_DISARM_HOUR) * 60 + Eeprom_ReadByte(ADD_DISARM_MINUTE);

      if ( armtime < disarmtime ) {
        if ( timenow < armtime || timenow > disarmtime ) {
          if ( Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {
            Eeprom_WriteByte(ADD_ARM_DISARM, 0 );
            c_arm_status = 0;
            led_armed(0);
            led_disarmed(1);
            Sounder_Silent();
            trigger_reset_flag = 0;
            sounder_time_flag = 0;
          }
        } else {
          if ( Eeprom_ReadByte( ADD_ARM_DISARM) == 0 ) {
            Eeprom_WriteByte( ADD_ARM_DISARM, 1 );
            c_arm_status = 1;
            led_armed(1);
            led_disarmed(0);
          }
        }
      } else { // midnight rollover
        if ( timenow >= disarmtime && timenow < armtime ) {
          if ( Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {
            Eeprom_WriteByte(ADD_ARM_DISARM, 0 );
            c_arm_status = 0;
            led_armed(0);
            led_disarmed(1);
            Sounder_Silent();
            trigger_reset_flag = 0;
            sounder_time_flag = 0;
          }
        } else {
          if ( Eeprom_ReadByte( ADD_ARM_DISARM) == 0 ) {
            Eeprom_WriteByte( ADD_ARM_DISARM, 1 );
            c_arm_status = 1;
            led_armed(1);
            led_disarmed(0);
          }
        }
      }
    } else { // manual arm/disarm
      if ( Eeprom_ReadByte(ADD_ARM_DISARM) == 0 ) {
        //disarm
        led_armed(0);
        led_disarmed(1);
      } else {
        led_armed(1);
        led_disarmed(0);
        //arm
      }
    }

    led_hoot_cut(HOOTER_CUT);
    led_mot_sir_cut(~MOT_SIR_CUT);

    ///////////////////////////////////////////////// DAY/NIGHT DETECTION ////////////////////////////////////////////////////

    if ( Eeprom_ReadByte(ADD_DAY_NIGHT_TYPE) == 0 ) // auto daynight
    {  
    //  serial1_string("DAY_NIGHT_A\n");
      timenow = RTC_GetHour() * 60 + RTC_GetMinute();
      daytime = Eeprom_ReadByte(ADD_DAY_HOUR) * 60 + Eeprom_ReadByte(ADD_DAY_MINUTE);
      nighttime = Eeprom_ReadByte(ADD_NIGHT_HOUR) * 60 + Eeprom_ReadByte(ADD_NIGHT_MINUTE);
      if ( timenow < daytime || timenow > nighttime ) {
        daynight = 0;/// night
      } else {
        daynight = 1;/// day
      }
    } else { // manual daynight
      // manual daynight    
    //  serial1_string("DAY_NIGHT_M\n");
      daynight = ReadDayNightKey();
    }

    if ( Get_holiday() == 1) {
      daynight = 0;/// night
    }

    if ( daynight == 0 ) { // night mode
     // serial1_string("NIGHT\n");
      night_power(1);
      led_night(1);
      led_day(1);
    } else {              // day mode
     // serial1_string("DAY\n");
      pir_count = 0;
      mag_count = 0;
      night_power(0);
      led_day(0);
      led_night(0);
    }

    if ( daynight == 0 && daynight_prev == 1 ) { // day to night
      //#asm("cli") // disable interrupts
      send_daynight_status();
      lcd_clear();
      lcd_string_f("  DAY TO NIGHT");
      i = Eeprom_ReadByte(ADD_EXIT_DELAY);
      while ( i > 0) {
        lcd_cmd(0xC6);
        lcd_data(i / 100 + 48);
        lcd_num(i % 100);
        delay(600);
        i--;
      }

      //#asm("sei") // enable interrupts
      lcd_clear();
      display_counter = 0;
      for ( i = 0; i <= 7; i++ ) {
        remotezonestatus[i] = 0; // clear remote zones
      }
    }

    if ( daynight == 1 && daynight_prev == 0 ) { // night to day
      send_daynight_status();
      for ( i = 0; i <= 7; i++ ) {
        remotezonestatus[i] = 0;// clear remote zones
      }
    }
    daynight_prev = daynight;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if ( Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) { /// armed
      if ( tamper_flag == 1 ) {   
     // serial1_string("tamper_done\n");                    // TOKENNO0004
        //ad_temp(1);
        int_hoot(1);

        ArmTamper_delay_count++;
        if ( ArmTamper_delay_count == 8 ) {
          ArmTamper();                        // Send Tamper SMS
          //            sms_tamp_flag = 0;
          ArmTamper_delay_count = 0;
        }

        relay3_trig(1);
        /*if(trigger_flag==0)
          {
          int_hoot(int_hoot_bit);
          }*/
      } else { 
       // serial1_string("tamper_off\n");
        ArmTamper_delay_count = 0;
        //        sms_tamp_flag = 1;
        //ad_temp(0);
        int_hoot_bit = 0;
        relay3_trig(0);
        int_hoot(int_hoot_bit);
        if ( sounder_flag == 1 ) {
          int_hoot(1);
          hooter_trig(1);
        } else {
          int_hoot(0);
          hooter_trig(0);
        }
      }

      if ( trigger_flag == 1 ) {  
      //serial1_string("trigger_done\n");
        int_hoot(1);
        hooter_trig(1);
      } else {   
      //serial1_string("trigger_done\n");
        hooter_trig(0);
        relay1_trig(0);
        relay2_trig(0);
      }
    } else {  /// disarmed
      Sounder_Silent();
      relay3_trig(0);
      relay4_trig(0);
    }

    //////////////////////////////////////////////////////////////////////////////
    backlight = 0;
    tamper_flag = 0;
    //trigger_flag=0;
    short_flag = 0;
    open_flag = 0;


    if ( HOOTER_CUT == 1 && Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {
      serial1_string("hootCut\n");
      tamper_flag = 1;
      hooter_trig(1);
      /*=================For Remote Keypad============*/
      if ( hooter_cut_flag == 0) {
        //RemotekepadData( 8, 0 );  // Type =8 for hooter cut, zone nuber irrelevent
        //hooter_cut_flag = 1;
      }
      /*==============================================*/
    } else {
      if (hooter_cut_flag == 1) {
        //RemotekepadData(9,0);  // Type =8 for hooter present, zone nuber irrelevent
        //hooter_cut_flag = 0;
      }
    }

    if ( MOT_SIR_CUT == 0 && Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {
      tamper_flag = 1;
      if ( MAIN_ON == 0 ) {
        tamper_flag = 1;
        hooter_trig(1);
      }
    }

    a = Eeprom_ReadByte(ADD_NIGHT_TRIG);

    if ( tamp_switch == 1) {
      tamper_flag = 1;
      open_flag = 1;
    }

    // ************************************************************************************
    // ***************************** Zone Logic (Part 1) **********************************
    // ************************************************************************************

    // --------------------- SCAN : DAY ----------------------
    for ( i = 1; i <= 16; i++ )
    {  
      //sprintf(buffer, "%u", adc_valu);
     // serial1_string("zone");
    //  serial1_putc(i+'0'); 
    //  serial1_putc('\n');
      if ( GetZoneSettings(i) == TYPE_DAY && daynight == 0 )
      {
        continue;
      }
      /*if(GetZoneSettings(i)==TYPE_NIGHT && daynight==1)
        {
        continue;
        }*/
      if ( GetZoneSettings(i) == TYPE_ISO )
      {
        zonestates[i - 1] = STATE_ISO;
        //led_zone(i,0);
        led_zone(i, 1);
        delay(250);
        led_zone(i, 0);
        /*=================For Remote Keypad============*/

        //        RemotekepadData(1,i);

        /*==============================================*/
        continue;
      }

      /*if(zonestates[i-1]==STATE_OPEN && GetZoneSettings(i)==TYPE_NIGHT && a==1)
        {
        zonestates[i-1]= STATE_TRIG;
        //intrtrig = 1;
        trigger_zone_number = i;
        continue;
        }*/

      //if(zonestates[i-1]!=STATE_TRIG /*&& ((zonestates[i-1]!=STATE_OPEN) || a==0)*/ )

      Part_area = (i % 2);
      if ( Part_area == 0 ) {
        Part_area = (i / 2);
      } else {
        Part_area = Part_area + 1;
      }

      if ( zonestates[i - 1] != STATE_TRIG )
      {
        if ( ( i == PIR_ZONE ) || ( i == MAG_ZONE ) )
        {
          zoneval = GetZoneStatus_dedicated_nightzone(i);
          if ( ( zoneval == STATE_TRIG ) && ( GetZoneSettings(i) == TYPE_NIGHT ) && ( daynight == 1 ) ) {
            zoneval = STATE_NORMAL;
          }
        } else {
          zoneval = GetZoneStatus(i);
          if ( ( zoneval == STATE_TRIG ) && ( GetZoneSettings(i) == TYPE_NIGHT ) && ( daynight == 1) ) {
            zoneval = STATE_NORMAL;
          }
        }
        /*
          if((zonestates[i-1]== STATE_NORMAL)&& (zoneval==STATE_TRIG))
          {
          if(((i==MAG_ZONE)||(i==PIR_ZONE)) && )
          {
          zonestates[i-1] = STATE_NORMAL;
          }
          else
          {
          zonestates[i-1] = STATE_ACTIVATED;
          }
          }*/

        if (zoneval == STATE_TRIG)
        {
          zonestates[i - 1] = STATE_ACTIVATED;
        }

        if ( ( zoneval == STATE_NORMAL ) && ( i == MAG_ZONE ) )
        {
          zonestates[i - 1] = STATE_NORMAL;
        }            
        
        
// ======================================================================================================================================================================

/*
if ( zonestates[i - 1] == STATE_ACTIVATED )
{
    unsigned char delay_time;
    unsigned char current_second;
    unsigned int elapsed;
    unsigned int remaining;
    
    // Select appropriate delay based on zone and mode
    if ( i == 6 && daynight == 1 )  // Zone 6 in Night Mode
    {
        delay_time = Eeprom_ReadByte(ADD_ENTRY_DELAY);
    }
    else  // All other zones or Zone 6 in Day/Fire mode
    {
        delay_time = Eeprom_ReadByte(ADD_ZONE_1_DELAY + ( i - 1 ) );
    }
    
    current_second = RTC_GetSecond();
    
    // Check if this zone has been initialized in ACTIVATED state
    if ( zone_init_flag[i - 1] == 0 )  // Changed: Use dedicated init flag
    {
        // First time entering ACTIVATED state - store the start second
        zone_init_flag[i - 1] = 1;  // Mark as initialized
        zone_activated_time[i - 1] = RTC_GetSecond();
        zone_total_elapsed[i - 1] = 0;  // Reset total elapsed time
        zone_led_state[i - 1] = 0;  // Reset LED state
        zone_prev_elapsed[i - 1] = 0;
    }
    
    // Calculate elapsed time with minute rollover handling
    if ( current_second >= zone_activated_time[i - 1] )
    {
        current_elapsed_in_minute = current_second - zone_activated_time[i - 1];
        elapsed = zone_total_elapsed[i - 1] + current_elapsed_in_minute;
    }
    else
    {
        // Minute has rolled over (59→0)
        current_elapsed_in_minute = (60 - zone_activated_time[i - 1]) + current_second;
        zone_total_elapsed[i - 1] += (60 - zone_activated_time[i - 1]);
        zone_activated_time[i - 1] = 0;  // Reset for next minute
        elapsed = zone_total_elapsed[i - 1] + current_second;
    }
    
    remaining = delay_time - elapsed;
    
    // Check if delay time has elapsed
    if ( elapsed >= delay_time )
    {
        zonestates[i - 1] = STATE_TRIG;
        zone_init_flag[i - 1] = 0;  // Reset flag for next activation
        led_zone(i, 1);  // LED stays ON after trigger
    }
    else
    {
        // Toggle LED only when elapsed time changes (once per second)
        if ( elapsed != zone_prev_elapsed[i - 1] )
        {
            zone_led_state[i - 1] = ~ zone_led_state[i - 1];  // Toggle LED state
            led_zone(i, zone_led_state[i - 1]);
            zone_prev_elapsed[i - 1] = elapsed;
        }
        
        // Display "ZONE6 ACTIVE" with countdown on SECOND ROW (only for Zone 6)
//        if ( i == 6 )  // Only for Zone 6
//        {
//            lcd_cmd(0xC0);                    // Second row, column 0
//            lcd_clear_line();                 // Clear entire second row
//            lcd_cmd(0xC0);                    // Reset cursor to start of second row
//            lcd_string_f("ZONE6 ACTIVE");     // 13 characters
//            lcd_string_f("  ");               // 2 spaces
//            lcd_num_3digit(remaining);        // Display remaining time (3 digits)
//        }
    }
}
else
{
    unsigned char zone_delay;
    
    zone_delay = Eeprom_ReadByte(ADD_ZONE_1_DELAY + ( i - 1 ) );
    zone_delay_array_present[i - 1] = zone_delay;  // This stays unchanged
    zone_init_flag[i - 1] = 0;  // NEW: Reset init flag when leaving ACTIVATED
    led_zone(i, 0);
    zonestates[i - 1] = zoneval;
}
*/


if ( zonestates[i - 1] == STATE_ACTIVATED )
{
    unsigned char delay_time;
    unsigned char current_second;
    unsigned int elapsed;
    unsigned int remaining;
    unsigned char zone_type;
    
    // Read zone type from EEPROM
    zone_type = Eeprom_ReadByte(ADD_ZONE_1_TYPE + ( i - 1 ) );
    
    // Select appropriate delay based on zone type and mode
    if ( zone_type == TYPE_FIRE )  // Fire zone - always trigger immediately (no delay)
    {
        delay_time = 0;
    }
    else if ( i == 6 && daynight == 0 )  // Zone 6 in Day Mode (non-fire)
    {
        delay_time = Eeprom_ReadByte(ADD_ENTRY_DELAY);
    }
    else  // All other zones or Zone 6 in Night mode
    {
        delay_time = Eeprom_ReadByte(ADD_ZONE_1_DELAY + ( i - 1 ) );
    }
    
    current_second = RTC_GetSecond();
    
    // Check if this zone has been initialized in ACTIVATED state
    if ( zone_init_flag[i - 1] == 0 )  // Changed: Use dedicated init flag
    {
        // First time entering ACTIVATED state - store the start second
        zone_init_flag[i - 1] = 1;  // Mark as initialized
        zone_activated_time[i - 1] = RTC_GetSecond();
        zone_total_elapsed[i - 1] = 0;  // Reset total elapsed time
        zone_led_state[i - 1] = 0;  // Reset LED state
        zone_prev_elapsed[i - 1] = 0;
    }
    
    // Calculate elapsed time with minute rollover handling
    if ( current_second >= zone_activated_time[i - 1] )
    {
        current_elapsed_in_minute = current_second - zone_activated_time[i - 1];
        elapsed = zone_total_elapsed[i - 1] + current_elapsed_in_minute;
    }
    else
    {
        // Minute has rolled over (59→0)
        current_elapsed_in_minute = (60 - zone_activated_time[i - 1]) + current_second;
        zone_total_elapsed[i - 1] += (60 - zone_activated_time[i - 1]);
        zone_activated_time[i - 1] = 0;  // Reset for next minute
        elapsed = zone_total_elapsed[i - 1] + current_second;
    }
    
    remaining = delay_time - elapsed;
    
    // Check if delay time has elapsed
    if ( elapsed >= delay_time )
    {
        zonestates[i - 1] = STATE_TRIG;
        zone_init_flag[i - 1] = 0;  // Reset flag for next activation
        led_zone(i, 1);  // LED stays ON after trigger
    }
    else
    {
        // Toggle LED only when elapsed time changes (once per second)
        if ( elapsed != zone_prev_elapsed[i - 1] )
        {
            zone_led_state[i - 1] = ~ zone_led_state[i - 1];  // Toggle LED state
            led_zone(i, zone_led_state[i - 1]);
            zone_prev_elapsed[i - 1] = elapsed;
        }
        
        // Display "ZONE6 ACTIVE" with countdown on SECOND ROW (only for Zone 6)
//        if ( i == 6 )  // Only for Zone 6
//        {
//            lcd_cmd(0xC0);                    // Second row, column 0
//            lcd_clear_line();                 // Clear entire second row
//            lcd_cmd(0xC0);                    // Reset cursor to start of second row
//            lcd_string_f("ZONE6 ACTIVE");     // 13 characters
//            lcd_string_f("  ");               // 2 spaces
//            lcd_num_3digit(remaining);        // Display remaining time (3 digits)
//        }
    }
}
else
{
    unsigned char zone_delay;
    
    zone_delay = Eeprom_ReadByte(ADD_ZONE_1_DELAY + ( i - 1 ) );
    zone_delay_array_present[i - 1] = zone_delay;  // This stays unchanged
    zone_init_flag[i - 1] = 0;  // NEW: Reset init flag when leaving ACTIVATED
    led_zone(i, 0);
    zonestates[i - 1] = zoneval;
}



// ======================================================================================================================================================
        if ( zoneval == STATE_OPEN && GetZoneSettings(i) == TYPE_NIGHT && a == 1 && daynight == 0 )
        {
          zonestates[i - 1] = STATE_TRIG;
          //intrtrig = 1;
          //trigger_zone_number = i;
          //continue;
        }

        if ( (zonestates[i - 1] == STATE_TRIG ) && ( GetZoneSettings(i) == TYPE_NIGHT ) && ( daynight == 1 ) )
        {
          zonestates[i - 1] = STATE_NORMAL;
        }


        // ***************************** CASE : ZONE STATE : TRIGGER ************ (0)/(3)
        if ( zonestates[i - 1] == STATE_TRIG )
        { // TOKENNO0002

          sounder_count = 0;
          trigger_reset_flag = 1;
          sounder_time_flag = 0; // RESET timeout calculation for the new trigger
          lcd_back(1);

          // SUB-CASE ZONE SETTINGS FIRE (0)/(1)
          if ( GetZoneSettings(i) == TYPE_FIRE )
          {
            led_zone(i, 1);
            if ( Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {  // TOKENNO0003
              relay1_trig(1);
              trigger_flag = 1;
              sounder_flag = 1; //for int hoot
            } else {
              if ( Eeprom_ReadByte(ADD_ARM_DISARM_P1 + ( Part_area - 1 ) ) != 0) {
                relay1_trig(1);
                trigger_flag = 1;
                sounder_flag = 1; //for int hoot
              } else {
                // If system is disarmed and no part-arm logic, ensure flags stay 0
                trigger_flag = 0;
                sounder_flag = 0;
              }
            }
            AddLog(i, 0);
            firetrig = 1;
          }

          // SUB-CASE ZONE SETTINGS NIGHT or DAY (1)/(1)
          if ( GetZoneSettings(i) == TYPE_DAY || GetZoneSettings(i) == TYPE_NIGHT )
          {
            led_zone(i, 1);
            if ( Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {            // TOKENNO0006
              relay2_trig(1);
              trigger_flag = 1;
              sounder_flag = 1; //for int hoot
              if (GetZoneSettings(i) == TYPE_NIGHT ) {
                relay4_trig(1);
              }
            } else {
              if ( Eeprom_ReadByte(ADD_ARM_DISARM_P1 + ( Part_area - 1 ) ) != 0) {
                relay2_trig(1);
                trigger_flag = 1;
                sounder_flag = 1; //for int hoot
                if ( GetZoneSettings(i) == TYPE_NIGHT ) {
                  relay4_trig(1);
                }
              } else {
                // If system is disarmed and no part-arm logic, ensure flags stay 0
                trigger_flag = 0;
                sounder_flag = 0;
              }
            }
            AddLog( i, 1 );
            intrtrig = 1;
          }
          display_counter = (5 * i);
          trigger_zone_number = i;
        }

        // ***************************** CASE : ZONE STATE : OPEN ************************ (1)/(3)
        if ( zonestates[i - 1] == STATE_OPEN )
        {
          if ( ( i == PIR_ZONE ) || ( i == MAG_ZONE ) ) {
            zoneval = GetZoneStatus_dedicated_nightzone(i);
            if ( zoneval != STATE_OPEN ) {
              continue;
            }
          }
          //open_flag=1;
          if ( GetZoneSettings(i) != TYPE_ISO ) {
            open_flag = 1;
            zone_open_isolate = zone_delay_iso_open_array[i - 1];
            if ( zone_open_isolate < 201 ) {
              //zone_open_isolate++;
              zone_delay_iso_open_array[i - 1] = (zone_delay_iso_open_array[i - 1] + 1 );
              //lcd_cmd(0x81);
              //lcd_3_num(zone_delay_iso_open_array[i-1]);
            }
            if ( zone_open_isolate == 200 ) {
              Eeprom_WriteByte( ADD_ZONE_1_TYPE + ( i - 1 ), 3 );
              zone_delay_iso_open_array[i - 1] = 0;
              //zone_open_isolate = 0;
              open_iso = 1;
              //open_iso_fun()
            }
          }
        } else {
          zone_delay_iso_open_array[i - 1] = 0;
        }

        // ***************************** CASE : ZONE STATE : SHORT ******************************* (2)/(3)
        if ( zonestates[i - 1] == STATE_SHORT )
        {
          short_flag = 1;
        }

        if (/*zonestates[i-1]!=STATE_NORMAL*/trigger_flag == 1 || tamper_flag == 1 )
        {
          backlight = 1;
        }

        // ***************************** CASE : ZONE STATE : OPEN OR SHORT ************************ (3)/(3)
        if ( zonestates[i - 1] == STATE_OPEN || zonestates[i - 1] == STATE_SHORT )
        {
          tamper_flag = 1;
          if (display_tamper_flag == 0) {
            display_counter = ( i * 5 );
            display_tamper_flag = 1;
          }
        }
      } else {      // already triggered
        led_zone(i, 1);
        zonestates[i - 1] = STATE_TRIG;
        continue;
      }
    }

    // ************************************* END **************************************
    // *************************** Zone Logic (Part 2) ********************************
    // --------------------- SCAN : NORMAL ----------------------
    for ( i = 1; i <= 16; i++ )
    {
      if ( zonestates[i - 1] == STATE_NORMAL ) {
        Healthy_Zone++;
      } else if ( zonestates[i - 1] == STATE_OPEN ) {
        /*=================For Remote Keypad================*/

        //        RemotekepadData(2,i);
        /*==================================================*/
        TamperFlagRemoteKeypad = 0;
      } else if ( zonestates[i - 1] == STATE_SHORT ) {
        /*=================For Remote Keypad================*/
        //        RemotekepadData(3,i);
        /*==================================================*/
        TamperFlagRemoteKeypad = 0;
      }
    }

    if (Healthy_Zone == 16)
    {
      display_tamper_flag = 0;
      /*=================For Remote Keypad================*/
      if (TamperFlagRemoteKeypad == 0) {
        //        RemotekepadData(7,0);
        TamperFlagRemoteKeypad = 1;
      }
      /*==================================================*/
    }
    /////////////////////////////////////////////////////////////////////////////

    if ( open_iso == 1 )
    {
      backlight = 0;
      lcd_back(backlight);
      delay(200);
      backlight = 1;
      lcd_back(backlight);
    }

    led_open(open_flag);
    led_short(short_flag);
    lcd_back(backlight);

    //if((trigger_reset_flag==1)&&(sounder_time_flag==0))
    if ( ( trigger_reset_flag == 1 ) && ( sounder_time_flag == 0 ) )
    {
      d = Eeprom_ReadByte(ADD_SOUNDER_TIME);
      if ( d < 1 || d > 120 ) {
        d = 3;
      }
      hooterofftime = ( (unsigned int)RTC_GetHour() * 60 ) + (unsigned int)RTC_GetMinute() + d;
      SounderOffSec = RTC_GetSecond();
      if ( hooterofftime >= 1440 ) {
        hooterofftime = hooterofftime - 1440;
      }
      sounder_time_flag = 1;
    }
    //if(trigger_reset_flag==1)
    if ( trigger_reset_flag == 1 )
    {
      unsigned int current_total_min;
      current_total_min = (unsigned int)RTC_GetHour() * 60 + (unsigned int)RTC_GetMinute();
      
      if ( current_total_min == hooterofftime ) {
        if ((unsigned char)RTC_GetSecond() >= SounderOffSec) {
            trigger_reset_flag = 0;
            sounder_time_flag = 0;
            Sounder_Silent();
            trigger_flag = 0;
            Reset();
        }
      }
      else if (hooterofftime < d && current_total_min < d && current_total_min > hooterofftime) {
          // Rollover case: e.g., trigger at 23:58, d=5. hooterofftime = 00:03.
          // current_total_min is now say 00:04. 00:04 > 00:03 AND 00:04 < 5.
            trigger_reset_flag = 0;
            sounder_time_flag = 0;
            Sounder_Silent();
            trigger_flag = 0;
            Reset();
      }
      else if (hooterofftime >= d && current_total_min > hooterofftime) {
          // Normal case: e.g., trigger at 10:00, d=5. hooterofftime = 10:05.
          // current_total_min is now 10:06. 10:06 > 10:05 AND 10:05 >= 5.
            trigger_reset_flag = 0;
            sounder_time_flag = 0;
            Sounder_Silent();
            trigger_flag = 0;
            Reset();
      }
    }

    /*=================================AREA SELECTION==============================*/

    Part_area = ( trigger_zone_number % 2 );
    if (Part_area == 0) {
      Part_area = ( trigger_zone_number / 2 );
    } else {
      Part_area = Part_area + 1;
    }

    /*============================================================================*/
     check_serial();

    /////////////////////////////////////////////////////////////////////////////
    if ( (firetrig == 1 || intrtrig == 1) && Eeprom_ReadByte(ADD_ARM_DISARM) != 0 ) {
      display_counter = ( trigger_zone_number * 5 );
      if (Eeprom_ReadByte(ADD_ARM_DISARM) != 0 || Eeprom_ReadByte(ADD_ARM_DISARM_P1 + (Part_area - 1)) != 0) {
        int_hoot(1);
        hooter_trig(1);
      } else {
        int_hoot(0);
        hooter_trig(0);
      }
      firetrig = 0;
      intrtrig = 0;
    } else if (firetrig == 1 || intrtrig == 1) {
      firetrig = 0;
      intrtrig = 0;
      Sounder_Silent();
    }
    check_serial();

    /////////////////////////////////////////////////////////////////////////////////
    lcd_re_init_count++;
    if (lcd_re_init_count == 150)
    {
      lcd_re_init_count = 0;
      lcd_init();
      lcd_cmd(0xC0);
      ShowTime();
    }

    ShowCenteredName();
    check_serial();
    lcd_cmd(0x8F);
    //lcd_data(CUSTOM_CHAR_BATTERY);

    lcd_data( MAIN_ON == 1 ? CUSTOM_CHAR_BATTERY : CUSTOM_CHAR_MAINS);
    if ( MAIN_ON == 0 ) {
      led_mains(1);
      led_battery(0);
    } else {
      led_mains(0);
      led_battery(1);
    }

    // *********************************************************************************
    // ************************** Display State Machine ********************************
    // *********************************************************************************
    // CASE : 0
    if ( display_counter <= 4 )
    {
      lcd_cmd(0xC0);
      ShowTime();
      lcd_clear_line();

      display_counter++;

      // CASE : 1
    }
    if ( display_counter <= 84 ) { 
     RS485_EN();
      if ( zonestates[ ( display_counter / 5 ) - 1 ] == STATE_NORMAL )
      {
        display_counter += 5;
        //lcd_cmd(0xCB);
        //lcd_num(Healthy_Zone);
      } else {
        if ( display_counter % 5 == 0 )
        {
          if ( display_counter / 5 <= 16 )
          {
            LcdSetCursor(1, 0);
            lcd_clear_line();
            LcdSetCursor(1, 1);
            lcd_string_f("ZONE");
            strcpy(DataForSend, " ZONE");                              // TOKENNO0001
            if ( (display_counter / 5 ) <= 9 ) { 
              LcdSetCursor(1, 5);
              lcd_data( display_counter / 5 + 48 );
              DataForSend[5] = display_counter / 5 + 48;
            } else {
              LcdSetCursor(1, 5);
              lcd_num( display_counter / 5 );
              //DataForSend[5] = display_counter / 5 + 48;
              
              /*
                // Added on 11-08-2022 to Active Hooter for Wireless Zone Activation
                if(display_counter / 5 == 13)
                {
                  int_hoot(1);
                  hooter_trig(1);
                  delay(3000);
                  int_hoot(0);
                  hooter_trig(0);
                }
              */
            }
          } else {
            lcd_num(display_counter / 5);
          }
          LcdSetCursor(1, 8);
          //lcd_data(' ');
          lcd_string_f( str_states[zonestates[ ( display_counter / 5 ) - 1 ] ] ); 
          DataForSend[6] = ' ';
          for(i=0;i<7;i++){
            DataForSend[7+i]= str_states[zonestates[ ( display_counter / 5 ) - 1 ]][i];
          }
          // Added on 11-08-2022 to Active Hooter for Wireless Zone Activation
          if (display_counter / 5 == 13 && Eeprom_ReadByte(ADD_ARM_DISARM) != 0 )
          {
            relay2_trig(1);
            lcd_back(1);
            int_hoot(1);
            hooter_trig(1);
            delay(3000);
            int_hoot(0);
            hooter_trig(0);
            //                        Auto_dialer_fun(13,1);
            relay2_trig(0);
          }
          else if (display_counter / 5 == 13)
          {
            Sounder_Silent();
          }
          lcd_clear_line();
          lcd_cmd(0xC0);
        }
        display_counter++;
      }         
//      serial_string(DataForSend);
      RS485_DN();
      // CASE : 2
    } else if ( display_counter <= 99 ) {      //program for tamper switch
      if ( tamp_switch == 1) {
        if ( display_counter == 95 ) {
//          lcd_cmd(0xC0);
          LcdSetCursor(1,0);
          lcd_string_f("TAMP ZONE TRIGGR");
          RS485_EN();  
          //_delay_ms(10);
          serial_string_f("TAMP ZONE TRIGGR");
          //_delay_ms(10);
          RS485_DN();
          tamper_flag = 1;
          led_open(1);
          lcd_clear_line();
          lcd_cmd(0xC0);
        }
        display_counter++;
      } else {
        display_counter = 100;
      }
      // CASE : 5
    } else if ( display_counter <= 104 ) {
      if ( Healthy_Zone == 16) {
        if (display_counter == 100) {  
        //RS485_EN();  
        //_delay_ms(10);
        //serial_string_f("ALL ZONE HEALTHY");
        //_delay_ms(10);
        //RS485_DN();
//          lcd_cmd(0xC0);
          LcdSetCursor(1,0);
          lcd_string_f("ALL ZONE HEALTHY");
        }
        display_counter++;
      } else {
        display_counter = 0;
      }
      // CASE : 6
    } else {
      display_counter = 0;
    }

    Healthy_Zone = 0;  // setting the value for next step count;
    /*KEYPAD_SEL_A=0;
      KEYPAD_SEL_B=0;
      KEYPAD_SEL_C=0;
      delay(50);
      //
      lcd_clear();
      lcd_num(KEYPAD_IN);
      delay(500);
      lcd_clear();
    */
    //key=GetKey();
    //manual_silent();
    //manual_reset();
    check_serial();
    if ( GetKey() == KEY_DOWN ) {              //  ON-OFF FUNCTION THROUGH KEYPAD
      delay(1000);
      if ( GetKey() == KEY_DOWN ) {
        lcd_clear();
        backlight = 1;
        lcd_back(backlight);
        menu_flag = 1;
        password_match = PasswordDlg_on_off(0);
        if ( password_match == 0 ) {
          lcd_string_f(" ACCESS DENIED");
          delay(1000);
          lcd_clear();
          menu_flag = 0;
          //return;
        } else if ( password_match == 1 ) {
          SendLatchData(1, latch1_data);
          SendLatchData(2, latch2_data);
          SendLatchData(3, latch3_data);
          SendLatchData(4, latch4_data);
          ADCSRA = 0x00;
          all_led_off();
          int_hoot(0);
          hooter_trig(0);
          relay1_trig(0);
          relay2_trig(0);
          lcd_cmd(0x01);
          delay(10);

          while (1) {
            led_syson(1);
            _delay_ms(1000);
            led_syson(0);
            _delay_ms(1000);
            if ( GetKey() == KEY_DOWN ) {              //  ON OFF FUNCTION THROUGH KEYPAD
              delay(1000);
              if ( GetKey() == KEY_DOWN ) {
                Reset();
              }
            }
          }
        }
        display_counter = 0;
        lcd_cmd(0x80);
      }
    }

    if ( key_index == 3 ) {
      menu_flag = 1;
      int_hoot_bit = 0;
      int_hoot(int_hoot_bit);

      delay(500);
      lcd_back(1);
      //send_log_email();
      //SelectGSM();

      //TIMSK=0x00; // timers start
      TIMSK0 = 0x00;
      // Timer/Counter 1 Interrupt(s) initialization
      TIMSK1 = 0x00;
      // Timer/Counter 2 Interrupt(s) initialization
      TIMSK2 = 0x00;
      // Timer/Counter 3 Interrupt(s) initialization
      TIMSK3 = 0x00;


      SetupDlg();
      menu_flag = 0;
      //TIMSK=0x41; // timers start
      TIMSK0 = 0x01;
      // Timer/Counter 1 Interrupt(s) initialization
      TIMSK1 = 0x00;
      // Timer/Counter 2 Interrupt(s) initialization
      TIMSK2 = 0x01;
      // Timer/Counter 3 Interrupt(s) initialization
      TIMSK3 = 0x00;
      display_counter = 0;
      lcd_cmd(0x80);
      lcd_back(0);
      key_index = 0;
    }
    check_serial();
    if ( Read_Tamper_key() == 0 ) {
      tamp_switch = 1;
      backlight = 1;
      tamper_flag = 1;
      led_open(1);
      open_flag = 1;
      if ( tapmer_zone_trigger == 0 ) {
        display_counter = 95;
        lcd_cmd(0x80);
        tapmer_zone_trigger = 1;
        //        sms_tamp_flag = 1;
        /*=================For Remote Keypad (BOI CODE)============*/

        //        RemotekepadData(10,0);  // Type =8 for hooter cut zone nuber irrelevent

        /*==============================================*/
      }
    } else {
      tamp_switch = 0;
      if (tapmer_zone_trigger == 1) {
        /*=================For Remote Keypad (BOI CODE)============*/

        //        RemotekepadData(11,0);  // Type =8 for hooter cut zone nuber irrelevent

        /*==============================================*/
      }
      tapmer_zone_trigger = 0;
    }

    /*===========================================CMS DATA SENDING FOR DIFFRENT EVENTS ON PI==============*/
   check_serial();
    for ( i = 1; i <= 16; i++ ) {
      if ( zonestates[i - 1] == STATE_NORMAL ) {
        CmsBitChk = 0x0000;
        if ( ( (CmsBitChk |= 1 << ( i - 1 ) ) & CmsSetReset ) != 0x0000 ) {
          CmsSetReset &= ~(1 << (i - 1));
          //          SendAlarmCms(i,4);       ///Alaarm Restore Message
        }
      } else if ( zonestates[i - 1] == STATE_TRIG ) {
        if ( GetZoneSettings(i) == TYPE_FIRE ) {
          CmsBitChk = 0x0000;
          if ( ( ( CmsBitChk |= 1 << (i - 1) ) & CmsSetReset ) == 0 ) {
            //            SendAlarmCms(i,1);
            Eeprom_WriteByte(7000, 0x88);
            ////////////////////////////////////////////////////////////
            /*
              HeartBeatTime=(RTC_GetHour()*60)+(RTC_GetMinute()+d);
              if(HeartBeatTime>1439)
              {
              HeartBeatTime=HeartBeatTime-1439;
              }
              NextBeat = HeartBeatTime+2;
            */
            ///////////////////////////////////////////////////////////////
          }
          CmsSetReset |= 1 << (i - 1);
        } else if ( (GetZoneSettings(i) == TYPE_NIGHT ) || ( GetZoneSettings(i) == TYPE_DAY) ) {
          CmsBitChk = 0x0000;
          if ( ( ( CmsBitChk |= 1 << ( i - 1 ) ) & CmsSetReset ) == 0 ) {
            Eeprom_WriteByte(7002, 0x88);
            //            SendAlarmCms(i,0);
            /////////////////////////////////////////////////////////////////
            /*
              HeartBeatTime=(RTC_GetHour()*60)+(RTC_GetMinute()+d);
              if(HeartBeatTime>1439)
              {
              HeartBeatTime=HeartBeatTime-1439;
              }
              NextBeat = HeartBeatTime+2;
            */
            ////////////////////////////////////////////////////////////////////
          }
          CmsSetReset |= 1 << ( i - 1 );
        }
      } else if ( zonestates[i - 1] == STATE_OPEN ) {
        CmsBitChk = 0x0000;
        if ( ( ( CmsBitChk |= 1 << ( i - 1 ) ) & CmsSetReset ) == 0 ) {
          //                SendAlarmCms(i,2);
        }
        CmsSetReset |= 1 << ( i - 1 );
      } else if ( zonestates[i - 1] == STATE_SHORT ) {
        CmsBitChk = 0x0000;
        if ( ( ( CmsBitChk |= 1 << ( i - 1 ) ) & CmsSetReset ) == 0 ) {
          //          SendAlarmCms( i , 3 );
        }
        CmsSetReset |= 1 << ( i - 1 );
      }
      if ( zonestates[i - 1] == STATE_ISO ) {
        //Nothing Implemented
      }
    }

    check_serial();
    /*=============================================CMS HEART BEAT FUNCTION===============================*/

    HeartBeatTime = ( RTC_GetHour() * 60 ) + ( RTC_GetMinute() );
    //if(NextBeat>1439)
    if ( ( NextBeat - HeartBeatTime ) > 1400 ) {
      //NextBeat=NextBeat-1439;
      //      SendAlarmCms(0,15);
      HeartBeatTime = ( RTC_GetHour() * 60 ) + ( RTC_GetMinute() );
      NextBeat =  HeartBeatTime + Eeprom_ReadByte(HEARTBEAT_TIME_INT);
    }
    if ( HeartBeatTime >= NextBeat ) {
      //      SendAlarmCms(0,15);  // Sending heartbeat on regular interval ; interval = 1mint
      NextBeat =  HeartBeatTime + Eeprom_ReadByte(HEARTBEAT_TIME_INT);
    }
    send_arm_status();
    send_tamper_status();
    send_power_status();
    send_battery_cut_status();
    send_battery_low_status();
    check_serial();
    send_zone_status();
    send_daynight_status();
    send_battery_voltage(); 
    
    if(zone_mode_change==1){
     send_initial_zone_status();
     zone_mode_change=0;
    }
    if (Keypad_GetKey() == 'B') {
      CommonTest();
    }
    /*====================================================================================================*/ 
	if (strncmp(rcv, "hello1", 6) == 0) {
		
		RS485_EN();
		serial_string(rcv);
		RS485_DN();
		
		for(i=0;i<16;i++){
			rcv[i]='\0';
		}
	}
    if (strncmp(rcv, "&&", 2) == 0) {
                    int a=1;
                    RS485_EN();
                    serial_string("menu_in");
					serial_putc('\n');
					_delay_ms(10);
                    RS485_DN(); 
                    
                    while(a==1){
                     check_serial();
					 //relay1_trig(1);
                     //_delay_ms(1);
//===========================================================Menu Out===========================================
                     if (strncmp(rcv, "menu_out", 8) == 0) {
					 _delay_ms(100);	 
                     RS485_EN();
                     serial_string("menu_out\n");
					 //serial_putc('\n');
					 _delay_ms(20);
                     RS485_DN();
                     for(i=0;i<50;i++){
                      rcv[i]='\0';
                     }
					 a=0;
                    }
//============================================================Day/Night Mode========================================
					if (strncmp(rcv, "D/N_Mode?", 9) == 0) {
					_delay_ms(100);
					RS485_EN();
					serial_string("MANUAL\n");
					//serial_putc('\n');
					_delay_ms(10);
					RS485_DN();
					
					for(i=0;i<50;i++){
						rcv[i]='\0';
					}
					}
//=============================================================Arm/Disarm Mode========================================
                    if (strncmp(rcv, "A/D_Mode?", 9) == 0) {
	                    _delay_ms(100);
	                    RS485_EN();
	                    serial_string("AUTOMATIC\n");
	                    //serial_putc('\n');
	                    _delay_ms(10);
	                    RS485_DN();
	                    
	                    for(i=0;i<50;i++){
		                    rcv[i]='\0';
	                    }
                    }
//==============================================================ZoneSetting============================================
                    if (strncmp(rcv, "ZoneMode?", 9) == 0) {
	                    _delay_ms(100);
	                    RS485_EN();
	                    serial_string("NIGHT\n");
	                    //serial_putc('\n');
	                    _delay_ms(10);
	                    RS485_DN();
	                    
	                    for(i=0;i<50;i++){
		                    rcv[i]='\0';
	                    }
                    }
//===============================================================Entry Time==============================================
					if (strncmp(rcv, "EntryTime?", 10) == 0) {
						_delay_ms(100);
						RS485_EN();
						serial_string("EntryTime:5\n");
						//serial_putc('\n');
						_delay_ms(10);
						RS485_DN();
	
						for(i=0;i<50;i++){
							rcv[i]='\0';
						}
					}
                   }
				   
         } else{
			 RS485_EN();
			 serial_string(rcv);
			 serial_putc('\n');
			 _delay_ms(10);
			 RS485_DN();
			 _delay_ms(100);
		 }
		 
  }
}


