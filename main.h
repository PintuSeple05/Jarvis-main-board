#ifndef _MAIN_H_
#define _MAIN_H_
//#define F_CPU 8000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>

void SetZoneMode(unsigned char zone, char mode_char, unsigned char uart_num);
void send_ok(unsigned char uart_num);
void Sounder_Silent();

extern unsigned char c_hour, c_min, c_sec, c_day, c_month, c_year;
extern unsigned char c_arm_status;
extern unsigned char c_name[17];
extern unsigned char name_refresh_flag;

#include "lcd_lib.h"
#include "rtc_lib.h"
#include "globals.h"
#include "serial_lib.h"
//#include "GSM.h"
#include "remote_keypad.h"
//#include "system_control.h"
//#include "MENU.h"


//#define KEYPAD_IN   PINA.0
#define MAIN_ON     (PINA & (1 << PA3))
#define HOOTER_CUT  (PINA & (1 << PA1))
#define MOT_SIR_CUT (PINA & (1 << PA4))
#define OUT_EN      PA6     // Shift Register 74HC595 OE - 13 
#define D4          PB4
#define D5          PB5
#define D6          PB6     // Shift Register 74HC595 RCLK - 12
#define D7          PB7     // Shift Register 74HC595 SER - 14
#define LATCH_1     PC2
#define LATCH_2     PC3
#define LATCH_3     PC4
#define LATCH_4     PC7     // Shift Register 74HC595 SRCLK - 12

//===================================================

void send_power_status(void);
void send_hooter_cut_status(void);
void send_m_hooter_cut_status(void);
void send_battery_low_status(void);
void send_battery_voltage(void);
unsigned int read_bat_volt(void);
unsigned int read_adc(unsigned char adc_input);
void SendNameToUARTs(void);
void SendAddressToUARTs(void);
void SendPhoneNumbersToUARTs(void);
void SendEmailIDToUARTs(void);
void SendNameAddressToUARTs(void);
void ShowCenteredName(void);

//===================================================

//#define GSM_EN          PORTA.0
//#define AUTODIALER_EN   PORTC.1
//#define REC             PORTC.0
//#define INT_APR_PLAY    PORTA.2
//#define FIRE_APR_PLAY   PORTB.3
#define LCD_BACK        PD5
#define BAT_CUT_SWITCH  PD7
//#define CONTROL         PORTD.6
//#define KEYPAD_SEL_A    D4
//#define KEYPAD_SEL_B    D5
//#define KEYPAD_SEL_C    D6
#define SEL_ZONE_A      D4
#define SEL_ZONE_B      D5
#define SEL_ZONE_C      D6
#define DS              D7
#define SHCP            D6
// all latch buffer variables


unsigned char   latch1_data = 0, latch2_data = 0, latch3_data = 0, latch4_data = 0;
                
#define PIR_ZONE    7
#define MAG_ZONE    6
#define REF_1       150
#define REF_2       340
#define REF_3       422
#define STATE_NORMAL    0
#define STATE_TRIG      1
#define STATE_OPEN      2
#define STATE_SHORT     3
#define STATE_ISO       4   //alredy triggerd but output not yet started
#define STATE_ACTIVATED 5

