#ifndef SSS_OUTPUT_H
#define SSS_OUTPUT_H

#include "stdint.h"
#include <iostream>
#include <fstream>

using namespace std;

void print_bits(ostream *output_target, const uint32_t number);
void print_bit_array(ostream *output_target, const uint32_t *bit_array, const uint32_t bit_array_length);
void print_subset(ostream *output_target, const uint32_t *subset, const uint32_t subset_size);
void print_bit_array_color(ostream *output_target, const uint32_t *bit_array, unsigned long int max_sums_length, uint32_t min, uint32_t max);

void print_subset_calculation(ostream *output_target, const uint64_t iteration, uint32_t *subset, const uint32_t subset_size, const bool success);

#endif
