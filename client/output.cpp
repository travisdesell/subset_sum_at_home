
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


