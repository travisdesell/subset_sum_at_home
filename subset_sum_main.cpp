#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

const int ELEMENT_SIZE = sizeof(unsigned int) * 8;

unsigned long int max_sums_length;

/**
 *  Print the bits in an unsigned int.  Note this prints out from right to left (not left to right)
 */
void print_bits(const unsigned int number) {
    unsigned int pos = 1 << (ELEMENT_SIZE) - 1;
    while (pos > 0) {
        if (number & pos) printf("1");
        else printf("0");
        pos >>= 1;
    }
}

/**
 * Print out an array of bits
 */
void print_bit_array(const unsigned int *bit_array, const unsigned int bit_array_length) {
    for (unsigned int i = 0; i < bit_array_length; i++) {
        print_bits(bit_array[i]);
    }
}

/**
 *  Print out all the elements in a subset
 */
void print_subset(const unsigned int *subset, const unsigned int subset_size) {
    printf("[");
    for (unsigned int i = 0; i < subset_size; i++) {
        printf("%4d", subset[i]);
    }
    printf("]");
}

/**
 *  Shift all the bits in an array of to the left by shift. src is unchanged, and the result of the shift is put into dest.
 *  length is the number of elements in dest (and src) which should be the same for both
 *
 *  Performs:
 *      dest = src << shift
 */
static inline void shift_left(unsigned int *dest, const unsigned int length, const unsigned int *src, const unsigned int shift) {
    unsigned int full_element_shifts = shift / ELEMENT_SIZE;
    unsigned int sub_shift = shift % ELEMENT_SIZE;

    /**
     *  Note that the shift may be more than the length of an unsigned int (ie over 32), this needs to be accounted for, so the element
     *  we're shifting from may be ahead a few elements in the array.  When we do the shift, we can do this quickly by getting the target bits
     *  shifted to the left and doing an or with a shift to the right.
     *  ie (if our elements had 8 bits):
     *      00011010 101111101
     *  doing a shift of 5, we could update the first one to:
     *     00010111         // src[i + (full_element_shifts = 0) + 1] >> ((ELEMENT_SIZE = 8) - (sub_shift = 5)) // shift right 3
     *     |
     *     01000000
     *  which would be:
     *     01010111
     *  then the next would just be the second element shifted to the left by 5:
     *     10100000
     *   which results in:
     *     01010111 10100000
     *   which is the whole array shifted to the left by 5
     */
    for (unsigned int i = 0; i < (length - full_element_shifts) - 1; i++) {
        dest[i] = src[i + full_element_shifts] << sub_shift | src[i + full_element_shifts + 1] >> (ELEMENT_SIZE - sub_shift);
    }
    dest[length - 1] = src[length - 1] << sub_shift;
}

/**
 *  updates dest to:
 *      dest != src
 *
 *  Where dest and src are two arrays with length elements
 */
static inline void or_equal(unsigned int *dest, const unsigned int length, const unsigned int *src) {
    for (unsigned int i = 0; i < length; i++) dest[i] |= src[i];
}

/**
 *  Adds the single bit (for a new set element) into dest:
 *
 *  dest |= 1 << number
 */
static inline void or_single(unsigned int *dest, const unsigned int length, const unsigned int number) {
    unsigned int pos = number / ELEMENT_SIZE;
    unsigned int tmp = number % ELEMENT_SIZE;

    dest[length - pos - 1] |= 1 << tmp;
}

/**
 *  Tests to see if all the bits are 1s between min and max
 */
static inline bool all_ones(const unsigned int *subset, const unsigned int length, const unsigned int min, const unsigned int max) {
    unsigned int min_pos = min / ELEMENT_SIZE;
    unsigned int min_tmp = min % ELEMENT_SIZE;
    unsigned int max_pos = max / ELEMENT_SIZE;
    unsigned int max_tmp = max % ELEMENT_SIZE;

//    printf("\n");
#ifdef VERBSOSE
    printf("  match %4d to %4d           ", min, max);
#endif
    /*
    if (min_pos < max_pos) {
        if (max_tmp > 0) {
            print_bits(UINT_MAX >> (ELEMENT_SIZE - max_tmp));
        } else {
            print_bits(0);
        }
        for (unsigned int i = min_pos + 1; i < max_pos - 1; i++) {
            print_bits(UINT_MAX);
        }
        print_bits(UINT_MAX << (min_tmp - 1));
    } else {
        print_bits((UINT_MAX << (min_tmp - 1)) & (UINT_MAX >> (ELEMENT_SIZE - max_tmp)));
    }
    */

    if (min_pos == max_pos) {
        unsigned int against = (UINT_MAX >> (ELEMENT_SIZE - max_tmp)) & (UINT_MAX << (min_tmp - 1));
        return against == (against & subset[length - max_pos - 1]);
    } else {
        unsigned int against = UINT_MAX << (min_tmp - 1);
        if (against != (against & subset[length - min_pos - 1])) {
#ifdef VERBSOSE
            printf(" MIN ");
#endif
            return false;
        }
//        printf("min success\n");

        for (unsigned int i = (length - min_pos - 2); i > (length - max_pos); i--) {
            if (UINT_MAX != (UINT_MAX & subset[i])) {
#ifdef VERBOSE
                printf(" MID ");
#endif
                return false;
            }
        }
//        printf("mid success\n");

        if (max_tmp > 0) {
            against = UINT_MAX >> (ELEMENT_SIZE - max_tmp);
            if (against != (against & subset[length - max_pos - 1])) {
#ifdef VERBOSE
                printf(" MAX ");
#endif
                return false;
            }
        }

//        printf("max success\n");

    }

    return true;
}

