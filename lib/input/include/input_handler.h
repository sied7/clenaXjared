#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include <stdio.h>
#include "customdef.h"
#include "config.h"

 void input_handler_init(void);
 ret_status_t input_get_current_state(uint8_t *state);
 void refresh_input_state(void);


#endif /* __INPUT_HANDLER_H__ */