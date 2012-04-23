#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>

#include "boinc_db.h"
#include "error_numbers.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"

using namespace std;

/*
 * Given a set of results, check for a canonical result,
 * i.e. a set of at least min_quorum/2+1 results for which
 * that are equivalent according to check_pair().
 *
 * invariants:
 * results.size() >= wu.min_quorum
 * for each result:
 *      result.outcome == SUCCESS
 *      result.validate_state == INIT
 */
int check_set(vector<RESULT>& results, WORKUNIT& wu, int& canonicalid, double&, bool& retry) {
    RESULT canonical_result;
    int retval;
    int n_matches;

    vector<bool> had_error(false, results.size());

    for (int i = 0; i < results.size(); i++) {
        if (had_error[i]) continue;
        vector<bool> matches;
        matches.resize(results.size());

        vector<FILE_INFO> files;
        retval = get_output_file_infos(results[i], files);
        if (retval) {
            log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] check_pair: can't get output filenames for new result\n", results[i].id, results[i].name);
            return retval;
        }

        for (unsigned int i=0; i<files.size(); i++) {
            FILE_INFO& fi = files[i];
            if (fi.no_validate) continue;
            string file_path = fi.path;

            cout << "file path: " << file_path << endl;

        }


/*
        try {
            result_fitness_i = parse_xml<double>(results[i].stderr_out, "search_likelihood", atof);
        } catch (string error_message) {
            log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", results[i].id, results[i].name, error_message.c_str());
            results[i].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            results[i].validate_state = VALIDATE_STATE_INVALID;
            had_error[i] = true;
            continue;
        }

        n_matches = 1;

        for (int j = 0; j < results.size(); j++) {
            if (had_error[j]) continue;

            if (i == j) {
                matches[i] = true;
                continue;
            }

            try {
                result_fitness_j = parse_xml<double>(results[j].stderr_out, "search_likelihood", atof);
            } catch (string error_message) {
                log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with error: %s\n", results[j].id, results[j].name, error_message.c_str());
                results[j].outcome = RESULT_OUTCOME_VALIDATE_ERROR;
                results[j].validate_state = VALIDATE_STATE_INVALID;
                matches[j] = false;
                had_error[j] = true;
                continue;
            }

            if (fabs(result_fitness_j - result_fitness_i) < FITNESS_ERROR_BOUND) {
                n_matches++;
                matches[j] = true;
            } else {
                matches[j] = false;
            }
        }

        if (n_matches >= min_quorum) {
            canonicalid = results[i].id;

            for (int k = 0; k < results.size(); k++) {
                if (had_error[k]) continue;

                if (matches[k]) results[k].validate_state = VALIDATE_STATE_VALID;
                else results[k].validate_state = VALIDATE_STATE_INVALID;
            }
            return 0;
        }
*/
    }

    exit(0);

    return 0;
}

void check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry) {
    vector<FILE_INFO> canonical_files;
    vector<FILE_INFO> new_files;
    int retval;

    retval = get_output_file_infos(canonical_result, canonical_files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] check_pair: can't get output filenames for canonical result\n", canonical_result.id, canonical_result.name);
        exit(0);
    }

    retval = get_output_file_infos(new_result, new_files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] check_pair: can't get output filenames for new result\n", new_result.id, new_result.name);
        exit(0);
    }

    for (unsigned int i=0; i < new_files.size(); i++) {
        FILE_INFO& fi = new_files[i];
        if (fi.no_validate) continue;
        string file_path = fi.path;

        cout << "file path: " << file_path << endl;

    }
    exit(0);

/*
    double new_fitness, canonical_fitness;

    try {
        new_fitness = parse_xml<double>(new_result.stderr_out, "search_likelihood", atof);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_pair([RESULT#%d %s]) failed with error: %s\n", new_result.id, new_result.name, error_message.c_str());
        new_result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        new_result.validate_state = VALIDATE_STATE_INVALID;
        return;
    }

    try {
        canonical_fitness = parse_xml<double>(canonical_result.stderr_out, "search_likelihood", atof);
    } catch (string error_message) {
        log_messages.printf(MSG_CRITICAL, "ea_validation_policy check_set([RESULT#%d %s]) failed with TERMINAL error: %s -- COULD NOT PARSE CANONICAL RESULT XML\n", canonical_result.id, canonical_result.name, error_message.c_str());
        exit(0);
    }

    if (fabs(new_fitness - canonical_fitness) < FITNESS_ERROR_BOUND) {
        new_result.validate_state = VALIDATE_STATE_VALID;
    } else {
        new_result.validate_state = VALIDATE_STATE_INVALID;
    }
*/
}

