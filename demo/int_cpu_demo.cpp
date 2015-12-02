#include "stdint.h"

#include "../common/generate_subsets.hpp"
#include "../common/n_choose_k.hpp"

#include <limits>
#include <climits>

#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>

#define MAX_SOURCE_SIZE (0x100000)

using std::numeric_limits;

void print_subset_sums_int(uint32_t *subset, uint35_t subset_size, uint_32_t max_set_value) {
    uint32_t i, j;
    uint32_t sum = 0;
    uint64_t nck;
    uint32_t total_sum = 0;
    

    printf("For subset {");
    for (i = 0; i < subset_size; i++) {
      total_sum += subset[i];
      if (i != (subset_size - 1)){
	printf("%d ", subset[i]);
      }
      else {
	printf("%d}:  ", subset[i]);
      }
    }
    
    char *present_sums = (char *)malloc(sizeof(char) * total_sum);
    
    for (i = 1; i < pow(2, subset_size); i++) {
      for (j = 0; j < subset_size; j++) {
	if (((int)i / (int)pow(2, j)) == 1) {
	  sum += src[j];
	}
      }
      present_sums[sum] = 1;
      sum = 0;
    }

    for (i = 0; i < total_sum; i++) {
      printf("%d", present_sums[i]);
    }
    printf("\n");

}
