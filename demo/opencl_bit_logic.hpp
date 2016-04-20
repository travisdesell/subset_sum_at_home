#include "stdint.h"

#include <limits>
#include <climits>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>

//required for va_start, etc.
#include <stdarg.h>

using std::numeric_limits;

//common data for opencl

extern uint32_t *block_size;


void build_cl_program(const uint32_t subset_length);

void release_cl_program();

void cl_shift_left(uint32_t *block, uint32_t subset_size, int *complete);
