#include <msp430fr2153.h>
#include "eUSCI.h"
#include "rtc.h"

/**
 * Writes the time to the RTC one register at a time.
 * @param rtc_time - pointer to a created RTC struct
 * @param rtc_reg_idx - the RTC register being written to
 *
 * The value of rtc_reg_ptr dictates which RTC register is being
 * written to. When rtc_reg_ptr is 0, the RTC's internal register
 * pointer is set to the "Seconds" register. Therefore, to perform
 * a multi-byte I2C write, the user should start rtc_reg_ptr
 * at 0 (externally) and then increment it before each subsequent
 * call to this function.
 *
 * This function assumes rtc_time has been updated.
 */
void rtc_write_time_reg(MCP7940N_time *rtc_time, uint8_t *rtc_reg_idx)
{
    switch(*rtc_reg_idx)
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

/**
 * Reads the time from the RTC one register at a time.
 * @param rtc_time - pointer to a created RTC struct
 * @param rtc_reg_idx - the RTC register being read from
 *
 * The value of rtc_reg_ptr dictates which RTC register is being
 * read from. To perform a multi-byte I2C read, the user should 
 * start rtc_reg_ptr at 0 (externally) and then increment it 
 * before each subsequent call to this function.
 *
 * This function assumes UCB0RXBUF has been updated.
 */
void rtc_read_time_reg(MCP7940N_time *rtc_time, uint8_t *rtc_reg_idx)
{
    switch(*rtc_reg_idx)
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
