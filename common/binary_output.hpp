#ifndef SSS_OUTPUT_H
#define SSS_OUTPUT_H

#include "stdint.h"
#include <iostream>
#include <fstream>

using namespace std;

void print_subset_calculation(ofstream *output_target, const uint64_t iteration, uint32_t *subset, const uint32_t subset_size, const bool success);

#endif
