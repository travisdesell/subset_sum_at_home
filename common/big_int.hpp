#ifndef SSS_BIG_INT_H
#define SSS_BIG_INT_H

#include <iostream>
#include <string>
#include "../undvc_common/parse_xml.hxx"

#include <boost/algorithm/string.hpp>

using std::cout;
using std::endl;
using std::string;

class BigInt {
    private:
        uint32_t size;
        uint32_t *data;


    public:
        /**
         *  Constructors
         */
        BigInt() {
            size = 1;
            data = new uint32_t[1];
            data[0] = 0;
        }

        BigInt(uint32_t digits) {
        }

        BigInt(uint64_t value) {
        }

        BigInt(string value) {

        }

        BigInt(const char* value) {
        }

        /**
         *  Copy Constructor
         */

        BigInt(const BigInt &other) {
            cout << "copying big int of size: " << other.size << endl;

            size = other.size;
            data = new uint32_t[size];
            for (int i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
        }

        /**
         *  Destructor
         */
        ~BigInt() {
            cout << "destroying big int of size: " << size << endl;
            delete data;
        }


        /**
         *  Methods involving output
         */
        string to_decimal_string() {
            return "";
        }

        string to_binary_string() {
            return "";
        }


        /**
         *  Overloaded operators
         */

        /**
         *  The not equal operator. Check and see if the data is not the same.  The size of data
         *  may be different, but as long as the BigInt with a larger size doesn't have all 0s 
         *  in its additional data then they are different number.  Otherwise they're different
         *  if any of their data elements are different.
         */
        bool operator!=(const BigInt &other) {
            if (other.size < size) {
                int i = 0;
                for (; i < other.size; i++) {
                    if (data[i] != other.data[i]) return true;
                }

                for (; i < size; i++) {
                    if (data[i] != 0) return true;
                }
            } else {
                int i = 0;
                for (; i < size; i++) {
                    if (data[i] != other.data[i]) return true;
                }

                for (; i < other.size; i++) {
                    if (data[i] != 0) return true;
                }
            }
            return false;
        }

        /**
         *  The equal operator. Check and see if the data is the same.  The size of data
         *  may be different, but as long as the BigInt with a larger size has all 0s in its
         *  additional data then they are the same number.
         */
        bool operator==(const BigInt &other) {
            if (other.size < size) {
                int i = 0;
                for (; i < other.size; i++) {
                    if (data[i] != other.data[i]) return false;
                }

                for (; i < size; i++) {
                    if (data[i] != 0) return false;
                }
            } else {
                int i = 0;
                for (; i < size; i++) {
                    if (data[i] != other.data[i]) return false;
                }

                for (; i < other.size; i++) {
                    if (data[i] != 0) return false;
                }
            }
            return true;
        }


        /**
         *  prefix:  ++BigInt
         *
         */
        BigInt& operator++() {
            int current = 0;

            while (current < size && data[current] == numeric_limits<uint32_t>::max()) {
                data[current] = 0;
                current++;
            }

            /**
             * If current == size, then the number was 11111111111... for all its data elements.
             * We need to expand the bigint by adding another data element, and setting to one, so
             * the data will be:
             * 1 00000... 00000... 00000...
             */
            if (current == size) {
                uint32_t *temp_data = new uint32_t[size + 1];
                for (int i = 0; i < size; i++) temp_data[i] = data[i];
                temp_data[size] = 1;
                size++;

                delete data;
                data = temp_data;
            } else {
                //Just increment the current data because it was not going to overflow
                data[current]++;
            }
        }

        /**
         *  postfix: BigInt++
         *  This returns the value of the variable, then increments, so we
         *  need to make a copy, increment this and then return the copy.
         */
        BigInt operator++(int) { 
            BigInt tmp(*this);
            operator++();
            return tmp;
        }
};

void string_to_vector(string s, vector<BigInt> &v) {
    v.clear();

    vector<std::string> split_string;
    boost::split(split_string, s, boost::is_any_of("[], "));

    for (uint32_t i = 0; i < split_string.size(); i++) {
        if (split_string[i].size() > 0) {
            v.push_back( BigInt(split_string[i]) );
        }   
    }   
}

template<>
void parse_xml_vector<BigInt>(string xml, const char tag[], vector<BigInt> &result) throw (string) {
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
        string_to_vector(xml.substr(start, length), result);
        return;
    } else if (length == 0) {
        return;
    } else {
        ostringstream oss;
        oss << "Tag '" << tag << "' was not found, find(" << start_tag << ") returned " << start << ", find(" << end_tag << ") returned " << end << ", error in file [" << __FILE__ << "] on line [" << __LINE__ << "]" << endl;
        throw oss.str();
    }   
}

#endif
