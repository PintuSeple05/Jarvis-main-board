
#ifndef _GLOBALS_H
#define _GLOBALS_H

#define PASSWORD_ENGG    1423
#define PASSWORD_RESET   1111
#define PASSWORD_SILENT  2211

#define FIRE_REF_1  100
#define FIRE_REF_2  350
#define FIRE_REF_3  440

#define KEY_NONE        0
#define KEY_MENU        'C'
#define KEY_LEFT        '*'
#define KEY_RIGHT       '#'
#define KEY_UP          '8'
#define KEY_DOWN        '0'
#define KEY_ENTER       'D'
#define KEY_DAY_NIGHT   7
#define KEY_TAMPER      8


#define KEY_1           KEY_LEFT
#define KEY_2           KEY_UP
#define KEY_3           KEY_DOWN
#define KEY_4           KEY_RIGHT

#define CUSTOM_CHAR_BATTERY		0
#define CUSTOM_CHAR_MAINS		1
#define CUSTOM_CHAR_NETWORK     2
#define CUSTOM_CHAR_NETWORK_2   3
#define CUSTOM_CHAR_PSTN		7

#define CMD_ZONE_1_NORMAL    1
#define CMD_ZONE_1_TRIG        2
#define CMD_ZONE_1_OPEN        3
#define CMD_ZONE_1_SHORT    4
#define CMD_ZONE_1_ISOLATE    5

#define CMD_ZONE_2_NORMAL    6
#define CMD_ZONE_2_TRIG        7
#define CMD_ZONE_2_OPEN        8
#define CMD_ZONE_2_SHORT    9
#define CMD_ZONE_2_ISOLATE    10

#define CMD_ZONE_3_NORMAL    11
#define CMD_ZONE_3_TRIG        12
#define CMD_ZONE_3_OPEN        13
#define CMD_ZONE_3_SHORT    14
#define CMD_ZONE_3_ISOLATE    15

#define CMD_ZONE_4_NORMAL    16
#define CMD_ZONE_4_TRIG        17
#define CMD_ZONE_4_OPEN        18
#define CMD_ZONE_4_SHORT    19
#define CMD_ZONE_4_ISOLATE    20

#define CMD_ZONE_5_NORMAL    21
#define CMD_ZONE_5_TRIG        22
#define CMD_ZONE_5_OPEN        23
#define CMD_ZONE_5_SHORT    24
#define CMD_ZONE_5_ISOLATE    25

#define CMD_ZONE_6_NORMAL    26
#define CMD_ZONE_6_TRIG        27
#define CMD_ZONE_6_OPEN        28
#define CMD_ZONE_6_SHORT    29
#define CMD_ZONE_6_ISOLATE    30

#define CMD_ZONE_7_NORMAL    31
#define CMD_ZONE_7_TRIG        32
#define CMD_ZONE_7_OPEN        33
#define CMD_ZONE_7_SHORT    34
#define CMD_ZONE_7_ISOLATE    35

#define CMD_ZONE_8_NORMAL    36
#define CMD_ZONE_8_TRIG        37
#define CMD_ZONE_8_OPEN        38
#define CMD_ZONE_8_SHORT    39
#define CMD_ZONE_8_ISOLATE	40

#define CMD_ZONE_STAT_1		41
#define CMD_ZONE_STAT_2		42
#define CMD_ZONE_STAT_3		43
#define CMD_ZONE_STAT_4		44
#define CMD_ZONE_STAT_5		45
#define CMD_ZONE_STAT_6		46
#define CMD_ZONE_STAT_7		47
#define CMD_ZONE_STAT_8		48

#define CMD_RESET			155

#define CMD_TIME			160
#define CMD_START			161
#define CMD_SYNC            162
#define CMD_SYNC_BACK       163
#define CMD_IP_ADDRESS      164

/*
#define ZONE_FIRE   0
#define ZONE_DAY    1
#define ZONE_NIGHT  2
#define ZONE_ISO    3
*/

//#define SMS_CMD_RESET   '1'
//#define SMS_CMD_ARM     '2'
//#define SMS_CMD_DISARM  '3'
//#define SMS_CMD_SILENT  '4'
//#define SMS_CMD_TRIG    '5'
//#define SMS_CMD_MONT    '6'

