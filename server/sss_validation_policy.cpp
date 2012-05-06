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

int get_data_from_result(uint32_t &uint32_max, uint32_t &checksum, string &failed_sets, RESULT &result) throw (int) {
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

//    log_messages.printf(MSG_DEBUG, "file path: %s\n", file_path.c_str());

    string fc;
    try {
        fc = get_file_as_string(file_path);
    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] get_data_from_result: could not open file for result\n", result.id, result.name);
        log_messages.printf(MSG_CRITICAL, "     file path: %s\n", file_path.c_str());
        //TODO: retry this result?
        result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
        result.validate_state = VALIDATE_STATE_INVALID;
        throw 0;
    }

//    cout << fc << endl;

    try {
        checksum = parse_xml<uint32_t>( fc, "checksum", convert_unsigned_int );
//        log_messages.printf(MSG_DEBUG,"checksum: %u\n", checksum);

        try {
            uint32_max = parse_xml<uint32_t>( fc, "uint32_max", convert_unsigned_int );
        } catch (string error_message) {
            uint32_max = 0;
        }

        failed_sets = parse_xml<string>( fc, "failed_subsets", convert_string );
//        log_messages.printf(MSG_DEBUG, "failed_subsets: '%s'\n", failed_sets.c_str());

    } catch (string error_message) {
        //Should eventually be able to remove this after everyone starts using application 0.04
        try {
            failed_sets = parse_xml<string>( fc, "tested_subsets", convert_string );
//            log_messages.printf(MSG_DEBUG, "failed_sets: '%s'\n", failed_sets.c_str());

        } catch (string error_message) {
            log_messages.printf(MSG_CRITICAL, "sss_validation_policy get_data_from_result([RESULT#%d %s]) failed with error: %s\n", result.id, result.name, error_message.c_str());
            result.outcome = RESULT_OUTCOME_VALIDATE_ERROR;
            result.validate_state = VALIDATE_STATE_INVALID;
            throw 0;
        }
    }

    uint32_t current = 0, target = 0;

//   std::remove( failed_sets.begin(), failed_sets.end(), '\r' );
    while (target < failed_sets.size()) {
        while (failed_sets[target] == '\r') {
            target++;
        }

        if (failed_sets[target] == '\n') {
            failed_sets[current] = ' ';
        } else {
            failed_sets[current] = failed_sets[target];
        }

        current++;
        target++;
    }
    failed_sets.resize(current);


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

    vector<bool> had_error(results.size(), false);
    vector<uint32_t> checksums(results.size(), 0);
    vector<uint32_t> uint32_maxes(results.size(), 0);
    vector<string> failed_vector(results.size(), " ");

    for (int i = 0; i < results.size(); i++) {
        uint32_t checksum, uint32_max;
        string failed_sets;

        try {
            retval = get_data_from_result(uint32_max, checksum, failed_sets, results[i]);
            if (retval) return retval;

        } catch (int err) {
            log_messages.printf(MSG_CRITICAL, "result[%2d] - id: %10d, error from get_data_from_result.\n", i, results[i].id);
            had_error[i] = true;
            checksums[i] = 0;
            uint32_maxes[i] = 0;
            failed_vector[i] = " ";
            continue;
        }

        checksums[i] = checksum;
        uint32_maxes[i] = uint32_max;
        failed_vector[i] = failed_sets;
    }

    log_messages.printf(MSG_DEBUG, "CHECK SET:\n", results.size());
    log_messages.printf(MSG_DEBUG, "%d results found:\n", results.size());
    for (int i = 0; i < results.size(); i++) {
        if (had_error[i]) {
            log_messages.printf(MSG_DEBUG, "    results[%2d] - id: %10d, checksum: %15u, uint32_max: %15u, had_error: TRUE\n", i, results[i].id, checksums[i], uint32_maxes[i]);
        } else {
            log_messages.printf(MSG_DEBUG, "    results[%2d] - id: %10d, checksum: %15u, uint32_max: %15u, had_error: FALSE\n", i, results[i].id, checksums[i], uint32_maxes[i]);
        }
    }


    uint32_t n_matches;

    for (int i = 0; i < results.size(); i++) {
        if (had_error[i]) continue;

        vector<bool> matches(results.size(), false);
        n_matches = 0;

        for (int j = 0; j < results.size(); j++) {
            if (had_error[j]) continue;

            if (i == j) {
                log_messages.printf(MSG_DEBUG, "        result[%2d] (checksum: %15u) matches results [%2d] (checksum: %15u)\n", i, checksums[i], j, checksums[j]);
                matches[j] = true;
                n_matches++;
                continue;
            }

            if (checksums[i] == checksums[j]) {
                log_messages.printf(MSG_DEBUG, "        result[%2d] (checksum: %15u) matches results [%2d] (checksum: %15u)\n", i, checksums[i], j, checksums[j]);
                if (failed_vector[i].compare( failed_vector[j] ) == 0) {
                    log_messages.printf(MSG_DEBUG, "        result[%2d] (failed_sets) matches results [%2d] (failed_sets)\n", i, j);
                    n_matches++;
                    matches[j] = true;
                } else {
                    log_messages.printf(MSG_CRITICAL, "        result[%2d] (failed_sets) DOES NOT MATCH results [%2d] (failed_sets), but checksums match\n", i, j);
                    log_messages.printf(MSG_CRITICAL, "result[%d] length: %d\n\n", i, failed_vector[i].size());
                    log_messages.printf(MSG_CRITICAL, "result[%d] failed sets: %s\n\n", i, failed_vector[i].c_str());
                    for (int k = 0; k < failed_vector[i].size(); k++) log_messages.printf(MSG_CRITICAL, "string[%d]: %d\n", k, (int)failed_vector[i][k]);
                    log_messages.printf(MSG_CRITICAL, "result[%d] length: %d\n\n", j, failed_vector[j].size());
                    log_messages.printf(MSG_CRITICAL, "result[%d] failed sets: %s\n\n", j, failed_vector[j].c_str());
                    for (int k = 0; k < failed_vector[j].size(); k++) log_messages.printf(MSG_CRITICAL, "string[%d]: %d\n", k, (int)failed_vector[j][k]);
                    log_messages.printf(MSG_CRITICAL, "EXITING.");
                    matches[j] = false;
                    exit(0);
                }
            } else {
                matches[j] = false;
                log_messages.printf(MSG_DEBUG, "        result[%2d] (checksum: %15u) DOES NOT MATCH results [%2d] (checksum: %15u)\n", i, checksums[i], j, checksums[j]);
            }
        }

        log_messages.printf(MSG_DEBUG,"    %d matches found, min_quorum: %d.\n", n_matches, min_quorum);
        if (n_matches >= min_quorum) {
            canonicalid = results[i].id;

            for (int k = 0; k < results.size(); k++) {
                if (had_error[k]) continue;

                if (matches[k]) {
                    log_messages.printf(MSG_DEBUG, "result[%d]: id (%10d) set to VALID\n", k, results[k].id);
                    results[k].validate_state = VALIDATE_STATE_VALID;
                } else {
                    log_messages.printf(MSG_DEBUG, "result[%d]: id (%10d) set to INVALID\n", k, results[k].id);
                    results[k].validate_state = VALIDATE_STATE_INVALID;
                }
            }

            log_messages.printf(MSG_DEBUG, "FOUND CANONICAL RESULT: %d -- %s\n", results[i].id, results[i].name);
            return 0;
        }
    }

    log_messages.printf(MSG_DEBUG, "DID NOT FIND CANONICAL WORKUNIT: %d -- %s\n", wu.id, wu.name);

    return 0;
}

