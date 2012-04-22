#ifndef SSS_BIT_LOGIC_HPP
#define SSS_BIT_LOGIC_HPP

#include <climits>

const unsigned int ELEMENT_SIZE = sizeof(unsigned int) * 8;                      

#ifdef HTML_OUTPUT                                                               
extern double max_digits;                                                        
extern double max_set_digits;                                                    
#endif                                                                           

extern unsigned long int max_sums_length;                                        
extern unsigned int *sums;                                                       
extern unsigned int *new_sums;                                                   


//void shift_left(unsigned int *dest, const unsigned int length, const unsigned int *src, const unsigned int shift);
//void or_equal(unsigned int *dest, const unsigned int length, const unsigned int *src);
//void or_single(unsigned int *dest, const unsigned int length, const unsigned int number);


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

//    printf("shift: %u, full_element_shifts: %u, sub_shift: %u\n", shift, full_element_shifts, sub_shift);

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
    unsigned int i;
    for (i = 0; i < length; i++) dest[i] = 0;

    if ((ELEMENT_SIZE - sub_shift) == 32) {
        for (i = 0; i < (length - full_element_shifts) - 1; i++) {
            dest[i] = src[i + full_element_shifts] << sub_shift;
        }
    } else {
        for (i = 0; i < (length - full_element_shifts) - 1; i++) {
            dest[i] = src[i + full_element_shifts] << sub_shift | src[i + full_element_shifts + 1] >> (ELEMENT_SIZE - sub_shift);
        }
    }

    dest[i] = src[length - 1] << sub_shift;
    i++;

    for (; i < length; i++) {
        dest[i] = 0;
    }
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

    /*
     * This will print out all the 1s that we're looking for.

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
            return false;
        }
//        fprintf(output_target, "min success\n");

        for (unsigned int i = (length - min_pos - 2); i > (length - max_pos); i--) {
            if (UINT_MAX != (UINT_MAX & subset[i])) {
                return false;
            }
        }
//        fprintf(output_target, "mid success\n");

        if (max_tmp > 0) {
            against = UINT_MAX >> (ELEMENT_SIZE - max_tmp);
            if (against != (against & subset[length - max_pos - 1])) {
                return false;
            }
        }
//        fprintf(output_target, "max success\n");

    }

    return true;
}





#endif