#define ADD_TEST_BYTE       0
#define ADD_DAY_HOUR        1
#define ADD_DAY_MINUTE      2
#define ADD_NIGHT_HOUR      3
#define ADD_NIGHT_MINUTE    4
#define ADD_PASSWORD_D0     5
#define ADD_PASSWORD_D1     6
#define ADD_PASSWORD_D2     7
#define ADD_PASSWORD_D3     8
#define ADD_DAY_NIGHT_TYPE  9
#define ADD_ZONE_1_TYPE     10
#define ADD_ZONE_2_TYPE     11
#define ADD_ZONE_3_TYPE     12
#define ADD_ZONE_4_TYPE     13
#define ADD_ZONE_5_TYPE     14
#define ADD_ZONE_6_TYPE     15
#define ADD_ZONE_7_TYPE     16
#define ADD_ZONE_8_TYPE     17
#define ADD_ZONE_9_TYPE     18
#define ADD_ZONE_10_TYPE    19
#define ADD_ZONE_11_TYPE    20
#define ADD_ZONE_12_TYPE    21
#define ADD_ZONE_13_TYPE    22
#define ADD_ZONE_14_TYPE    23
#define ADD_ZONE_15_TYPE    24
#define ADD_ZONE_16_TYPE    25
//#define ADD_GSM_EN          27 // 0 if disabled
//#define ADD_FIRE_CALL       28
//#define ADD_INTR_CALL       29
//#define ADD_FIRE_SMS        30
//#define ADD_INTR_SMS        31
//#define ADD_TAMP_SMS        32
#define ADD_ENTRY_DELAY     33                   // Changed from 54 to 33
#define ADD_NIGHT_TRIG      34
#define ADD_SMS_SETUP       35 // 0 if disabled
#define ADD_PSTN_EN         36 // 0 if disabled
#define ADD_EXIT_DELAY      37
#define ADD_ARM_DISARM_TYPE 38
#define ADD_ARM_HOUR        39
#define ADD_DISARM_HOUR     40
#define ADD_ARM_MINUTE      41
#define ADD_DISARM_MINUTE   42
#define ADD_POWER_SMS       43
#define ADD_DAY_NIGHT_SMS   44


#define ADD_IP_BYTE_1       45
#define ADD_IP_BYTE_2       46
#define ADD_IP_BYTE_3       47
#define ADD_SOUNDER_TIME    48

#define ADD_ZONE_1_DELAY     49
#define ADD_ZONE_2_DELAY     50
#define ADD_ZONE_3_DELAY     51
#define ADD_ZONE_4_DELAY     52
#define ADD_ZONE_5_DELAY     53
#define ADD_ZONE_6_DELAY     54
#define ADD_ZONE_7_DELAY     55
#define ADD_ZONE_8_DELAY     56
#define ADD_ZONE_9_DELAY     57
#define ADD_ZONE_10_DELAY    58
#define ADD_ZONE_11_DELAY    59
#define ADD_ZONE_12_DELAY    60
#define ADD_ZONE_13_DELAY    61
#define ADD_ZONE_14_DELAY    62
#define ADD_ZONE_15_DELAY    63
#define ADD_ZONE_16_DELAY    64


#define ADD_EMAIL_EN         70
#define ADD_REDIAL_NUM       71
#define ADD_FIRE_CALL_SET    72
#define ADD_INTR_CALL_SET    73
#define ADD_TMPR_CALL_SET    74
#define ADD_FIRE_SMS_SET     75
#define ADD_INTR_SMS_SET     76
#define ADD_TMPR_SMS_SET     77
#define ADD_FIRE_CALL_SEQ    78
#define ADD_INTR_CALL_SEQ    79
#define ADD_TMPR_CALL_SEQ    80
#define ADD_FIRE_SMS_SEQ     81
#define ADD_INTR_SMS_SEQ     82
#define ADD_TMPR_SMS_SEQ     83
/*#define ADD_FIRE_MSG_REC     76
#define ADD_FIRE_MSG_PLY     77
#define ADD_INTR_MSG_REC     78
#define ADD_INTR_MSG_PLY     79
#define ADD_TMPR_MSG_REC     80
#define ADD_TMPR_MSG_PLY     81
#define ADD_MSG4_REC         82
#define ADD_MSG4_PLY         83 
*/
/// 16 digits per number
/// 15 fire numbers, 15 intrusion numbers
/// 100->100+480=100->580

