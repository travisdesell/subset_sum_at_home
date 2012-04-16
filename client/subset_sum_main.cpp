#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cmath>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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

/**
 *  Includes for subset sum
 */

#include "output.hpp"

using namespace std;

const unsigned int ELEMENT_SIZE = sizeof(unsigned int) * 8;

unsigned long int max_sums_length;
unsigned int *sums;
unsigned int *new_sums;

string checkpoint_file = "sss_checkpoint.txt";
string output_filename = "failed_sets.txt";
FILE *output_target;

vector<unsigned long long> failed_sets = new vector<unsigned long long>();

#ifdef HTML_OUTPUT
double max_digits;
double max_set_digits;
#endif

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

/**
 *  Tests to see if a subset all passes the subset sum hypothesis
 */
static inline bool test_subset(const unsigned int *subset, const unsigned int subset_size, const unsigned long long iteration, const unsigned int starting_subset, const bool doing_slice) {
    //this is also symmetric.  TODO: Only need to check from the largest element in the set (9) to the sum(S)/2 == (13), need to see if everything between 9 and 13 is a 1
    unsigned int M = subset[subset_size - 1];
    unsigned int max_subset_sum = 0;

    for (unsigned int i = 0; i < subset_size; i++) max_subset_sum += subset[i];
    
    for (unsigned int i = 0; i < max_sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }

//    fprintf(output_target, "\n");
    unsigned int current;
    for (unsigned int i = 0; i < subset_size; i++) {
        current = subset[i];

        shift_left(new_sums, max_sums_length, sums, current);                    // new_sums = sums << current;
//        fprintf(output_target, "new_sums = sums << %2u    = ", current);
//        print_bit_array(new_sums, sums_length);
//        fprintf(output_target, "\n");

        or_equal(sums, max_sums_length, new_sums);                               //sums |= new_sums;
//        fprintf(output_target, "sums |= new_sums         = ");
//        print_bit_array(sums, sums_length);
//        fprintf(output_target, "\n");

        or_single(sums, max_sums_length, current - 1);                           //sums |= 1 << (current - 1);
//        fprintf(output_target, "sums != 1 << current - 1 = ");
//        print_bit_array(sums, sums_length);
//        fprintf(output_target, "\n");
    }

    bool success = all_ones(sums, max_sums_length, M, max_subset_sum - M);

#ifdef VERBOSE
#ifdef FALSE_ONLY
    if (!success) {
#endif

#ifdef SHOW_SUM_CALCULATION
        for (unsigned int i = 0; i < max_sums_length; i++) {
            sums[i] = 0;
            new_sums[i] = 0;
        }

        fprintf(output_target, "\n");
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
            fprintf(output_target, "sums != 1 << current - 1                                       = ");
            print_bit_array(sums, max_sums_length);
            fprintf(output_target, "\n");
        }
#endif

#ifdef HTML_OUTPUT
        double whitespaces;
        if (iteration == 0) whitespaces = (max_digits - 1);
        else {
            if (doing_slice)    whitespaces = (max_digits - floor(log10(iteration + starting_subset))) - 1;
            else                whitespaces = (max_digits - floor(log10(iteration))) - 1;
        }

        for (int i = 0; i < whitespaces; i++) fprintf(output_target, "&nbsp;");
#endif

#ifndef HTML_OUTPUT
        if (doing_slice)    fprintf(output_target, "%15llu ", (iteration + starting_subset));
        else                fprintf(output_target, "%15llu ", iteration);
#else
        if (doing_slice)    fprintf(output_target, "%llu ", (iteration + starting_subset));
        else                fprintf(output_target, "%llu ", iteration);
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

#ifdef FALSE_ONLY
    }
#endif
#endif

    return success;
}

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

static inline void generate_ith_subset(unsigned long long i, unsigned int *subset, unsigned int subset_size, unsigned int max_set_value) {
    unsigned int pos = 0;
    unsigned int current_value = 1;
    unsigned long long nck;

    while (pos < subset_size - 1) {
        //TODO: this does not need to be recalcualted, there is a faster way to do this
        //Would be the fastest way to do it if we were using a table of n choose k values -- which we should for a big int representation
        nck = n_choose_k((max_set_value - 1) - current_value, (subset_size - 1) - (pos + 1));

        if (i < nck) {
            subset[pos] = current_value;
            pos++;
        } else {
            i -= nck;
        }
        current_value++;
    }

    subset[subset_size - 1] = max_set_value;
}

