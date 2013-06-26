#include <stdint.h>
#include <iostream>

/**
 *  Includes required for BOINC
 */
#ifdef _BOINC_
#ifdef _WIN32
    #include "boinc_win.h"
    #include "str_util.h"
#endif

    #include "diagnostics.h"
    #include "util.h"
    #include "filesys.h"
    #include "boinc_api.h"
    #include "mfile.h"
#endif


#include "n_choose_k.hpp"

#include <boost/multiprecision/gmp.hpp>

using boost::multiprecision::mpz_int;
using std::cout;
using std::endl;

mpz_int **n_choose_k_lookup_table;

uint32_t max_n, max_k;

void n_choose_k_init(uint32_t _max_n, uint32_t _max_k) {
    max_n = _max_n;
    max_k = _max_k;

    max_n += 5;
    max_k += 5;

    n_choose_k_lookup_table = new mpz_int*[max_n];

    for (uint32_t i = 0; i < max_n; i++) {
        n_choose_k_lookup_table[i] = new mpz_int[max_k];

        for (uint32_t j = 0; j < max_k; j++) {
            n_choose_k_lookup_table[i][j] = mpz_int(0);
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

    if (n >= max_n) {
        std::cerr << "lookup of (" << n << ") in n_choose_k failed, because n >= max_n (" << max_n << ") of the lookup table.\n" << std::endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    if (k >= max_k) {
        std::cerr << "lookup of k (" << k << ") in n_choose_k failed, because k >= max_k (" << max_k << ") of the lookup table.\n" << std::endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }
            
    return n_choose_k_lookup_table[n][k];
}