/**
 *  Tests to see if a subset all passes the subset sum hypothesis
 */
static inline bool test_subset(const unsigned int *subset, const unsigned int subset_size) {
    unsigned int M = 0; // M is the max value in the subset
    unsigned int max_subset_sum = 0; //the sum of all values in the subset

    unsigned long int sums_length = 0;
    for (unsigned int i = 0; i < subset_size; i++) {
        sums_length += subset[i];
    }
//    sums_length /= 2;
    max_subset_sum = sums_length;
    M = subset[subset_size - 1];

//    printf("subset requires %ld bits.\n", sums_length);
    sums_length /= ELEMENT_SIZE;
    sums_length++;
//    printf("which is %ld unsigned ints.\n", sums_length);

    //this is also symmetric.  Only need to check from the largest element in the set (9) to the sum(S)/2 == (13), need to see if everything between 9 and 13 is a 1
    unsigned int *sums = new unsigned int[sums_length];
    unsigned int *new_sums = new unsigned int[sums_length];
    
    for (unsigned int i = 0; i < sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }

//    printf("\n");
    unsigned int current;
    for (unsigned int i = 0; i < subset_size; i++) {
        current = subset[i];

        shift_left(new_sums, sums_length, sums, current);                    // new_sums = sums << current;
//        printf("new_sums = sums << %2d    = ", current);
//        print_bit_array(new_sums, sums_length);
//        printf("\n");

        or_equal(sums, sums_length, new_sums);                               //sums |= new_sums;
//        printf("sums |= new_sums         = ");
//        print_bit_array(sums, sums_length);
//        printf("\n");

        or_single(sums, sums_length, current - 1);                           //sums |= 1 << (current - 1);
//        printf("sums != 1 << current - 1 = ");
//        print_bit_array(sums, sums_length);
//        printf("\n");
    }

#ifdef VERBOSE
    if (sums_length < max_sums_length) {
        for (unsigned int i = 0; i < (max_sums_length - sums_length); i++) print_bits(0);
    }
    print_bit_array(sums, sums_length);
#endif

    bool success = all_ones(sums, sums_length, M, max_subset_sum - M);

    delete [] sums;
    delete [] new_sums;

    return success;
}

/**
 *  calculates n!
 */
long double fac(unsigned int n) {
    long double result = 1;
    for (unsigned int i = 1; i <= n; i++) {
        result *= i;
    }
//    printf("result: %Lf\n", result);
    return result;
}


int main(int argc, char** argv) {
    unsigned int max_set_value = atoi(argv[1]);
    unsigned int subset_size = atoi(argv[2]);

    /**
     *  Get the maximum set length (in bits) so we can use this for printing out the values cleanly.
     */
    unsigned int *max_set = new unsigned int[subset_size];
    for (unsigned int i = 0; i < subset_size; i++) max_set[subset_size - i - 1] = max_set_value - i;
//    printf("max set: ");
//    print_subset(max_set, subset_size);
//    printf("\n");

    for (unsigned int i = 0; i < subset_size; i++) {
        max_sums_length += max_set[i];
    }
//    sums_length /= 2;
    max_sums_length /= ELEMENT_SIZE;
    max_sums_length++;

    delete [] max_set;

    unsigned int *subset = new unsigned int[subset_size];
    for (unsigned int i = 0; i < subset_size; i++) subset[i] = i + 1;

    bool success;
    unsigned int current;
    unsigned long long pass = 0;
    unsigned long long fail = 0;
    while (subset[0] <= (max_set_value - subset_size + 1)) {
#ifdef VERBOSE
        print_subset(subset, subset_size);
        printf(" = ");
#endif

        success = test_subset(subset, subset_size);
#ifdef VERBOSE
        if ( success ) {
            printf(" = pass\n");
        } else {
            printf(" = fail\n");
        }
#endif

        if (success) {
            pass++;
        } else {
            fail++;
        }

        current = subset_size - 1;
        subset[current]++;
        while (current > 0 && subset[current] > (max_set_value - (subset_size - (current + 1)))) {
            subset[current - 1]++;
            current --;
        }
        while (current < subset_size - 1) {
            subset[current + 1] = subset[current] + 1;
            current++;
        }

    }

    /**
     * pass + fail should = M! / (N! * (M - N)!)
     */
    long double expected_total = fac(max_set_value) / (fac(subset_size) * fac(max_set_value - subset_size));

    printf("the expected total number of sets is: %Lf\n", expected_total);
    printf("%lld total sets, %lld sets passed, %lld sets failed, %lf success rate.\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));

    delete [] subset;
}
