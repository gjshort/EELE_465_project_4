#ifndef RTC_H
#define RTC_H

// I2C address
#define MCP7940N_I2C_ADDR       0x6F

// RTC Time Registers
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

#endif