static inline void generate_next_subset(unsigned int *subset, unsigned int subset_size, unsigned int max_set_value) {
    unsigned int current = subset_size - 2;
    subset[current]++;

//    fprintf(output_target, "subset_size: %u, max_set_value: %u\n", subset_size, max_set_value);

//    print_subset(subset, subset_size);
//    fprintf(output_target, "\n");

    while (current > 0 && subset[current] > (max_set_value - (subset_size - (current + 1)))) {
        subset[current - 1]++;
        current--;

//        print_subset(subset, subset_size);
//        fprintf(output_target, "\n");
    }

    while (current < subset_size - 2) {
        subset[current + 1] = subset[current] + 1;
        current++;

//        print_subset(subset, subset_size);
//        fprintf(output_target, "\n");
    }

    subset[subset_size - 1] = max_set_value;

//    print_subset(subset, subset_size);
//    fprintf(output_target, "\n");
}

void write_checkpoint(string filename, const unsigned long long iteration, const unsigned long long pass, const unsigned long long fail) {
#ifdef _BOINC_
    string output_path;
    int retval = boinc_resolve_filename_s(filename.c_str(), output_path);
    if (retval) {
        fprintf(stderr, "APP: error writing checkpoint (resolving checkpoint file name)\n");
        return;
    }   

    ofstream checkpoint_file(output_path.c_str());
#else
    ofstream checkpoint_file(filename.c_str());
#endif
    if (!checkpoint_file.is_open()) {
        fprintf(stderr, "APP: error writing checkpoint (opening checkpoint file)\n");
        return;
    }   

    checkpoint_file << "iteration: " << iteration << endl;
    checkpoint_file << "pass: " << pass << endl;
    checkpoint_file << "fail: " << fail << endl;

    checkpoint_file.close();
}

bool read_checkpoint(string sites_filename, unsigned long long &iteration, unsigned long long &pass, unsigned long long &fail) {
#ifdef _BOINC_
    string input_path;
    int retval = boinc_resolve_filename_s(sites_filename.c_str(), input_path);
    if (retval) {
        return 0;
    }

    ifstream sites_file(input_path.c_str());
#else
    ifstream sites_file(sites_filename.c_str());
#endif
    if (!sites_file.is_open()) return false;

    string s;
    sites_file >> s >> iteration;
    if (s.compare("iteration:") != 0) {
        fprintf(stderr, "ERROR: malformed checkpoint! could not read 'iteration'\n");
        exit(0);
    }

    sites_file >> s >> pass;
    if (s.compare("pass:") != 0) {
        fprintf(stderr, "ERROR: malformed checkpoint! could not read 'pass'\n");
        exit(0);
    }

    sites_file >> s >> fail;
    if (s.compare("fail:") != 0) {
        fprintf(stderr, "ERROR: malformed checkpoint! could not read 'fail'\n");
        exit(0);
    }

    return true;
}


