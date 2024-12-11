#ifndef CLI_CONTROL_H
#define CLI_CONTROL_H

#include <stdint.h>
#include "app_usbd_cdc_acm.h"

void cli_init(void);
void cli_process(void);

#endif // CLI_CONTROL_H 