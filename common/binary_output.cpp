#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "stdint.h"
#include "binary_output.hpp"

#include "../undvc_common/parse_xml.hxx"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <boost/multiprecision/gmp.hpp>

using namespace std;

using boost::multiprecision::mpz_int;

/**
 *  Some (rough) conversion functions.  These are lossy
 *  and will lose precision, especially converting to an
 *  int if the mpz_int is larger.  But that shouldn't happen
 *  at least in this code.
 */
uint32_t mpz_int_to_uint32_t(mpz_int value) {
    mpz_t temp;
    mpz_init(temp);
    mpz_set(temp, value.backend().data());
    return mpz_get_si(temp);
} 

double mpz_int_to_double(mpz_int value) {
    mpz_t temp;
    mpz_init(temp);
    mpz_set(temp, value.backend().data());
    return mpz_get_d(temp);
} 

template <>
void string_to_vector<mpz_int>(string s, vector<mpz_int> &v) {
    v.clear();

    vector<std::string> split_string;
    boost::split(split_string, s, boost::is_any_of("[], "));

    for (uint32_t i = 0; i < split_string.size(); i++) {
        if (split_string[i].size() > 0) {
            v.push_back( mpz_int(split_string[i]) );
        }   
    }   
}

template <>
void parse_xml_vector<mpz_int>(string xml, const char tag[], vector<mpz_int> &result) throw (string) {
    string start_tag("<");
    start_tag.append(tag);
    start_tag.append(">");

    string end_tag("</");
    end_tag.append(tag);
    end_tag.append(">");

    int start = xml.find(start_tag, 0) + start_tag.size();
    int end = xml.find(end_tag, 0); 
    int length = end - start;

    if (length > 0) {
        string_to_vector<mpz_int>(xml.substr(start, length), result);
        return;
    } else if (length == 0) {
        return;
    } else {
        ostringstream oss;
        oss << "Tag '" << tag << "' was not found, find(" << start_tag << ") returned " << start << ", find(" << end_tag << ") returned " << end << ", error in file [" << __FILE__ << "] on line [" << __LINE__ << "]" << endl;
        throw oss.str();
    }   
}


/**
 *  Convert an mpz_int to an (un-marked up) string of 0s and 1s.
 */
string mpz_int_to_binary_string(mpz_int x) {
    string s;
    do {
        s.push_back('0' + mpz_int_to_uint32_t(x & 1));
    } while (x >>= 1);
    reverse(s.begin(), s.end());
    return s;
}


/**
 *  Print out all the elements in a subset
 */
void print_subset(ostream *output_target, const uint32_t *subset, const uint32_t subset_size) {
    ostringstream set;

    set << "[";
    for (uint32_t i = 0; i < subset_size; i++) {
        set << setw(4) << subset[i];
    }
    set << "]";

    string set_string = set.str();

    boost::replace_all(set_string, " ", "&nbsp;");

    *output_target << set_string;
}

/**
 * Print out an array of bits, coloring the required subsets green, if there is a missing sum (a 0) it is colored red
 */
string mpz_int_to_color_binary_string(mpz_int bits, uint32_t min, uint32_t max) {
    string s = mpz_int_to_binary_string(bits);
    string s_colored;

    for (uint32_t i = 0; i < s.size(); i++) {
        if (i == s.size() - max) s_colored.append("<b><span class=\"courier_green\">");

        if (i < s.size() - max || i > s.size() - min) {
            s_colored.push_back(s[i]);
        } else {
            if (s[i] == '0') {
                s_colored.append("<span class=\"courier_red\">0</span>");
            } else {
                s_colored.push_back('1');
            }
        }

        if (i == s.size() - min) s_colored.append("</span></b>");
    }

    return s_colored;
}

uint32_t max_digits = 0;
uint32_t max_binary_width = 0;

uint32_t get_number_digits(mpz_int value) {
    ostringstream value_str;
    value_str << value;

    return value.str().size();
}

