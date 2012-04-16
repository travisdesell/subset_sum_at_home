#ifndef SSS_N_CHOOSE_K_HPP
#define SSS_N_CHOOSE_K_HPP

/**
 *  This only works up 68 choose 34.  After that we need to use a big number library
 */
static inline unsigned long long n_choose_k(unsigned int n, unsigned int k) {
    /**
     *  Create pascal's triangle and use it for look up
     *  implement for ints of arbitrary length (need to implement binary add and less than)
     */
    unsigned int numerator = n - (k - 1);
    unsigned int denominator = 1;

    unsigned long long combinations = 1;

    while (numerator <= n) {
        combinations *= numerator;
        combinations /= denominator;

        numerator++;
        denominator++;
    }

    return combinations;
}

#endif