unsigned char zonestates[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char remotezonestatus[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//extern volatile bool sms_flag = 0;
 bool sounder_flag;
 bool menu_flag;
/*===========================*/
//unsigned char INTR_TRIG = 0;
/*============================*/
//unsigned int pass;
unsigned int timenow_hooter = 0, hooterofftime;
unsigned char SounderOffSec = 0;
unsigned char key_index;
unsigned char trigger_reset_flag = 0;
unsigned int GetZoneStatus(unsigned char a);
bool int_hoot_bit = 0;
//bit keypad_present = 0;
extern volatile char serial_buff[512];
extern volatile unsigned char ch_start,ch_end,num_start,num_end;
extern volatile unsigned char b_direct_buffer;
extern volatile unsigned int buffer_len;
extern volatile unsigned int buff_index;
unsigned int day_arr[30];
unsigned int month_arr[30];
unsigned int year_arr[30];
bool tamper_flag=0,trigger_flag=0; //sms_tamp_flag=0;
bool backlight=0,daynight=0,daynight_prev=0,short_flag=0,open_flag=0,sounder_time_flag=0;;


//flash unsigned char  lcd_battery[]={0xe, 0x1f, 0x11, 0x1f, 0x11, 0x1f, 0x11, 0x1f};
const unsigned char PROGMEM lcd_battery[]={0xe,0x1f,0x11,0x1f,0x1f,0x11,0x1f};
const unsigned char PROGMEM lcd_mains[]={0xa, 0xa, 0x1f, 0x1f, 0x1f, 0xe, 0x4,0x3};
const unsigned char PROGMEM lcd_network1[]={0x1F, 0x15, 0x0E, 0x04, 0x04, 0x04, 0x04, 0x04}; // Tower symbol (Y-shape antenna)
const unsigned char PROGMEM lcd_network2[]={0x01, 0x01, 0x01, 0x05, 0x05, 0x15, 0x15, 0x15}; // Strength symbol (3 sticks: heights 3, 5, 8)

const char *months[] = {"JAN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"};
const char  *str_states[] = {"NORMAL","TRIG.","OPEN","SHORT","ISOLATE","ACTIVE"};
const char  *str_menu[] = {"  SETUP USER  ","   SET T/D    "," SET DAY TIME ","SET NIGHT TIME","DAY/NIGHT MODE","  ARM/DISARM  ","ARM/DISARM MOD","   ARM TIME   ","  DISARM TIME "," ZONE SETTINGS","  DELAY SETUP ","  ENTRY TIME  ","   EXIT TIME  "," SOUNDER TIME ","NIGHT CUT TRIG","   WALK TEST  ", "   SET NAME   ","  SET ADDRESS ","   VIEW LOGS  ","  ERASE LOGS  "," HOLIDAY LIST ","  CLEAR LIST  ","CHANGE PASSWD "," FACTORY RESET","KEYPAD SETTING","  CMS SETTING "," ADD FIRE NUM "," ADD INTR NUM "," FIRE CALL SET"," INTR CALL SET"," TMPR CALL SET"," FIRE SMS SET "," INTR SMS SET "," TMPR SMS SET "," ADD EMAIL ID "," FIRE MSG REC "," FIRE MSG PLY "," INTR MSG REC "," INTR MSG PLY "," TMPR MSG REC "," TMPR MSG PLY ","   MSG4 REC   ","   MSG4 PLY   "};

#define   MENU_NUM          43  
#define   LEFT_KEY          0
#define   RIGHT_KEY         1
#define   UP_KEY            2
#define   DOWN_KEY          3
#define   MENU_KEY          4                                                                           
#define   ENTER_KEY         5

#define DAY_NIGHT_KEY  (PIND & (1 << PD6))
#define TAMP_SWITCH    (PIND & (1 << PD4))


unsigned char l_sys_on=0;
unsigned char l_mains=0;
unsigned char l_battery=0;
unsigned char l_battery_cut=0;
unsigned char l_battery_low=0;
unsigned char l_battery_charging=1;
unsigned char l_siren_cut=0;
unsigned char l_m_siren_cut=0;
unsigned char l_day_mode=0;
unsigned char l_night_mode=0;
unsigned char l_armed     = 0;
unsigned char l_disarmed  =0;  



#define ADC_VREF_TYPE 0x40
// Read the AD conversion result
#define BAT_CUT_VOLT    700
#define BAT_FULL_VOLT   672
#define BAT_TRIP_VOLT    570
#define BAT_LOW_VOLT     600

//unsigned char mains_on = 2;
//unsigned int last_sec = 60;

unsigned char GetKey();
void delay(unsigned int a);
void Reset();

void int_hoot(unsigned char a);
void hooter_trig(unsigned char a);
void relay1_trig(unsigned char a);
void relay2_trig(unsigned char a);

//extern void SendFormattedSMS(char * number,flash char * event);
//extern void SendSMS(char * number ,char * str);
//extern unsigned char GetGSMEnabled(); // 0 if disabled else enabled

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//void GetNumber(unsigned char index,char * number);
//void ArmIntr();
//void Auto_dialer_fun(unsigned char zone);
//void Auto_dialer_fun(unsigned char zone,unsigned char event);


#endif

#ifndef _MENU_INCLUDED_
#define _MENU_INCLUDED_

unsigned int GetPassword();
unsigned int GetPasswordUser(unsigned char user_index);
// 0=wrong, 1=ok,2=cancel      //use = 1 for MENU; use = 2 for RESET; use = 3 for SILENT
unsigned char PasswordDlg(unsigned char fun);
// 0=wrong, 1=ok, 2=cancel      //x = 0 for SYSTEM OFF CODE; x = 1 for SYSTEM ON CODE
unsigned char PasswordDlg_on_off(unsigned char x);

/*
unsigned char which_key(unsigned char bus_value,unsigned char key);
unsigned char GetKey();
*/

#endif