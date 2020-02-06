#ifndef __RTC_H__
#define __RTC_H__

#include "utils.h"

#define RTC_REG_SEC 0x00
#define RTC_REG_MIN 0x02
#define RTC_REG_HOUR 0x04
#define RTC_REG_WEEK 0x06
#define RTC_REG_DMOT 0x07
#define RTC_REG_MOT 0x08
#define RTC_REG_YEAR 0x09

/**
 * @brief Initialize rtc device
 * 
 * @param out_bit bit value to be filled on subscribe
 * @return int execution return code
 */
int rtc_init(uint8_t* out_bit);

/**
 * @brief Unsubscribe from device
 * 
 */
void rtc_destroy();

/**
 * @brief Try to read value from a register
 * 
 * @param param register to be read
 * @param value pointer to be filled with register value if success
 * @return int 0 if success, error code otherwise
 */
int rtc_get_param(uint8_t param, uint8_t* value);


#endif /* __RTC_H__ */
