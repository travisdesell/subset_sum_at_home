#include <stdint.h>
#include <iostream>

#include "n_choose_k.hpp"

#include <boost/multiprecision/gmp.hpp>

using boost::multiprecision::mpz_int;

mpz_int **n_choose_k_lookup_table;

void n_choose_k_init(uint32_t max_n, uint32_t max_k) {
    max_n += 5;
    max_k += 5;

    n_choose_k_lookup_table = new mpz_int*[max_n];

    for (uint32_t i = 0; i < max_n; i++) {
        n_choose_k_lookup_table[i] = new mpz_int[max_k];

        for (uint32_t j = 0; j <= max_k; j++) {
            n_choose_k_lookup_table[i][j] = 0;
        }
    }

    for (uint32_t i = 0; i < max_n; i++) {
        n_choose_k_lookup_table[i][0] = 1;
        for (uint32_t j = 1; j <= i && j < max_k; j++) {
            n_choose_k_lookup_table[i][j] = n_choose_k_lookup_table[i-1][j] + n_choose_k_lookup_table[i-1][j-1];
        }
    }
}

mpz_int n_choose_k(uint32_t n, uint32_t k) {
//    std::cout << "lookup of " << n << " choose " << k << " is: " << n_choose_k_lookup_table[n][k] << std::endl;

    return n_choose_k_lookup_table[n][k];
}
