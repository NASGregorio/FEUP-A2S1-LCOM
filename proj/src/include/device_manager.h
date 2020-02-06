#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#include "vector2d.h"

/**
 * @brief Initialize all devices and respective managers
 *  and enter starting menu
 * 
 * @return int execution return code
 */
int dm_init();

/**
 * @brief Start device interrupt loop
 * 
 */
void dm_start();

/**
 * @brief Free memory and unsubscribe all devices.
 * 
 */
void dm_destroy();

#endif
