#ifndef _MENU_INCLUDED_
#define _MENU_INCLUDED_
#define F_CPU 8000000UL
#include <util/delay.h>
unsigned int GetPassword();
unsigned int GetPasswordUser(unsigned char user_index);
// 0=wrong, 1=ok,2=cancel      //use = 1 for MENU; use = 2 for RESET; use = 3 for SILENT
unsigned char PasswordDlg(unsigned char fun);
 
/*
unsigned char which_key(unsigned char bus_value,unsigned char key);
unsigned char GetKey();
*/


#endif