int main(int argc, char** argv) {
#ifdef _BOINC_
    int retval = 0;
#ifdef BOINC_APP_GRAPHICS
#if defined(_WIN32) || defined(__APPLE)
    retval = boinc_init_graphics(worker);
#else
    retval = boinc_init_graphics(worker, argv[0]);
#endif
#else
    retval = boinc_init();
#endif
    if (retval) exit(retval);
#endif

    if (argc != 3 && argc != 5) {
        fprintf(stderr, "ERROR, wrong command line arguments.\n");
        fprintf(stderr, "USAGE:\n");
        fprintf(stderr, "\t./subset_sum <M> <N> [<i> <count>]\n\n");
        fprintf(stderr, "argumetns:\n");
        fprintf(stderr, "\t<M>      :   The maximum value allowed in the sets.\n");
        fprintf(stderr, "\t<N>      :   The number of elements allowed in a set.\n");
        fprintf(stderr, "\t<i>      :   (optional) start at the <i>th generated subset.\n");
        fprintf(stderr, "\t<count>  :   (optional) only test <count> subsets (starting at the <i>th subset).\n");
        exit(0);
    }

    unsigned long max_set_value = atol(argv[1]);
#ifdef HTML_OUTPUT
    max_set_digits = ceil(log10(max_set_value)) + 1;
#endif

    unsigned long subset_size = atol(argv[2]);

    unsigned long long iteration = 0;
    unsigned long long pass = 0;
    unsigned long long fail = 0;

#ifdef ENABLE_CHECKPOINTING
    bool started_from_checkpoint = read_checkpoint(checkpoint_file, iteration, pass, fail, failed_subsets);
#else
    bool started_from_checkpoint = false;
#endif

#ifdef _BOINC_
    string output_path;
    retval = boinc_resolve_filename_s(output_filename.c_str(), output_path);
    if (retval) {
        fprintf(stderr, "APP: error opening output file for failed sets.\n");
        exit(0);
    }   

    if (started_from_checkpoint) {
        output_target = fopen(output_path.c_str(), "a");
    } else {
        output_target = fopen(output_path.c_str(), "w");
    }
#else 
    output_target = stdout;
#endif

#ifdef HTML_OUTPUT
    fprintf(output_target, "<!DOCTYPE html PUBLIC \"-//w3c//dtd html 4.0 transitional//en\">\n");
    fprintf(output_target, "<html>\n");
    fprintf(output_target, "<head>\n");
    fprintf(output_target, "  <meta http-equiv=\"Content-Type\"\n");
    fprintf(output_target, " content=\"text/html; charset=iso-8859-1\">\n");
    fprintf(output_target, "  <meta name=\"GENERATOR\"\n");
    fprintf(output_target, " content=\"Mozilla/4.76 [en] (X11; U; Linux 2.4.2-2 i686) [Netscape]\">\n");
    fprintf(output_target, "  <title>%lu choose %lu</title>\n", max_set_value, subset_size);
    fprintf(output_target, "\n");
    fprintf(output_target, "<style type=\"text/css\">\n");
    fprintf(output_target, "    .courier_green {\n");
    fprintf(output_target, "        color: #008000;\n");
    fprintf(output_target, "    }   \n");
    fprintf(output_target, "</style>\n");
    fprintf(output_target, "<style type=\"text/css\">\n");
    fprintf(output_target, "    .courier_red {\n");
    fprintf(output_target, "        color: #FF0000;\n");
    fprintf(output_target, "    }   \n");
    fprintf(output_target, "</style>\n");
    fprintf(output_target, "\n");
    fprintf(output_target, "</head><body>\n");
    fprintf(output_target, "<h1>%lu choose %lu</h1>\n", max_set_value, subset_size);
    fprintf(output_target, "<hr width=\"100%%\">\n");
    fprintf(output_target, "\n");
    fprintf(output_target, "<br>\n");
    fprintf(output_target, "<tt>\n");
#endif

    if (!started_from_checkpoint) {
#ifndef HTML_OUTPUT
        fprintf(output_target, "max_set_value: %lu, subset_size: %lu\n", max_set_value, subset_size);
#else
        fprintf(output_target, "max_set_value: %lu, subset_size: %lu<br>\n", max_set_value, subset_size);
#endif
        if (max_set_value < subset_size) {
            fprintf(stderr, "Error max_set_value < subset_size. Quitting.\n");
            exit(0);
        }
    } else {
        fprintf(stderr, "Starting from checkpoint on iteration %llu, with %llu pass, %llu fail.\n", iteration, pass, fail);
    }

    //timestamp flag
#ifdef TIMESTAMP
    time_t start_time;
    time( &start_time );
    if (!started_from_checkpoint) {
        fprintf(output_target, "start time: %s", ctime(&start_time) );
    }
#endif

    bool doing_slice = false;
    unsigned int starting_subset = 0;
    unsigned int subsets_to_calculate = 0;

    if (argc == 5) {
        doing_slice = true;
        starting_subset = atoi(argv[3]);
        subsets_to_calculate = atoi(argv[4]);
    }

    /**
     *  Calculate the maximum set length (in bits) so we can use this for printing out the values cleanly.
     */
    unsigned int *max_set = new unsigned int[subset_size];
    for (unsigned int i = 0; i < subset_size; i++) max_set[subset_size - i - 1] = max_set_value - i;
    for (unsigned int i = 0; i < subset_size; i++) max_sums_length += max_set[i];

//    sums_length /= 2;
    max_sums_length /= ELEMENT_SIZE;
    max_sums_length++;

    delete [] max_set;

    unsigned int *subset = new unsigned int[subset_size];

//    this caused a problem:
//    
//    fprintf(output_target, "%15u ", 296010);
//    generate_ith_subset(296010, subset, subset_size, max_set_value);
//    print_subset(subset, subset_size);
//    fprintf(output_target, "\n");

    unsigned long long expected_total = n_choose_k(max_set_value - 1, subset_size - 1);

#ifdef HTML_OUTPUT
    max_digits = ceil(log10(expected_total));
#endif

//    for (unsigned long long i = 0; i < expected_total; i++) {
//        fprintf(output_target, "%15llu ", i);
//        generate_ith_subset(i, subset, subset_size, max_set_value);
//        print_subset(subset, subset_size);
//        fprintf(output_target, "\n");
//    }


#ifndef HTML_OUTPUT
    if (!started_from_checkpoint) {
        if (doing_slice) {
            fprintf(output_target, "performing %u set evaluations.\n", subsets_to_calculate);
        } else {
            fprintf(output_target, "performing %llu set evaluations.\n", expected_total);
        }
    }
#else
    if (!started_from_checkpoint) {
        if (doing_slice) {
            fprintf(output_target, "performing %u set evaluations.<br>\n", subsets_to_calculate);
        } else {
            fprintf(output_target, "performing %llu set evaluations.<br>\n", expected_total);
        }
    }
#endif

    if (started_from_checkpoint) {
        if (iteration >= expected_total) {
            fprintf(stderr, "starting subset [%u] > total subsets [%llu]\n", starting_subset, expected_total);
            fprintf(stderr, "quitting.\n");
            exit(0);
        }
        generate_ith_subset(iteration, subset, subset_size, max_set_value);
    } else if (doing_slice) {
        if (starting_subset >= expected_total) {
            fprintf(stderr, "starting subset [%u] > total subsets [%llu]\n", starting_subset, expected_total);
            fprintf(stderr, "quitting.\n");
            exit(0);
        }
        generate_ith_subset(starting_subset, subset, subset_size, max_set_value);
    } else {
        for (unsigned int i = 0; i < subset_size - 1; i++) subset[i] = i + 1;
        subset[subset_size - 1] = max_set_value;
    }

    sums = new unsigned int[max_sums_length];
    new_sums = new unsigned int[max_sums_length];

    bool success;

#ifdef _BOINC_
    if (!started_from_checkpoint) {
        fprintf(output_target, "<tested_subsets>\n");
        fflush(output_target);
    }
#endif

    while (subset[0] <= (max_set_value - subset_size + 1)) {
        success = test_subset(subset, subset_size, iteration, starting_subset, doing_slice);

        if (success) {
            pass++;
        } else {
            fail++;
            failed_sets.push_back(starting_subset + iteration);
        }

        generate_next_subset(subset, subset_size, max_set_value);

        iteration++;
        if (doing_slice && iteration >= subsets_to_calculate) break;

#ifdef ENABLE_CHECKPOINTING
        /**
         *  Need to checkpoint if we found a failed set, otherwise the output file might contain duplicates
         */
        if (!success || (iteration % 10000) == 0) {
            double progress;
            if (doing_slice) {
                progress = (double)iteration / (double)subsets_to_calculate;
            } else {
                progress = (double)iteration / (double)expected_total;
           }
#ifdef _BOINC_
            boinc_fraction_done(progress);
#endif
//            printf("\r%lf", progress);

            if (!success || (iteration % 60000000) == 0) {      //this works out to be a checkpoint every 10 seconds or so
//                fprintf(stderr, "\n*****Checkpointing! *****\n");
                write_checkpoint(checkpoint_file, iteration, pass, fail, failed_subsets);
#ifdef _BOINC_
                boinc_checkpoint_completed();
#endif
            }
        }
#endif
    }

#ifdef _BOINC_
    fprintf(output_target, "</tested_subsets>\n");
    fprintf(output_target, "<extra_info>\n");
#endif

    /**
     * pass + fail should = M! / (N! * (M - N)!)
     */

#ifndef HTML_OUTPUT
    if (doing_slice) {
        fprintf(output_target, "expected to compute %u sets\n", subsets_to_calculate);
    } else {
        fprintf(output_target, "the expected total number of sets is: %llu\n", expected_total);
    }
    fprintf(output_target, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
#else
    if (doing_slice) {
        fprintf(output_target, "expected to compute %u sets<br>\n", subsets_to_calculate);
    } else {
        fprintf(output_target, "the expected total number of sets is: %llu<br>\n", expected_total);
    }
    fprintf(output_target, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.<br>\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
#endif

#ifdef _BOINC_
    fprintf(output_target, "</extra_info>\n");
#endif

    delete [] subset;
    delete [] sums;
    delete [] new_sums;

#ifdef TIMESTAMP
    time_t end_time;
    time( &end_time );
    fprintf(output_target, "end time: %s", ctime(&end_time) );
    fprintf(output_target, "running time: %ld\n", end_time - start_time);
#endif

#ifdef _BOINC_
    boinc_finish(0);
#endif

#ifdef HTML_OUTPUT
    fprintf(output_target, "</tt>\n");
    fprintf(output_target, "<br>\n");
    fprintf(output_target, "\n");
    fprintf(output_target, "<hr width=\"100%%\">\n");
    fprintf(output_target, "Copyright &copy; Travis Desell, Tom O'Neil and the University of North Dakota, 2012\n");
    fprintf(output_target, "</body>\n");
    fprintf(output_target, "</html>\n");

    if (fail > 0) {
        fprintf(stderr, "[url=http://volunteer.cs.und.edu/subset_sum/download/set_%luc%lu.html]%lu choose %lu[/url] -- %llu failures\n", max_set_value, subset_size, max_set_value, subset_size, fail);
    } else {
        fprintf(stderr, "[url=http://volunteer.cs.und.edu/subset_sum/download/set_%luc%lu.html]%lu choose %lu[/url] -- pass\n", max_set_value, subset_size, max_set_value, subset_size);
    }
#endif

    return 0;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR Args, int WinMode){
    LPSTR command_line;
    char* argv[100];
    int argc;

    command_line = GetCommandLine();
    argc = parse_command_line( command_line, argv );
    return main(argc, argv);
}
#endif

