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

#include "../common/generate_subsets.hpp"
#include "../common/binary_output.hpp"
#include "../common/n_choose_k.hpp"


#include <boost/multiprecision/gmp.hpp>


using namespace std;

using boost::multiprecision::mpz_int;

string checkpoint_file = "sss_checkpoint.txt";
string output_filename = "failed_sets.txt";

vector<mpz_int> *failed_sets = new vector<mpz_int>();

uint32_t checksum = 0;

mpz_int sums;
mpz_int new_sums;


/**
 *  Tests to see if a subset all passes the subset sum hypothesis
 */
static inline bool test_subset(const uint32_t *subset, const uint32_t subset_size) {
    //this is also symmetric.  TODO: Only need to check from the largest element in the set (9) to the sum(S)/2 == (13), need to see if everything between 9 and 13 is a 1
    uint32_t M = subset[subset_size - 1];
    uint32_t max_subset_sum = 0;

    for (uint32_t i = 0; i < subset_size; i++) max_subset_sum += subset[i];
    
    sums = 0;
    new_sums = 0;

    uint32_t current;
    for (uint32_t i = 0; i < subset_size; i++) {
        current = subset[i];

        new_sums = sums << current;
        sums |= new_sums;
        sums != 1 << (current - 1);
    }

    mpz_int target = 0;
    for (uint32_t i = M; i < max_subset_sum - M; i++) {
        target |= 1 << i;
    }

    bool success = (target & sums) == target;   //the sums bits are all ones between M and max_subset_sum - M

#ifdef _BOINC_
    //Alternate checksum calculation with overflow detection
    mpz_int ac = sums % UINT32_MAX;
    uint32_t add_to_checksum = ac;

    if (UINT32_MAX - checksum <= add_to_checksum) {
        checksum += add_to_checksum;
    } else { // avoid the overflow
        checksum = add_to_checksum - (UINT32_MAX - checksum);
    } 
#endif
    return success;
}

