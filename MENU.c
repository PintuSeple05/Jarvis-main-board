#include "MENU.h"
#include "globals.h"

extern unsigned char Keypad_GetKey_NonBlocking();
extern unsigned char Eeprom_ReadByte(unsigned int Addr);
//extern void _delay_ms(unsigned int n);
extern void lcd_clear();
extern void lcd_cmd (char cmd);
extern void lcd_string_f(const char *s);
extern void lcd_data (unsigned char data);
extern void delay(unsigned int a);
extern void serial_string(char *s);
extern void serial_putc(unsigned char c);
extern void serial1_string(char *s);
extern void serial1_putc(unsigned char c);

void send_duress_message(void)
{
    serial_string("$DRR"); serial_putc(0x0D); serial_putc(0x0A);
    serial1_string("$DRR"); serial1_putc(0x0D); serial1_putc(0x0A);
}

unsigned int GetPassword()
{    
    unsigned int d0,d1,d2,d3;
    unsigned int address=0;
    d0=Eeprom_ReadByte(ADD_PASSWORD_D0);
    d1=Eeprom_ReadByte(ADD_PASSWORD_D1);
    d2=Eeprom_ReadByte(ADD_PASSWORD_D2);
    d3=Eeprom_ReadByte(ADD_PASSWORD_D3);
    address=d3*1000+d2*100+d1*10+d0;
    return address;
}

unsigned int GetPasswordUser(unsigned char user_index)
{    
    unsigned int d0,d1,d2,d3,PassAddBase;
    unsigned int pass=0;
    PassAddBase = 675+((user_index-1)*4);
    d0=Eeprom_ReadByte(PassAddBase);
    d1=Eeprom_ReadByte(PassAddBase+1);
    d2=Eeprom_ReadByte(PassAddBase+2);
    d3=Eeprom_ReadByte(PassAddBase+3);
    pass=d3*1000+d2*100+d1*10+d0;
    return pass;
}

unsigned char PasswordDlg(unsigned char fun)
{    
    unsigned char a=0;
    unsigned char PasswordDlg_buff[4];
    unsigned int password=0;
    unsigned char result=0;
    unsigned char count=0;
    unsigned char key=0;
    unsigned char hex_value=0;
    unsigned int reveal_timer=0;
    unsigned int mask_delay=0;
    
    lcd_clear(); 
    lcd_cmd(0x80);
    
    if(fun == 1){
        lcd_string_f("    PASSWORD    ");
    }else if(fun == 2){
        lcd_string_f("PASSWORD  RESET");  
    }else if(fun == 3){
        lcd_string_f("PASSWORD SILENT");
    }
    lcd_cmd(0xC0);
    lcd_string_f(">");  
    _delay_ms(300);
    
    while(1){
        key = Keypad_GetKey_NonBlocking();
        
        if(key != 0xFF){
            // Check for Backspace key 'A'
            if(key == 'A'){
                if(a > 0){
                    // Backspace functionality
                    a--;
                    lcd_cmd(0xC0 + a + 1);
                    lcd_data(' ');
                    lcd_cmd(0xC0 + a + 1);
                    mask_delay = 0;
                }
                count = 0;
                while(Keypad_GetKey_NonBlocking() != 0xFF){
                    delay(1);
                }
                _delay_ms(300);
            }
            // Check for Home/Cancel key 'C' - return to home screen
            else if(key == 'C'){
                result = 12;
                break;
            }
            // Check for Enter key 'D' - submit password
            else if(key == 'D'){
                if(a == 4){
                    password = PasswordDlg_buff[0]*1000 + PasswordDlg_buff[1]*100 + PasswordDlg_buff[2]*10 + PasswordDlg_buff[3]; 
                    
                    if(fun == 1){
                        if(password==GetPassword() || password==PASSWORD_ENGG){
                            result=1;
                        }else if(password == GetPasswordUser(1)){
                            result=2; 
                        }else if(password == GetPasswordUser(2)){
                            result=3; 
                        }else if(password == GetPasswordUser(3)){
                            result=4; 
                        }else if(password == GetPasswordUser(4)){
                            result=5; 
                        }else if(password == GetPasswordUser(5)){
                            result=6; 
                        }else if(password == GetPasswordUser(6)){
                            result=7; 
                        }else if(password == GetPasswordUser(7)){
                            result=8; 
                        }else if(password == GetPasswordUser(8)){
                            result=9; 
                        }else if(password == GetPasswordUser(9)){
                            result=10; 
                        }else if(password == GetPasswordUser(10)){
                            result=11; 
                        }else{
                            result=0;
                            send_duress_message();
                        }
                    }
                    if(fun == 2){
                        if(password==PASSWORD_RESET){
                            result=1;
                        }else{
                            result=0;
                        }
                    } 
                    if(fun == 3){
                        if(password==PASSWORD_SILENT){
                            result=1;
                        }else{
                            result=0;
                        }
                    }
                    break;
                }
                count = 0;
                mask_delay = 0;
                while(Keypad_GetKey_NonBlocking() != 0xFF){
                    delay(1);
                }
                _delay_ms(300);
            }
            // Accept only digits 0-9
            else if(key >= '0' && key <= '9'){
                if(a < 4){
                    hex_value = key - '0';
                    PasswordDlg_buff[a] = hex_value;
                    
                    // Display the digit immediately
                    lcd_data(key);
                    
                    a++;
                    count = 0;
                    
                    // Start mask timer (200ms = 20 iterations of 10ms)
                    mask_delay = 1;
                    
                    // Wait for key release
                    while(Keypad_GetKey_NonBlocking() != 0xFF){
                        delay(1);
                    }
                    _delay_ms(300);
                }
            }
            // All other keys (B, *, #) are IGNORED
        }
        
        // Handle mask timer for the last entered digit
        if(mask_delay > 0){
            mask_delay--;
            if(mask_delay == 0){
                // Time to mask - replace last digit with '*'
                lcd_cmd(0xC0 + a);
                lcd_data('*');
            }
        }
        
        if(count == 50){
            result = 12;
            break;         
        }else{
            count++;
        }
        
        _delay_ms(10);
    }
    delay(200);
    lcd_cmd(0x01); 
	return result;
}