void initialize_print_widths(mpz_int max_iteration, uint32_t max_set_value, uint32_t subset_size) {
    max_digits = get_number_digits(max_iteration); //number of digits in the max iteration, in base 10

    max_binary_width = 0;
    for (uint32_t i = 0; i < subset_size; i++) {
        max_binary_width += max_set_value - subset_size + i + 1;
    }
}

void print_subset_calculation(ostream *output_target, mpz_int iteration, uint32_t *subset, const uint32_t subset_size, const bool success) {
    /**
     * First calculate the bits of the subset sums
     */
    mpz_int sums = 0;
    mpz_int new_sums = 0;

    uint32_t current;
    for (uint32_t i = 0; i < subset_size; i++) {
        current = subset[i];
        //cout << "current: " << current << endl;

        new_sums = sums << current;
        sums |= new_sums;
        sums |= 1 << (current - 1);
    }

    for (uint32_t i = get_number_digits(iteration); i < max_digits + 5; i++) {
        *output_target << "&nbsp;";
    }

    *output_target << iteration << "&nbsp;";
    print_subset(output_target, subset, subset_size);
    *output_target << " = ";

    uint32_t M = subset[subset_size - 1];
    uint32_t max_subset_sum = 0;

    for (uint32_t i = 0; i < subset_size; i++) max_subset_sum += subset[i];

    uint32_t min = M;
    uint32_t max = max_subset_sum - M;

    uint32_t whitespace_count = max_binary_width - mpz_int_to_binary_string(sums).size();
    for (uint32_t i = 0; i < whitespace_count; i++) *output_target << "0";

    string s_colored = mpz_int_to_color_binary_string(sums, min, max);
    *output_target << s_colored;

    *output_target << "  match " << setw(4) << min << " to " << setw(4) << max;
    if (success)    *output_target << " = <span class=\"courier_green\">pass</span><br>" << endl;
    else            *output_target << " = <span class=\"courier_red\">fail</span><br>" << endl;

    output_target->flush();
}

void print_html_header(ostream *output_target, uint32_t max_set_value, uint32_t subset_size) {
    /**
     *  Make the HTML header
     */
    *output_target << "<!DOCTYPE html PUBLIC \"-//w3c//dtd html 4.0 transitional//en\">" << endl;
    *output_target << "<html>" << endl;
    *output_target << "<head>" << endl;
    *output_target << "  <meta http-equiv=\"Content-Type\"" << endl;
    *output_target << " content=\"text/html; charset=iso-8859-1\">" << endl;
    *output_target << "  <meta name=\"GENERATOR\"" << endl;
    *output_target << " content=\"Mozilla/4.76 [en] (X11; U; Linux 2.4.2-2 i686) [Netscape]\">" << endl;
    *output_target << "  <title>" << max_set_value << " choose " << subset_size << "</title>" << endl;
    *output_target << "" << endl;
    *output_target << "<style type=\"text/css\">" << endl;
    *output_target << "    .courier_green {" << endl;
    *output_target << "        color: #008000;" << endl;
    *output_target << "    }   " << endl;
    *output_target << "</style>" << endl;
    *output_target << "<style type=\"text/css\">" << endl;
    *output_target << "    .courier_red {" << endl;
    *output_target << "        color: #FF0000;" << endl;
    *output_target << "    }   " << endl;
    *output_target << "</style>" << endl;
    *output_target << "" << endl;
    *output_target << "</head><body>" << endl;
    *output_target << "<h1>" << max_set_value << " choose " << subset_size << "</h1>" << endl;
    *output_target << "<hr width=\"100%%\">" << endl;
    *output_target << "" << endl;
    *output_target << "<br>" << endl;
    *output_target << "<tt>" << endl;
}

void print_html_footer(ostream *output_target) {
    *output_target << "</tt>" << endl;
    *output_target << "<br>" << endl << endl;
    *output_target << "<hr width=\"100%%\">" << endl;
    *output_target << "Copyright &copy; Travis Desell, Tom O'Neil and the University of North Dakota, 2012" << endl;
    *output_target << "</body>" << endl;
    *output_target << "</html>" << endl;
}
