#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include "customdef.h"
#include "logger.h"

ret_status_t system_manager_init(void);
void update_system(void);
void system_manager_deinit(void);

#endif // __SYSTEM_MANAGER_H__