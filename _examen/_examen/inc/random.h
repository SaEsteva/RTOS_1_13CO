#ifndef RANDOM_H
#define RANDOM_H

#include "stdint.h"
#include "sapi.h"

#define USE_FREERTOS

void random_set_seed( uint32_t semilla );
uint32_t random( int32_t min, int32_t max );
void random_seed_freertos();

#endif
