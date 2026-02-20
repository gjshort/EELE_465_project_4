#include <msp430fr2153.h>
#include "eUSCI.h"
#include "rtc.h"

void rtc_write_time_reg(MCP7940N_time *rtc_time, uint8_t *rtc_reg_ptr)
{
    switch(*rtc_reg_ptr)
    {
    case 0: // Send register pointer
        UCB0TXBUF = 0x00;
        break;
    case RTC_SEC_REG + 1:
        UCB0TXBUF = rtc_time->seconds | ST_BIT;
        break;
    case RTC_MIN_REG + 1:
        UCB0TXBUF = rtc_time->minutes;
        break;
    case RTC_HR_REG + 1:
        UCB0TXBUF = rtc_time->hours;
        break; 
    case RTC_WKDAY_REG + 1:
        UCB0TXBUF = rtc_time->weekday | VBATEN_BIT;
        break;
    case RTC_DATE_REG + 1:
        UCB0TXBUF = rtc_time->date;
        break;
    case RTC_MTH_REG + 1:
        UCB0TXBUF = rtc_time->month;
        break;
    case RTC_YR_REG + 1:
        UCB0TXBUF = rtc_time->year;
        break;
    default:
        break;
    }
}

void rtc_read_time_reg(MCP7940N_time *rtc_time, uint8_t *rtc_reg_ptr)
{
    switch(*rtc_reg_ptr)
    {
    case RTC_SEC_REG:
        rtc_time->seconds = UCB0RXBUF;
        break;
    case RTC_MIN_REG:
        rtc_time->minutes = UCB0RXBUF;
        break;
    case RTC_HR_REG:
        rtc_time->hours = UCB0RXBUF;
        break; 
    case RTC_WKDAY_REG:
        rtc_time->weekday = UCB0RXBUF;
        break;
    case RTC_DATE_REG:
        rtc_time->date = UCB0RXBUF;
        break;
    case RTC_MTH_REG:
        rtc_time->month = UCB0RXBUF;
        break;
    case RTC_YR_REG:
        rtc_time->year = UCB0RXBUF;
        break;
    default:
        break;
    }
}
