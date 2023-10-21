#ifndef _DAP_CALCULATION_H_
#define _DAP_CALCULATION_H_

#include <stdint.h>
#include <stdbool.h>

bool init_DAP_db();
void close_DAP_db();
float calculate_DAP_value(uint16_t kV, uint32_t uA, uint16_t ms);
#endif
