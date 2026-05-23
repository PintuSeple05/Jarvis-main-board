
#include "remote_keypad.h"
#include "globals.h"

unsigned char GetRPTREnabled()// 0 if disabled else enabled
{    
    return Eeprom_ReadByte(ADD_RPTR_EN);
}

void Repeater_Setting()
{    
    unsigned char a = 0;
    bool change = 1;
    unsigned char key;
    
    lcd_clear();
    _delay_ms(50);  // Reduced from 100
    lcd_cmd(0x80);
    lcd_string_f("RM-KEYPAD SETING");
    a = GetRPTREnabled(); 
    
    while(1) {            
        if(change == 1) {
            change = 0;
            lcd_cmd(0xC0);
            lcd_string_f(a == 0 ? "    DISABLED    " : "     ENABLED    ");
        }
        
        key = GetKey();
        
        if(key != KEY_NONE) {
            switch(key) {
                case KEY_UP:
                    a = 1;
                    change = 1;
                    break;
                    
                case KEY_DOWN:
                    a = 0;
                    change = 1;
                    break;
                    
                case KEY_ENTER:
                    Eeprom_WriteByte(ADD_RPTR_EN, a);
                    _delay_ms(50);  // Reduced from 100
                    goto exit_setting;
                    
                case KEY_MENU:
                    goto exit_setting;
            }
            _delay_ms(100);  // Reduced from 200 for key debounce
        }
        _delay_ms(20);  // Reduced from 50 for main loop delay                                                    
    }

exit_setting:
    _delay_ms(50);  // Reduced from 100
    lcd_clear();    
}