// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// sample_work_generator.cpp: an example BOINC work generator.
// This work generator has the following properties
// (you may need to change some or all of these):
//
// - Runs as a daemon, and creates an unbounded supply of work.
//   It attempts to maintain a "cushion" of 100 unsent job instances.
//   (your app may not work this way; e.g. you might create work in batches)
// - Creates work for the application "example_app".
// - Creates a new input file for each job;
//   the file (and the workunit names) contain a timestamp
//   and sequence number, so they're unique.

#include <unistd.h>
#include <cstdlib>
#include <string>
#include <cstring>

#include "boinc_db.h"
#include "error_numbers.h"
#include "backend_lib.h"
#include "parse.h"
#include "util.h"
#include "svn_version.h"

#include "sched_config.h"
#include "sched_util.h"
#include "sched_msgs.h"
#include "str_util.h"

#include "../common/n_choose_k.hpp"

#define CUSHION 10
    // maintain at least this many unsent results
#define REPLICATION_FACTOR  1

const char* app_name = "subset_sum";
const char* in_template_file = "subset_sum_in.xml";
const char* out_template_file = "subset_sum_out.xml";

char* in_template;
DB_APP app;
int start_time;
int seqno;

// create one new job
//
int make_job(unsigned int max_set_value, unsigned int set_size, unsigned long long starting_set, unsigned long long sets_to_evaluate) {
    DB_WORKUNIT wu;
    char name[256], path[256];
    char command_line[512];
    const char* infiles[0];
    int retval;

    // make a unique name (for the job and its input file)
    //
    sprintf(name, "%s_%u_%u_%llu", app_name, max_set_value, set_size, starting_set);
    fprintf(stdout, "name: '%s'\n", name);

    // Create the input file.
    // Put it at the right place in the download dir hierarchy
    //
    retval = config.download_path(name, path);
    if (retval) return retval;
    FILE* f = fopen(path, "w");
    if (!f) return ERR_FOPEN;
    fprintf(f, "This is the input file for job %s", name);
    fclose(f);

    double number_of_sets = 1e9;        //TODO: figure out the number of sets for a decent sized workunit
    double fpops_per_set = 1e3;         //TODO: figure out an estimate of how many fpops per set calculation
    double fpops_est = fpops_per_set * number_of_sets;


    // Fill in the job parameters
    //
    wu.clear();
    wu.appid = app.id;
    strcpy(wu.name, name);
    wu.rsc_fpops_est = fpops_est;
    wu.rsc_fpops_bound = fpops_est * 100;
    wu.rsc_memory_bound = 1e8;
    wu.rsc_disk_bound = 1e8;
    wu.delay_bound = 86400;
    wu.min_quorum = REPLICATION_FACTOR;
    wu.target_nresults = REPLICATION_FACTOR;
    wu.max_error_results = REPLICATION_FACTOR*4;
    wu.max_total_results = REPLICATION_FACTOR*8;
    wu.max_success_results = REPLICATION_FACTOR*4;
//    infiles[0] = name;

    // Register the job with BOINC
    //
    sprintf(path, "templates/%s", out_template_file);

    sprintf(command_line, " %u %u %llu %llu", max_set_value, set_size, starting_set, sets_to_evaluate);
    fprintf(stdout, "command line: '%s'\n", command_line);

    return create_work(
        wu,
        in_template,
        path,
        config.project_path(path),
        infiles,
        0,
        config,
        command_line
    );
    return 1;
}

int make_jobs(unsigned int max_set_value, unsigned int set_size) {
    int unsent_results;
    int retval;

    check_stop_daemons();   //This checks to see if there is a stop in place, if there is it will exit the work generator.

	//Aaron Comment: retval tells us if the count_unsent_results
	//function is working properly. If it is, then it's value
	//should be 0. Anything creater than 0 and the program exits.
    retval = count_unsent_results(unsent_results, 0);
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "count_unsent_jobs() failed: %s\n", boincerror(retval)
        );
        exit(retval);
    }

    //divide up the sets into mostly equal sized workunits

    unsigned int SETS_PER_WORKUNIT = 2203961430;    //maybe have this as a #define
                                                    //this should give a workunit around 30 minutes

    unsigned long long total_sets = n_choose_k(max_set_value, set_size);
    unsigned long long current_set = 0;

    unsigned long long total_generated = 0;

    while (current_set < total_sets) {
        if ((total_sets - current_set) > SETS_PER_WORKUNIT) {
            make_job(max_set_value, set_size, current_set, SETS_PER_WORKUNIT);
        } else {
            make_job(max_set_value, set_size, current_set, total_sets - current_set);
        }
        current_set += SETS_PER_WORKUNIT;

        total_generated++;
    }

    fprintf(stdout, "workunits generated: %llu\n", total_generated);
}