#define ADD_INTR_NUM_BASE        100

#define ADD_ADDRESS_BASE    600
/// 32 bytes of address text, 600->631
#define ADD_NAME_BASE       632
/// 32 bytes of name text, 632->663

#define ADD_ARM_DISARM      670

/*=======================PARTITION ARM/DISARM===============*/

#define ADD_ARM_DISARM_P1   671
#define ADD_ARM_DISARM_P2   672
#define ADD_ARM_DISARM_P3   673
#define ADD_ARM_DISARM_P4   674

/*===========================================================*/

/*=========================PASSWORD FOR 10 USERS==============*/

#define ADD_PASSWORD_D0_U1   675
#define ADD_PASSWORD_D1_U1   676
#define ADD_PASSWORD_D2_U1   677
#define ADD_PASSWORD_D3_U1   678

#define ADD_PASSWORD_D0_U2   679
#define ADD_PASSWORD_D1_U2   680
#define ADD_PASSWORD_D2_U2   681
#define ADD_PASSWORD_D3_U2   682

#define ADD_PASSWORD_D0_U3   683
#define ADD_PASSWORD_D1_U3   684
#define ADD_PASSWORD_D2_U3   685
#define ADD_PASSWORD_D3_U3   686

#define ADD_PASSWORD_D0_U4   687
#define ADD_PASSWORD_D1_U4   688
#define ADD_PASSWORD_D2_U4   689
#define ADD_PASSWORD_D3_U4   690

#define ADD_PASSWORD_D0_U5   691
#define ADD_PASSWORD_D1_U5   692
#define ADD_PASSWORD_D2_U5   693
#define ADD_PASSWORD_D3_U5   694

#define ADD_PASSWORD_D0_U6   695
#define ADD_PASSWORD_D1_U6   696
#define ADD_PASSWORD_D2_U6   697
#define ADD_PASSWORD_D3_U6   698

#define ADD_PASSWORD_D0_U7   699
#define ADD_PASSWORD_D1_U7   700
#define ADD_PASSWORD_D2_U7   701
#define ADD_PASSWORD_D3_U7   702

#define ADD_PASSWORD_D0_U8   703
#define ADD_PASSWORD_D1_U8   704
#define ADD_PASSWORD_D2_U8   705
#define ADD_PASSWORD_D3_U8   706

#define ADD_PASSWORD_D0_U9   707
#define ADD_PASSWORD_D1_U9   708
#define ADD_PASSWORD_D2_U9   709
#define ADD_PASSWORD_D3_U9   710

#define ADD_PASSWORD_D0_U10   711
#define ADD_PASSWORD_D1_U10   712
#define ADD_PASSWORD_D2_U10   713
#define ADD_PASSWORD_D3_U10   714

/*============================================================*/

/*==================USER PARTION ADDRESS SETTINGS=============*/

#define ADD_PART1_U1  715
#define ADD_PART2_U1  716
#define ADD_PART3_U1  717
#define ADD_PART4_U1  718

#define ADD_PART1_U2  719
#define ADD_PART2_U2  720
#define ADD_PART3_U2  721
#define ADD_PART4_U2  722

#define ADD_PART1_U3  723
#define ADD_PART2_U3  724
#define ADD_PART3_U3  725
#define ADD_PART4_U3  726

#define ADD_PART1_U4  727
#define ADD_PART2_U4  728
#define ADD_PART3_U4  729
#define ADD_PART4_U4  730

#define ADD_PART1_U5  731
#define ADD_PART2_U5  732
#define ADD_PART3_U5  733
#define ADD_PART4_U5  734

#define ADD_PART1_U6  735
#define ADD_PART2_U6  736
#define ADD_PART3_U6  737
#define ADD_PART4_U6  738

#define ADD_PART1_U7  739
#define ADD_PART2_U7  740
#define ADD_PART3_U7  741
#define ADD_PART4_U7  742

#define ADD_PART1_U8  743
#define ADD_PART2_U8  744
#define ADD_PART3_U8  745
#define ADD_PART4_U8  746

#define ADD_PART1_U9  747
#define ADD_PART2_U9  748
#define ADD_PART3_U9  749
#define ADD_PART4_U9  750

