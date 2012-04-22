#ifndef SSS_OUTPUT_H
#define SSS_OUTPUT_H

#include <cstdio>

extern FILE *output_target;

void print_bits(const unsigned int number);
void print_bit_array(const unsigned int *bit_array, const unsigned int bit_array_length);
void print_subset(const unsigned int *subset, const unsigned int subset_size);
void print_bit_array_color(const unsigned int *bit_array, unsigned long int max_sums_length, unsigned int min, unsigned int max);

void print_subset_calculation(const unsigned long long iteration, unsigned int *subset, const unsigned int subset_size, const bool success);

#endif