void usage(char *name) {
    fprintf(stderr, "This is an example BOINC work generator.\n"
        "This work generator has the following properties\n"
        "(you may need to change some or all of these):\n"
        "  It attempts to maintain a \"cushion\" of 100 unsent job instances.\n"
        "  (your app may not work this way; e.g. you might create work in batches)\n"
        "- Creates work for the application \"example_app\".\n"
        "- Creates a new input file for each job;\n"
        "  the file (and the workunit names) contain a timestamp\n"
        "  and sequence number, so that they're unique.\n\n"
        "Usage: %s [OPTION]...\n\n"
        "Options:\n"
        "  --app X                      Application name (default: example_app)\n"
        "  --in_template_file           Input template (default: example_app_in)\n"
        "  --out_template_file          Output template (default: example_app_out)\n"
        "  -m | --max_set_value         The maximum value in the sets to be generated (REQUIRED).\n"       //Added for the subset sum problem
        "  -n | --set_size              The size of the sets to be generated (REQUIRED).\n",               //Added for the subset sum problem
        "  [ -d X ]                     Sets debug level to X.\n"
        "  [ -h | --help ]              Shows this help text.\n"
        "  [ -v | --version ]           Shows version information.\n",
        name
    );
}

int main(int argc, char** argv) {
    int i, retval;
    char buf[256];
    unsigned int max_set_value, set_size;

    bool max_set_value_found = false;
    bool set_size_found = false;

//Aaron Comment: command line flags are explained in descriptions above.
    for (i=1; i<argc; i++) {
        if (is_arg(argv[i], "d")) {
            if (!argv[++i]) {
                log_messages.printf(MSG_CRITICAL, "%s requires an argument\n\n", argv[--i]);
                usage(argv[0]);
                exit(1);
            }
            int dl = atoi(argv[i]);
            log_messages.set_debug_level(dl);
            if (dl == 4) g_print_queries = true;
        } else if (!strcmp(argv[i], "--app")) {
            app_name = argv[++i];
        } else if (!strcmp(argv[i], "--in_template_file")) {
            in_template_file = argv[++i];
        } else if (!strcmp(argv[i], "--out_template_file")) {
            out_template_file = argv[++i];
        } else if (is_arg(argv[i], "h") || is_arg(argv[i], "help")) {
            usage(argv[0]);
            exit(0);
        } else if (is_arg(argv[i], "v") || is_arg(argv[i], "version")) {
            printf("%s\n", SVN_VERSION);
            exit(0);

        } else if (is_arg(argv[i], "m") || is_arg(argv[i], "max_set_value")) {
            max_set_value = atoi(argv[++i]);
            max_set_value_found = true;

        } else if (is_arg(argv[i], "n") || is_arg(argv[i], "set_size")) {
            set_size = atoi(argv[++i]);
            set_size_found = true;

        } else {
            log_messages.printf(MSG_CRITICAL, "unknown command line argument: %s\n\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    if ( !(max_set_value_found && set_size_found) ) {
        usage(argv[0]);
        exit(0);
    }

//Aaron Comment: if at any time the retval value is greater than 0, then the program
//has failed in some manner, and the program then exits.

//Aaron Comment: processing project's config file.
    retval = config.parse_file();
    if (retval) {
        log_messages.printf(MSG_CRITICAL,
            "Can't parse config.xml: %s\n", boincerror(retval)
        );
        exit(1);
    }

//Aaron Comment: opening connection to database.
    retval = boinc_db.open(
        config.db_name, config.db_host, config.db_user, config.db_passwd
    );
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "can't open db\n");
        exit(1);
    }

//Aaron Comment: looks for applicaiton to be run. If not found, program exits.
    sprintf(buf, "where name='%s'", app_name);
    if (app.lookup(buf)) {
        log_messages.printf(MSG_CRITICAL, "can't find app %s\n", app_name);
        exit(1);
    }

//Aaron Comment: looks for work templates, if cannot find, or are corrupted,
//the program exits.
    sprintf(buf, "templates/%s", in_template_file);
    if (read_file_malloc(config.project_path(buf), in_template)) {
        log_messages.printf(MSG_CRITICAL, "can't read input template %s\n", buf);
        exit(1);
    }

//Aaron Comment: if work generator passes all startup tests, the main work gneration
//loop is called.
    start_time = time(0);
    seqno = 0;

    log_messages.printf(MSG_NORMAL, "Starting\n");

    if (max_set_value <= set_size) {
        fprintf(stderr, "ERROR: max_set_value (%u) <= set_size (%u)\n", max_set_value, set_size);
        exit(1);
    }

    make_jobs(max_set_value, set_size);
}
