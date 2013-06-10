#ifndef SSS_OUTPUT_H
#define SSS_OUTPUT_H

#include "stdint.h"
#include <iostream>
#include <fstream>
#include <string>

#include <boost/multiprecision/gmp.hpp>

using std::string;
using std::ofstream;
using std::ostream;

using boost::multiprecision::mpz_int;

double mpz_int_to_double(mpz_int value);
uint32_t mpz_int_to_uint32_t(mpz_int value);
string mpz_int_to_binary_string(mpz_int x);

void initialize_print_widths(mpz_int max_iteration, uint32_t max_set_value, uint32_t subset_size);

void print_subset_calculation(ostream *output_target, mpz_int iteration, uint32_t *subset, const uint32_t subset_size, const bool success);

void print_html_header(ostream *output_target, uint32_t max_set_value, uint32_t subset_size);
void print_html_footer(ostream *output_target);
#endif