#define ADD_PART1_U10  751
#define ADD_PART2_U10  752
#define ADD_PART3_U10  753
#define ADD_PART4_U10  754

/*=============================================================*/

/*====================IP ADDRESS MEMORY========================*/
//
//#define IP_ADDRESS_FIRST_OCTET   755
//#define IP_ADDRESS_SECOND_OCTET  756
//#define IP_ADDRESS_THIRD_OCTET   757
//#define IP_ADDRESS_FOURTH_OCTET  758

/*==============================================================*/

/*============================ACCOUNT NUMBER====================================*/

//#define    ADD_ACCOUNT_0     759
//#define    ADD_ACCOUNT_1     760
//#define    ADD_ACCOUNT_2     761
//#define    ADD_ACCOUNT_3     762
//#define    ADD_ACCOUNT_4     763
//#define    ADD_ACCOUNT_5     764

/*==============================PORT NUMBER===================================*/

//#define ADD_PORT_NUMBER_0  765
//#define ADD_PORT_NUMBER_1  766
//#define ADD_PORT_NUMBER_2  767
//#define ADD_PORT_NUMBER_3  768
//#define ADD_PORT_NUMBER_4  769

/*================================HOST IP ADDRESS=================================*/

//#define  ADD_IP_BYTE_0    770
//#define  ADD_IP_BYTE_1    771
//#define  ADD_IP_BYTE_2    772
//#define  ADD_IP_BYTE_3    773

/*=================================================================*/

#define HEARTBEAT_TIME_INT  774         //for HeartBeat interval
#define ADD_CMS_EN          775
#define ADD_RPTR_EN         776

#define ADD_FIRE_NUM_BASE  800
#define ADD_EMAIL_ID_BASE   960

#define ADD_LOG_BASE       1000
#define ADD_HOLIDAY_BASE   9000
#define ADD_EMAIL_BASE     9500

/* 1000 logs will be there
		log format
	D0=zone (1->16)
	D1=type (0 for trig,else tamper)
	D2=hour
	D3=minute
	D4=second
	D5=day
	D6=month
	d7=year

*/

#define TYPE_FIRE   0
#define TYPE_DAY    1
#define TYPE_NIGHT  2
#define TYPE_ISO    3


#define MENU_USER_CREATE        0
#define MENU_SET_TIME           1            //needs to be changed when all menu configuration done
#define MENU_SET_DAY_TIME       2
#define MENU_SET_NIGHT_TIME     3
#define MENU_SET_DAY_NIGHT_MODE 4
#define MENU_ARM_DISARM         5
#define MENU_ARM_DISARM_MODE    6
#define MENU_ARM_TIME           7
#define MENU_DISARM_TIME        8
#define MENU_ZONE_SETTINGS      9
#define MENU_ZONE_DELAY         10
#define MENU_SET_ENTRY_TIME     11
#define MENU_SET_EXIT_TIME      12
#define MENU_SOUNDER_TIME       13
#define MENU_NIGHT_CUT_TRIG     14
#define MENU_WALK_TEST          15
#define NAME_                   16
#define ADDRESS_                17
#define MENU_VIEW_LOGS          18
#define MENU_ERASE_LOGS         19
#define MENU_HOLIDAY_LIST       20
#define MENU_HOLIDAY_CLEAR      21
#define MENU_CHANGE_PASSWORD    22
#define MENU_FACTORY_RESET      23
#define MENU_KEYPAD_ON_OFF      24
#define MENU_CMS_SETTING        25

#define MENU_ADD_FIRE_NUM       26
#define MENU_ADD_INTR_NUM       27
#define MENU_FIRE_CALL_SET      28
#define MENU_INTR_CALL_SET      29
#define MENU_TMPR_CALL_SET      30
#define MENU_FIRE_SMS_SET       31
#define MENU_INTR_SMS_SET       32
#define MENU_TMPR_SMS_SET       33
#define MENU_ADD_EMAIL_ID       34
#define MENU_FIRE_MSG_REC       35
#define MENU_FIRE_MSG_PLY       36
#define MENU_INTR_MSG_REC       37
#define MENU_INTR_MSG_PLY       38
#define MENU_TMPR_MSG_REC       39
#define MENU_TMPR_MSG_PLY       40
#define MENU_MSG4_REC           41
#define MENU_MSG4_PLY           42


#endif
                                 