void write_checkpoint(string filename, const mpz_int iteration, const mpz_int pass, const mpz_int fail, const vector<mpz_int> *failed_sets, const uint32_t checksum) {
#ifdef _BOINC_
    string output_path;
    int retval = boinc_resolve_filename_s(filename.c_str(), output_path);
    if (retval) {
        cerr << "APP: error writing checkpoint (resolving checkpoint file name)" << endl;
        return;
    }   

    ofstream checkpoint_file(output_path.c_str());
#else
    ofstream checkpoint_file(filename.c_str());
#endif
    if (!checkpoint_file.is_open()) {
        cerr << "APP: error writing checkpoint (opening checkpoint file)" << endl;
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

bool read_checkpoint(string sites_filename, mpz_int &iteration, mpz_int &pass, mpz_int &fail, vector<mpz_int> *failed_sets, uint32_t &checksum) {
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
        cerr << "ERROR: malformed checkpoint! could not read 'iteration'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    sites_file >> s >> pass;
    if (s.compare("pass:") != 0) {
        cerr << "ERROR: malformed checkpoint! could not read 'pass'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    sites_file >> s >> fail;
    if (s.compare("fail:") != 0) {
        cerr << "ERROR: malformed checkpoint! could not read 'fail'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    sites_file >> s >> checksum;
    if (s.compare("checksum:") != 0) {
        cerr << "ERROR: malformed checkpoint! could not read 'checksum'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }
    cerr << "read checksum " << checksum << " from checkpoint." << endl;

    uint32_t failed_sets_size = 0;
    sites_file >> s >> failed_sets_size;
    if (s.compare("failed_sets:") != 0) {
        cerr << "ERROR: malformed checkpoint! could not read 'failed_sets'" << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    uint64_t current;
    for (uint32_t i = 0; i < failed_sets_size; i++) {
        sites_file >> current;
        failed_sets->push_back(current);
        if (!sites_file.good()) {
            cerr << "ERROR: malformed checkpoint! only read '" << i << "' of '" << failed_sets_size << "' failed sets." << endl;
#ifdef _BOINC_
            boinc_finish(1);
#endif
            exit(1);
        }
    }

    return true;
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
#ifdef _BOINC_
            boinc_finish(1);
#endif
            exit(1);
        }
        
        result += place * val;
        place *= 10;
    }

    return result;
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
        cerr << "ERROR, wrong command line arguments." << endl;
        cerr << "USAGE:" << endl;
        cerr << "\t./subset_sum <M> <N> [<i> <count>]" << endl << endl;
        cerr << "argumetns:" << endl;
        cerr << "\t<M>      :   The maximum value allowed in the sets." << endl;
        cerr << "\t<N>      :   The number of elements allowed in a set." << endl;
        cerr << "\t<i>      :   (optional) start at the <i>th generated subset." << endl;
        cerr << "\t<count>  :   (optional) only test <count> subsets (starting at the <i>th subset)." << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    uint32_t max_set_value = parse_t<uint32_t>(argv[1]);
    uint32_t subset_size = parse_t<uint32_t>(argv[2]);

    /**
     *  mpz_int is a boost multi-precision integer class.  This allows for arbitrarily large
     *  integers, which are required when M > 53ish.
     */
    mpz_int starting_subset = 0;
    mpz_int iteration = 0;
    mpz_int pass = 0;
    mpz_int fail = 0;
    mpz_int subsets_to_calculate = 0;

    bool doing_slice = false;

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


    bool started_from_checkpoint = read_checkpoint(checkpoint_file, iteration, pass, fail, failed_sets, checksum);

    ofstream *output_target;

#ifdef _BOINC_
    string output_path;
    retval = boinc_resolve_filename_s(output_filename.c_str(), output_path);
    if (retval) {
        cerr << "APP: error opening output file for failed sets." << endl;
        boinc_finish(1);
        exit(1);
    }

    if (started_from_checkpoint) {
        output_target = new ofstream(output_path.c_str(), ios::out | ios::app);
    } else {
        output_target = new ofstream(output_path.c_str(), ios::out);
    }
#endif

    if (!started_from_checkpoint) {
        *output_target << "max_set_value: " << max_set_value << ", subset_size: " << subset_size << endl;

        if (max_set_value < subset_size) {
            cerr << "Error max_set_value < subset_size. Quitting." << endl;
#ifdef _BOINC_
            boinc_finish(1);
#endif
            exit(1);
        }
    } else {
        cerr << "Starting from checkpoint on iteration " << iteration << ", with " << pass << " pass, " << fail << " fail." << endl;
    }

    /**
     *  Determine, and print out, how many subset sums need to be
     *  calculated.
     */
    mpz_int expected_total = n_choose_k(max_set_value - 1, subset_size - 1);
    if (doing_slice) {
        *output_target << "performing " << subsets_to_calculate << " set evaluations." << endl;
    } else {
        *output_target << "performing " << expected_total << " set evaluations." << endl;
    }


    if (starting_subset + iteration > expected_total) {
        cerr << "starting subset [" << starting_subset + iteration << "] > total subsets [" << expected_total << "]" << endl;
        cerr << "quitting." << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    if (doing_slice && starting_subset + subsets_to_calculate > expected_total) {
        cerr << "starting subset [" << starting_subset << "] + subsets to calculate [" << subsets_to_calculate << "] > total subsets [" << expected_total << "]" << endl;
        cerr << "quitting." << endl;
#ifdef _BOINC_
        boinc_finish(1);
#endif
        exit(1);
    }

    /**
     *  Initialize the first subset to calculate the sum of
     */
    uint32_t *subset = new uint32_t[subset_size];

    if (started_from_checkpoint || doing_slice) {
        generate_ith_subset(starting_subset + iteration, subset, subset_size, max_set_value);
    } else {
        for (uint32_t i = 0; i < subset_size - 1; i++) subset[i] = i + 1;
        subset[subset_size - 1] = max_set_value;
    }

    bool success;

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
//                cerr << "\n*****Checkpointing! *****" << endl;
//                cerr << "CHECKSUM: " << checksum << endl;

//                cout << "[";
//                for (uint32_t i = 0; i < subset_size; i++) {
//                    cout << setw(4) << subset[i];
//                }
//                cout << "]";

//                if (!success) cout << " fail: " << fail << ", failed_subsets.size(): " << failed_sets->size() << "\n";
//                else cout << endl;

                write_checkpoint(checkpoint_file, iteration, pass, fail, failed_sets, checksum);
#ifdef _BOINC_
                boinc_checkpoint_completed();
#endif
            }
        }
#endif
    }

#ifdef _BOINC_
    *output_target << "<checksum>" << checksum << "</checksum>" << endl;
    *output_target << "<uint32_max>" << UINT32_MAX << "</uint32_max>" << endl;
    *output_target << "<failed_subsets>";

    cerr << "<checksum>" << checksum << "</checksum>" << endl;
    cerr << "<uint32_max>" << UINT32_MAX << "</uint32_max>" << endl;
    cerr << "<failed_subsets>";
#endif

    for (uint32_t i = 0; i < failed_sets->size(); i++) {
        generate_ith_subset(failed_sets->at(i), subset, subset_size, max_set_value);

#ifdef _BOINC_
        *output_target << " " << failed_sets->at(i);
#endif
        cerr << " " << failed_sets->at(i);
    }

#ifdef _BOINC_
    *output_target << "</failed_subsets>" << endl;
    *output_target << "<extra_info>" << endl;

    cerr << "</failed_subsets>" << endl;
    cerr << "<extra_info>" << endl;
#endif

    /**
     * pass + fail should = M! / (N! * (M - N)!)
     */

    if (doing_slice) {
        cerr << "expected to compute " << subsets_to_calculate << " sets" << endl;
        *output_target << "expected to compute " << subsets_to_calculate << " sets" << endl;
    } else {
        cerr << "the expected total number of sets is: " << expected_total << endl;
        *output_target << "the expected total number of sets is: " <<  expected_total << endl;
    }
    cerr << pass + fail << " total sets, " << pass << " sets passed, " << fail << " sets failed, " << ((double)pass / ((double)pass + (double)fail)) << " success rate." << endl;
    *output_target << pass + fail << " total sets, " << pass << " sets passed, " << fail << " sets failed, " << ((double)pass / ((double)pass + (double)fail)) << " success rate." << endl;

#ifdef _BOINC_
    cerr << "</extra_info>" << endl;
    cerr.flush();

    *output_target << "</extra_info>" << endl;
    output_target->flush();
    output_target->close();
#endif

    delete [] subset;

#ifdef _BOINC_
    boinc_finish(0);
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