void check_pair(RESULT& new_result, RESULT& canonical_result, bool& retry) {
    int retval;

    uint32_t new_checksum, new_uint32_max;
    string new_failed_sets;

    uint32_t canonical_checksum, canonical_uint32_max;
    string canonical_failed_sets;

    try {
        retval = get_data_from_result(new_uint32_max, new_checksum, new_failed_sets, new_result);
        if (retval) return;

    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "sss_validation_policy check_pair([RESULT#%d %s]) failed getting checksum and failed sets from new result.\n", new_result.id, new_result.name);
        exit(0);
        return;
    }

    try {
        retval = get_data_from_result(canonical_uint32_max, canonical_checksum, canonical_failed_sets, canonical_result);
        if (retval) return;

    } catch (int err) {
        log_messages.printf(MSG_CRITICAL, "sss_validation_policy check_pair([RESULT#%d %s]) failed getting checksum and failed sets from canonical result.\n", canonical_result.id, canonical_result.name);
        exit(0);
    }

    log_messages.printf(MSG_DEBUG, "CHECK_PAIRS:\n");
    if (new_checksum == canonical_checksum) {
        log_messages.printf(MSG_DEBUG, "        new result (checksum: %15u) matches canonical result (checksum: %15u)\n", new_checksum, canonical_checksum);
        if (new_failed_sets.compare( canonical_failed_sets ) == 0) {
            log_messages.printf(MSG_DEBUG, "        new result (failed_sets) matches canonical result (failed_sets)\n");
            new_result.validate_state = VALIDATE_STATE_VALID;
        } else {
            log_messages.printf(MSG_CRITICAL, "        new result (failed_sets) DOES NOT MATCH canonical result (failed_sets), but checksums match\n");
            log_messages.printf(MSG_CRITICAL, "new result failed sets: %s\n\n", new_failed_sets.c_str());
            log_messages.printf(MSG_CRITICAL, "canonical result failed sets: %s\n\n", canonical_failed_sets.c_str());
            new_result.validate_state = VALIDATE_STATE_INVALID;
            exit(0);
        }
    } else {
        new_result.validate_state = VALIDATE_STATE_INVALID;
        log_messages.printf(MSG_DEBUG, "        new result (checksum: %15u) DOES NOT MATCH canonical result (checksum: %15u)\n", new_checksum, canonical_checksum);
        log_messages.printf(MSG_DEBUG, "        new result (uint32_max: %15u) DOES NOT MATCH canonical result (uint32_max: %15u)\n", new_uint32_max, canonical_uint32_max);
    }
}

