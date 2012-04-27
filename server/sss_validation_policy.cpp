#include "config.h"
#include <vector>
#include <cstdlib>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#include "stdint.h"

#include "boinc_db.h"
#include "error_numbers.h"

#include "credit.h"
#include "sched_config.h"
#include "sched_msgs.h"
#include "sched_util.h"

#include "validator.h"
#include "validate_util.h"
#include "validate_util2.h"

#include "../common/util.hpp"

using namespace std;


string get_file_as_string(string file_path) throw (int) {
    //read the entire contents of the file into a string
    ifstream sites_file(file_path.c_str());

    if (!sites_file.is_open()) {
        throw 0;
    }

    std::string fc;

    sites_file.seekg(0, std::ios::end);   
    fc.reserve(sites_file.tellg());
    sites_file.seekg(0, std::ios::beg);

    fc.assign((std::istreambuf_iterator<char>(sites_file)), std::istreambuf_iterator<char>());

    return fc;
}

int get_data_from_result(uint32_t &checksum, string &failed_sets, RESULT &result) {
    vector<FILE_INFO> files;

    int retval = get_output_file_infos(result, files);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] get_data_from_result: can't get output filenames for new result\n", result.id, result.name);
        return retval;
    }

    //There is only one file in the result
    FILE_INFO& fi = files[0];
//    if (fi.no_validate) continue;             I THINK ITS SAFE TO COMMENT THIS OUT
    string file_path = fi.path;

    log_messages.printf(MSG_DEBUG, "file path: %s\n", file_path.c_str());

    string fc;
    try {
        fc = get_file_as_string(file_path);
    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] get_data_from_result: could not open file for result\n", result.id, result.name);
        log_messages.printf(MSG_CRITICAL, "     file path: %s\n", file_path.c_str());
        result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        result.validate_state = VALIDATE_STATE_INVALID;
        throw 0;
    }

    cout << fc << endl;

    try {
        checksum = parse_xml<uint32_t>( fc, "checksum", convert_unsigned_int );
        log_messages.printf(MSG_DEBUG,"checksum: %u\n", checksum);

        failed_sets = parse_xml<string>( fc, "failed_subsets", convert_string );
        std::remove( failed_sets.begin(), failed_sets.end(), '\r' );
        log_messages.printf(MSG_DEBUG, "failed_subsets: '%s'\n", failed_sets.c_str());

    } catch (string error_message) {
        //Should eventually be able to remove this after everyone starts using application 0.04
        try {
            failed_sets = parse_xml<string>( fc, "tested_subsets", convert_string );
            std::remove( failed_sets.begin(), failed_sets.end(), '\r' );
            log_messages.printf(MSG_DEBUG, "failed_sets: '%s'\n", failed_sets.c_str());

        } catch (string error_message) {
            log_messages.printf(MSG_CRITICAL, "sss_validation_policy get_data_from_result([RESULT#%d %s]) failed with error: %s\n", result.id, result.name, error_message.c_str());
            result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
            throw 0;
        }
    }

    return 0;
}


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
    int min_quorum = wu.min_quorum;

    vector<bool> had_error(false, results.size());
    vector<uint32_t> checksums;
    vector<string> failed_vector;

    for (int i = 0; i < results.size(); i++) {
        uint32_t checksum;
        string failed_sets;

        try {
            retval = get_data_from_result(checksum, failed_sets, results[i]);
            if (retval) return retval;

        } catch (int err) {
            had_error[i] = true;
            checksums.push_back(0);
            failed_vector.push_back(" ");
            continue;
        }

        checksums.push_back(checksum);
        failed_vector.push_back(failed_sets);
    }

    uint32_t n_matches;

    for (int i = 0; i < results.size(); i++) {
        if (had_error[i]) continue;

        vector<bool> matches(false, results.size());
        n_matches = 0;

        for (int j = 0; j < results.size(); j++) {
            if (i == 0) {
                matches[j] = true;
                continue;
            }

            if (checksums[i] != checksums[j] && failed_vector[i].compare( failed_vector[j] ) == 0) {
                matches[j] = false;
            } else {
                matches[j] = true;
                n_matches++;
            }
        }

        if (n_matches >= min_quorum) {
            canonicalid = results[i].id;

            for (int k = 0; k < results.size(); k++) {
                if (had_error[k]) continue;

                if (matches[k]) results[k].validate_state = VALIDATE_STATE_VALID;
                else results[k].validate_state = VALIDATE_STATE_INVALID;
            }

            log_messages.printf(MSG_DEBUG, "FOUND CANONICAL RESULT: %d -- %s\n", results[i].id, results[i].name);
//            exit(0);
            return 0;
        }
    }

    log_messages.printf(MSG_DEBUG, "DID NOT FIND CANONICAL WORKUNIT: %d -- %s\n", wu.id, wu.name);
//    exit(0);

    return 0;
}

void check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry) {
    int retval;

    uint32_t new_checksum;
    string new_failed_sets;

    uint32_t canonical_checksum;
    string canonical_failed_sets;

    try {
        retval = get_data_from_result(new_checksum, new_failed_sets, new_result);
        if (retval) return;

    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "sss_validation_policy check_pair([RESULT#%d %s]) failed getting checksum and failed sets from new result.\n", new_result.id, new_result.name);
        return;
    }

    try {
        retval = get_data_from_result(canonical_checksum, canonical_failed_sets, canonical_result);
        if (retval) return;

    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "sss_validation_policy check_pair([RESULT#%d %s]) failed getting checksum and failed sets from canonical result.\n", canonical_result.id, canonical_result.name);
        exit(0);
    }
}

