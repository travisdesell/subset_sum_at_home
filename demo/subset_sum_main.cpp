
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
#include <iomanip>

#include "stdint.h"
/**
 *  Includes for subset sum
 */
#include "opencl_bit_logic.hpp"
#include "../common/bit_logic.hpp"
#include "../common/generate_subsets.hpp"
#include "../common/binary_output.hpp"
#include "../common/n_choose_k.hpp"

using namespace std;

string checkpoint_file = "sss_checkpoint.txt";
string output_filename = "failed_sets.txt";

vector<uint64_t> *failed_sets = new vector<uint64_t>();

uint32_t checksum = 0;

#ifdef HTML_OUTPUT
double max_digits;                  //extern
double max_set_digits;              //extern
#endif

unsigned long int max_sums_length;  //extern
uint32_t *sums;                 //extern
uint32_t *new_sums;             //extern


/**
 *  Tests to see if a subset all passes the subset sum hypothesis
 */
static inline bool test_subset(const uint32_t *subset, const uint32_t subset_size) {
    //this is also symmetric.  TODO: Only need to check from the largest element in the set (9) to the sum(S)/2 == (13), need to see if everything between 9 and 13 is a 1

    uint32_t max_subset_sum = 0;

    for (uint32_t i = 0; i < subset_size; i++) max_subset_sum += subset[i];
    sums = (uint32_t *) malloc(sizeof(uint32_t)*max_subset_sum);

    for (uint32_t i = 0; i < max_sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }

//    *output_target << "\n");
/*TODO
Orginize the code so its more readable
Get the code to compile with original subsetsum code
*/
    //OpenCl version of shift left
    uint32_t M = subset[subset_size - 1];
    cl_shift_left(sums, &max_subset_sum, subset, &subset_size);
    bool success = cl_all_ones(sums, max_sums_length, M, max_subset_sum - M);
    return success;
}


template <class T>
T parse_t(const char* arg) {
    string n(arg);
    T result = 0;
    T place = 1;
    uint16_t val = 0;

    for (int i = (int)n.size() - 1; i >= 0; i--) {
//        cerr << "char[%d]: '%c'\n", i, n[i]);
        if      (n[i] == '0') val = 0;
        else if (n[i] == '1') val = 1;
        else if (n[i] == '2') val = 2;
        else if (n[i] == '3') val = 3;
        else if (n[i] == '4') val = 4;
        else if (n[i] == '5') val = 5;
        else if (n[i] == '6') val = 6;
        else if (n[i] == '7') val = 7;
        else if (n[i] == '8') val = 8;
        else if (n[i] == '9') val = 9;
        else {
            cerr << "ERROR in parse_uint64_t, unrecognized character in string: '" <<  n[i] << "', from argument '" << arg << "', string: '" << n << "', position: " << i << endl;
            exit(1);
        }

        result += place * val;
        place *= 10;
    }

    return result;
}


