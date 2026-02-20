#ifndef RTC_H
#define RTC_H

// I2C address
#define MCP7940N_I2C_ADDR       0x6F

// RTC Time Registers
#define RTC_NUM_TIME_REGS       0x07               // Number of timekeeping registers          
#define RTC_SEC_REG             0x00               // seconds (also includes ST bit)
#define RTC_MIN_REG             0x01               // minutes
#define RTC_HR_REG              0x02               // hours
#define RTC_WKDAY_REG           0x03               // weekday (also inlcudes VBAT bit)
#define RTC_DATE_REG            0x04               // day of the month
#define RTC_MTH_REG             0x05               // month
#define RTC_YR_REG              0x06               // year
#define CONTROL_REG             0x07               // control / mode


// ST and VBATEN configuration bits
#define ST_BIT                  0x80               // start oscillator bit (held in SEC register)
#define VBATEN_BIT              0x08               // enable backup battery (held in WKDAY register)

/******************************
* Struct to manage register data
* from the MCP7940N RTC
* !! All registers are in BCD !!
*******************************/
typedef struct MCP7940N_time
{
    uint8_t seconds;        // 0-0x59
    uint8_t minutes;        // 0-0x59
    uint8_t hours;          // 0x01 - 0x12 | 0x00 - 0x24
    uint8_t weekday;        // 0x01 - 0x07
    uint8_t date;           // 0x01 - 0x31
    uint8_t month;          // 0x01 - 0x12
    uint8_t year;           // 0x00 - 0x99
} MCP7940N_time;

void rtc_write_time_reg(MCP7940N_time*, uint8_t*);

#endif
