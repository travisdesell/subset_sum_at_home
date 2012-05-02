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

#include "stdint.h"

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

#include "bit_logic.hpp"
#include "../common/output.hpp"
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
static inline bool test_subset(const uint32_t *subset, const uint32_t subset_size, const uint64_t iteration, const uint32_t starting_subset, const bool doing_slice) {
    //this is also symmetric.  TODO: Only need to check from the largest element in the set (9) to the sum(S)/2 == (13), need to see if everything between 9 and 13 is a 1
    uint32_t M = subset[subset_size - 1];
    uint32_t max_subset_sum = 0;

    for (uint32_t i = 0; i < subset_size; i++) max_subset_sum += subset[i];
    
    for (uint32_t i = 0; i < max_sums_length; i++) {
        sums[i] = 0;
        new_sums[i] = 0;
    }

//    fprintf(output_target, "\n");
    uint32_t current;
    for (uint32_t i = 0; i < subset_size; i++) {
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

#ifdef _BOINC_
    //Calculate a checksum for verification on BOINC
//    for (uint32_t i = 0; i < max_sums_length; i++) checksum += sums[i];

    //Alternate checksum calculation with overflow detection
    for (uint32_t i = 0; i < max_sums_length; i++) {
        if (UINT32_MAX - checksum <= sums[i]) {
            checksum += sums[i];
        } else { // avoid the overflow
            checksum = sums[i] - (UINT32_MAX - checksum);
        } 
    }

#endif
    return success;
}

static inline void generate_ith_subset(uint64_t i, uint32_t *subset, uint32_t subset_size, uint32_t max_set_value) {
    uint32_t pos = 0;
    uint32_t current_value = 1;
    uint64_t nck;

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

static inline void generate_next_subset(uint32_t *subset, uint32_t subset_size, uint32_t max_set_value) {
    uint32_t current = subset_size - 2;
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

void write_checkpoint(string filename, const uint64_t iteration, const uint64_t pass, const uint64_t fail, const vector<uint64_t> *failed_sets, const uint32_t checksum) {
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
    checkpoint_file << "checksum: " << checksum << endl;

    checkpoint_file << "failed_sets: " << failed_sets->size() << endl;
    for (uint32_t i = 0; i < failed_sets->size(); i++) {
        checkpoint_file << " " << failed_sets->at(i);
    }
    checkpoint_file << endl;

    checkpoint_file.close();
}

bool read_checkpoint(string sites_filename, uint64_t &iteration, uint64_t &pass, uint64_t &fail, vector<uint64_t> *failed_sets, uint32_t &checksum) {
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

    sites_file >> s >> checksum;
    if (s.compare("checksum:") != 0) {
        fprintf(stderr, "ERROR: malformed checkpoint! could not read 'checksum'\n");
        exit(0);
    }

    uint32_t failed_sets_size = 0;
    sites_file >> s >> failed_sets_size;
    if (s.compare("failed_sets:") != 0) {
        fprintf(stderr, "ERROR: malformed checkpoint! could not read 'failed_sets'\n");
        exit(0);
    }

    uint64_t current;
    for (uint32_t i = 0; i < failed_sets_size; i++) {
        sites_file >> current;
        failed_sets->push_back(current);
        if (!sites_file.good()) {
            fprintf(stderr, "ERROR: malformed checkpoint! only read '%u' of '%u' failed sets. \n", i, failed_sets_size);
            exit(0);
        }
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

    uint64_t iteration = 0;
    uint64_t pass = 0;
    uint64_t fail = 0;

#ifdef ENABLE_CHECKPOINTING
    bool started_from_checkpoint = read_checkpoint(checkpoint_file, iteration, pass, fail, failed_sets, checksum);
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
    uint32_t starting_subset = 0;
    uint32_t subsets_to_calculate = 0;

    if (argc == 5) {
        doing_slice = true;
        starting_subset = atoi(argv[3]);
        subsets_to_calculate = atoi(argv[4]);
    }

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
//    fprintf(output_target, "%15u ", 296010);
//    generate_ith_subset(296010, subset, subset_size, max_set_value);
//    print_subset(subset, subset_size);
//    fprintf(output_target, "\n");

    uint64_t expected_total = n_choose_k(max_set_value - 1, subset_size - 1);

#ifdef HTML_OUTPUT
    max_digits = ceil(log10(expected_total));
#endif

//    for (uint64_t i = 0; i < expected_total; i++) {
//        fprintf(output_target, "%15llu ", i);
//        generate_ith_subset(i, subset, subset_size, max_set_value);
//        print_subset(subset, subset_size);
//        fprintf(output_target, "\n");
//    }


#ifndef HTML_OUTPUT
    if (doing_slice) {
        fprintf(output_target, "performing %u set evaluations.\n", subsets_to_calculate);
    } else {
        fprintf(output_target, "performing %llu set evaluations.\n", expected_total);
    }
#else
    if (doing_slice) {
        fprintf(output_target, "performing %u set evaluations.<br>\n", subsets_to_calculate);
    } else {
        fprintf(output_target, "performing %llu set evaluations.<br>\n", expected_total);
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
        for (uint32_t i = 0; i < subset_size - 1; i++) subset[i] = i + 1;
        subset[subset_size - 1] = max_set_value;
    }

    sums = new uint32_t[max_sums_length];
    new_sums = new uint32_t[max_sums_length];

    bool success;

    while (subset[0] <= (max_set_value - subset_size + 1)) {
        success = test_subset(subset, subset_size, iteration, starting_subset, doing_slice);

        if (success) {
            pass++;
        } else {
            fail++;
            failed_sets->push_back(starting_subset + iteration);
        }

        generate_next_subset(subset, subset_size, max_set_value);

        iteration++;
        if (doing_slice && iteration >= subsets_to_calculate) break;

#ifdef VERBOSE
#ifndef FALSE_ONLY
        print_subset_calculation(iteration, subset, subset_size, success);
#endif
#endif

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
//
            if (!success || (iteration % 60000000) == 0) {      //this works out to be a checkpoint every 10 seconds or so
//                fprintf(stderr, "\n*****Checkpointing! *****\n");
                write_checkpoint(checkpoint_file, iteration, pass, fail, failed_sets, checksum);
#ifdef _BOINC_
                boinc_checkpoint_completed();
#endif
            }
        }
#endif
    }

#ifdef _BOINC_
    fprintf(output_target ,"<checksum>%u</checksum>\n", checksum);
    fprintf(output_target, "<uint32_max>%u</uint32_max>\n", UINT32_MAX);
    fprintf(output_target, "<failed_subsets>\n");

    fprintf(stderr,"<checksum>%u</checksum>\n", checksum);
    fprintf(stderr, "<uint32_max>%u</uint32_max>\n", UINT32_MAX);
    fprintf(stderr, "<failed_subsets>\n");
#endif

#ifdef VERBOSE
#ifdef FALSE_ONLY
    for (uint32_t i = 0; i < failed_sets->size(); i++) {
        generate_ith_subset(failed_sets->at(i), subset, subset_size, max_set_value);
#ifdef _BOINC_
        fprintf(stderr, " %llu", failed_sets->at(i));
        fprintf(output_target, " %llu", failed_sets->at(i));
#else
        print_subset_calculation(failed_sets->at(i), subset, subset_size, false);
#endif
    }
#endif
#endif

#ifdef _BOINC_
    fprintf(stderr, "\n</failed_subsets>\n");
    fprintf(stderr, "<extra_info>\n");
    fprintf(output_target, "\n</failed_subsets>\n");
    fprintf(output_target, "<extra_info>\n");
#endif

    /**
     * pass + fail should = M! / (N! * (M - N)!)
     */

#ifndef HTML_OUTPUT
    if (doing_slice) {
        fprintf(stderr, "expected to compute %u sets\n", subsets_to_calculate);
        fprintf(output_target, "expected to compute %u sets\n", subsets_to_calculate);
    } else {
        fprintf(stderr, "the expected total number of sets is: %llu\n", expected_total);
        fprintf(output_target, "the expected total number of sets is: %llu\n", expected_total);
    }
    fprintf(stderr, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
    fprintf(output_target, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
#else
    if (doing_slice) {
        fprintf(output_target, "expected to compute %u sets<br>\n", subsets_to_calculate);
        fprintf(stderr, "expected to compute %u sets<br>\n", subsets_to_calculate);
    } else {
        fprintf(output_target, "the expected total number of sets is: %llu<br>\n", expected_total);
        fprintf(stderr, "the expected total number of sets is: %llu<br>\n", expected_total);
    }
    fprintf(output_target, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.<br>\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
    fprintf(stderr, "%llu total sets, %llu sets passed, %llu sets failed, %lf success rate.<br>\n", pass + fail, pass, fail, ((double)pass / ((double)pass + (double)fail)));
#endif

#ifdef _BOINC_
    fprintf(stderr, "</extra_info>\n");
    fflush(stderr);
    fprintf(output_target, "</extra_info>\n");
    fflush(output_target);
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

