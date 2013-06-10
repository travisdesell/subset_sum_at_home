#ifndef SSS_N_CHOOSE_K_HPP
#define SSS_N_CHOOSE_K_HPP

#include <stdint.h>

#include <boost/multiprecision/gmp.hpp>

using boost::multiprecision::mpz_int;

void n_choose_k_init(uint32_t max_n, uint32_t max_k);

/**
 *  This only works up 68 choose 34.  After that we need to use a big number library
 */
mpz_int n_choose_k(uint32_t n, uint32_t k);
#endif