int main(int argc, char** argv) {
    if (argc != 3 && argc != 5) {
        cerr << "ERROR, wrong command line arguments." << endl;
        cerr << "USAGE:" << endl;
        cerr << "\t./subset_sum <M> <N> [<i> <count>]" << endl << endl;
        cerr << "argumetns:" << endl;
        cerr << "\t<M>      :   The maximum value allowed in the sets." << endl;
        cerr << "\t<N>      :   The number of elements allowed in a set." << endl;
        cerr << "\t<i>      :   (optional) start at the <i>th generated subset." << endl;
        cerr << "\t<count>  :   (optional) only test <count> subsets (starting at the <i>th subset)." << endl;
        exit(1);
    }

    uint32_t max_set_value = parse_t<uint32_t>(argv[1]);

    uint32_t subset_size = parse_t<uint32_t>(argv[2]);

    uint64_t iteration = 0;
    uint64_t pass = 0;
    uint64_t fail = 0;

    bool doing_slice = false;
    uint64_t starting_subset = 0;
    uint64_t subsets_to_calculate = 0;

    n_choose_k_init();      //Initialize the n choose k table

    if (argc == 5) {
        doing_slice = true;
        starting_subset = parse_t<uint64_t>(argv[3]);
        subsets_to_calculate = parse_t<uint64_t>(argv[4]);
        cerr << "argv[1]:       " << argv[1]       << ", argv[2]:     " << argv[2]     << ", argv[3]:         " << argv[3]         << ", argv[4]:              " << argv[4] << endl;
	} else {
		subsets_to_calculate = n_choose_k(max_set_value - 1, subset_size - 1);
        cerr << "argv[1]:       " << argv[1]       << ", argv[2]:     " << argv[2]     << endl;
	}

    cerr << "max_set_value: " << max_set_value << ", subset_size: " << subset_size << ", starting_subset: " << starting_subset << ", subsets_to_calculate: " << subsets_to_calculate << endl;


    ostream *output_target = &cout;

    /**
     *  Calculate the maximum set length (in bits) so we can use this for printing out the values cleanly.
     */
    uint32_t *max_set = new uint32_t[subset_size];
    for (uint32_t i = 0; i < subset_size; i++) max_set[subset_size - i - 1] = max_set_value - i;
    for (uint32_t i = 0; i < subset_size; i++) max_sums_length += max_set[i];

//    sums_length /= 2;
    max_sums_length /= ELEMENT_SIZE;
    max_sums_length++;

    delete [] max_set;

    uint32_t *subset = new uint32_t[subset_size];

//    this caused a problem:
//
//    *output_target << "%15u ", 296010);
//    generate_ith_subset(296010, subset, subset_size, max_set_value);
//    print_subset(subset, subset_size);
//    *output_target << "\n");

    uint64_t expected_total = n_choose_k(max_set_value - 1, subset_size - 1);


//    for (uint64_t i = 0; i < expected_total; i++) {
//        *output_target << "%15llu ", i);
//        generate_ith_subset(i, subset, subset_size, max_set_value);
//        print_subset(subset, subset_size);
//        *output_target << "\n");
//    }

    *output_target << "performing " << expected_total << " set evaluations.";
    *output_target << endl;


    if (starting_subset + iteration > expected_total) {
        cerr << "starting subset [" << starting_subset + iteration << "] > total subsets [" << expected_total << "]" << endl;
        cerr << "quitting." << endl;
        exit(1);
    }

    if (doing_slice && starting_subset + subsets_to_calculate > expected_total) {
        cerr << "starting subset [" << starting_subset << "] + subsets to calculate [" << subsets_to_calculate << "] > total subsets [" << expected_total << "]" << endl;
        cerr << "quitting." << endl;
        exit(1);
    }
    sums = new uint32_t[max_sums_length];
    new_sums = new uint32_t[max_sums_length];

    bool success;

    build_cl_program(max_sums_length, subset_size);
    while (subset[0] <= (max_set_value - subset_size + 1)) {
        success = test_subset(subset, subset_size);

        if (success) {
            pass++;
        } else {
            fail++;
            failed_sets->push_back(starting_subset + iteration);
        }

        generate_next_subset(subset, subset_size, max_set_value);

        iteration++;
        if (doing_slice && iteration >= subsets_to_calculate) break;

    }
    release_cl_program();
    cout << failed_sets->size() << endl;

    for (uint32_t i = 0; i < failed_sets->size(); i++) {
        cout<< "testing" << endl;
        cout << subset_size << " " << max_set_value << endl;
        cout<< "testing" << endl;
        generate_ith_subset(failed_sets->at(i), subset, subset_size, max_set_value);
        cout<< "testing" << endl;
        print_subset_calculation(output_target, failed_sets->at(i), subset, subset_size, false);
    }

    /**
     * pass + fail should = M! / (N! * (M - N)!)
     */

    delete [] subset;
    delete [] sums;
    delete [] new_sums;

    return 0;
}
