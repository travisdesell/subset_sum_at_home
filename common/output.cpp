#include <cstdio>
#include <cmath>

#include "output.hpp"
#include "../client/bit_logic.hpp"

//ALL THESE GLOBALS SHOULD PROBABLY BE IN THEIR OWN HEADER FILE
FILE *output_target;


/**
 *  Print the bits in an unsigned int.  Note this prints out from right to left (not left to right)
 */
void print_bits(const unsigned int number) {
    unsigned int pos = 1 << (ELEMENT_SIZE - 1);
    while (pos > 0) {
        if (number & pos) fprintf(output_target, "1");
        else fprintf(output_target, "0");
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
#ifndef HTML_OUTPUT
    fprintf(output_target, "[");
    for (unsigned int i = 0; i < subset_size; i++) {
        fprintf(output_target, "%4u", subset[i]);
    }
    fprintf(output_target, "]");
#else
    fprintf(output_target, "[");
    for (unsigned int i = 0; i < subset_size; i++) {
        double whitespaces = (max_set_digits - floor(log10(subset[i]))) - 1;

        for (int j = 0; j < whitespaces; j++) fprintf(output_target, "&nbsp;");

        fprintf(output_target, "%u", subset[i]);
    }
    fprintf(output_target, "]");
#endif
}

/**
 * Print out an array of bits, coloring the required subsets green, if there is a missing sum (a 0) it is colored red
 */
void print_bit_array_color(const unsigned int *bit_array, unsigned long int max_sums_length, unsigned int min, unsigned int max) {
    unsigned int msl = max_sums_length * ELEMENT_SIZE;
    unsigned int number, pos;
    unsigned int count = 0;

//    fprintf(output_target, " - MSL: %u, MIN: %u, MAX: %u - ", msl, min, max);

    bool red_on = false;

//    fprintf(output_target, " msl - min [%u], msl - max [%u] ", (msl - min), (msl - max));

    for (unsigned int i = 0; i < max_sums_length; i++) {
        number = bit_array[i];
        pos = 1 << (ELEMENT_SIZE - 1);

        while (pos > 0) {
            if ((msl - min) == count) {
                red_on = true;
#ifndef HTML_OUTPUT
                fprintf(output_target, "\e[32m");
#else
                fprintf(output_target, "<b><span class=\"courier_green\">");
#endif
            }

            if (number & pos) fprintf(output_target, "1");
            else {
                if (red_on) {
#ifndef HTML_OUTPUT
                    fprintf(output_target, "\e[31m0\e[32m");
#else
                    fprintf(output_target, "<span class=\"courier_red\">0</span>");
#endif
                } else {
                    fprintf(output_target, "0");
                }
            }

            if ((msl - max) == count) {
#ifndef HTML_OUTPUT
                fprintf(output_target, "\e[0m");
#else
                fprintf(output_target, "</span></b>");
#endif
                red_on = false;
            }

            pos >>= 1;
            count++;
        }
    }
}

void print_subset_calculation(const unsigned long long iteration, unsigned int *subset, const unsigned int subset_size, const bool success) {

    unsigned int M = subset[subset_size - 1];
    unsigned int max_subset_sum = 0;

    for (unsigned int i = 0; i < subset_size; i++) max_subset_sum += subset[i];
    for (unsigned int i = 0; i < max_sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }


    for (unsigned int i = 0; i < max_sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }

    unsigned int current;
#ifdef SHOW_SUM_CALCULATION
    fprintf(output_target, "\n");
#endif
    for (unsigned int i = 0; i < subset_size; i++) {
        current = subset[i];

        shift_left(new_sums, max_sums_length, sums, current);                    // new_sums = sums << current;
//            fprintf(output_target, "new_sums = sums << %2u                                          = ", current);
//            print_bit_array(new_sums, max_sums_length);
//            fprintf(output_target, "\n");

        or_equal(sums, max_sums_length, new_sums);                               //sums |= new_sums;
//            fprintf(output_target, "sums |= new_sums                                               = ");
//            print_bit_array(sums, max_sums_length);
//            fprintf(output_target, "\n");

        or_single(sums, max_sums_length, current - 1);                           //sums |= 1 << (current - 1);
#ifdef SHOW_SUM_CALCULATION
        fprintf(output_target, "sums != 1 << current - 1                                       = ");
        print_bit_array(sums, max_sums_length);
        fprintf(output_target, "\n");
#endif
    }

#ifdef HTML_OUTPUT
    double whitespaces;
    if (iteration == 0) {
        whitespaces = (max_digits - 1);
    } else {
        whitespaces = (max_digits - floor(log10(iteration))) - 1;
    }

    for (int i = 0; i < whitespaces; i++) fprintf(output_target, "&nbsp;");
#endif

#ifndef HTML_OUTPUT
    fprintf(output_target, "%15llu ", iteration);
#else
    fprintf(output_target, "%llu ", iteration);
#endif
    print_subset(subset, subset_size);
    fprintf(output_target, " = ");

    unsigned int min = max_subset_sum - M;
    unsigned int max = M;
#ifdef ENABLE_COLOR
    print_bit_array_color(sums, max_sums_length, min, max);
#else 
    print_bit_array(sums, max_sums_length);
#endif

    fprintf(output_target, "  match %4u to %4u ", min, max);
#ifndef HTML_OUTPUT
#ifdef ENABLE_COLOR
    if (success)    fprintf(output_target, " = \e[32mpass\e[0m\n");
    else            fprintf(output_target, " = \e[31mfail\e[0m\n");
#else
    if (success)    fprintf(output_target, " = pass\n");
    else            fprintf(output_target, " = fail\n");
#endif
#else
    if (success)    fprintf(output_target, " = <span class=\"courier_green\">pass</span><br>\n");
    else            fprintf(output_target, " = <span class=\"courier_red\">fail</span><br>\n");
#endif

    fflush(output_target);
}


