#ifndef FGDO_UTIL_HPP
#define FGDO_UTIL_HPP

#include <iostream>
#include <string>
#include <stdint.h>

using namespace std;

const char *FGDO_UTIL_ERRORS[] = { "success", "XML tag not found" };

string convert_string(const char* cstr) {
    string str(cstr);
    return str;
}

uint32_t convert_unsigned_int(const char* cstr) {
    stringstream str_value;
    str_value << cstr;

    uint32_t int_value;
    str_value >> int_value;

    return int_value;
}

template <class T>
vector<T>* string_to_vector(string s, T (*convert)(const char*) ) {
    vector<T> *v = new vector<T>();

    const char *cstr = s.c_str();
    char *pch = strtok((char*)cstr, "[], ");
    while (pch != NULL) {
//        cout << "element: " << pch << endl;
        v->push_back( (*convert)(pch) );

        pch = strtok(NULL, "[], ");
    }

    return v;
} 

template <class T>
string vector_to_string(vector<T> *v) {
    ostringstream oss;

    oss << "[";
    for (int i = 0; i < v->size(); i++) {
        if (i > 0) oss << ",";
        oss << v->at(i);
    }
    oss << "]";

    return oss.str();
}

template <class T>
T parse_xml(string xml, string tag, T (*convert)(const char*) ) throw (string) {
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
//        cout << "parsing: " << tag << " from '" << xml.substr(start, length) << "'" << endl;

        return convert( xml.substr(start, length).c_str() );
    } else {
        ostringstream oss;
        oss << "Tag '" << tag << "' was not found, find(" << start_tag << ") returned " << start << ", find(" << end_tag << ") returned " << end << ", error in file [" << __FILE__ << "] on line [" << __LINE__ << "]" << endl;
        throw oss.str();
    }
}

template <>
string parse_xml<string>(string xml, string tag, string (*convert)(const char*) ) throw (string) {
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
//        cout << "parsing: " << tag << " from '" << xml.substr(start, length) << "'" << endl;

        return convert( xml.substr(start, length).c_str() );
    } else if (length == 0) {
        return string();
    } else {
        ostringstream oss;
        oss << "Tag '" << tag << "' was not found, find(" << start_tag << ") returned " << start << ", find(" << end_tag << ") returned " << end << ", error in file [" << __FILE__ << "] on line [" << __LINE__ << "]" << endl;
        throw oss.str();
    }
}


#endif
