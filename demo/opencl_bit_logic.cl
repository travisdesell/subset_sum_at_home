void shift_left(uint *dest, uint max_length, uint *src, uint shift, uint ELEMENT_SIZE){
    uint full_element_shifts = shift / ELEMENT_SIZE;
    uint sub_shift = shift % ELEMENT_SIZE;

    uint i;
    for(i = 0; i < max_length; i++){
        dest[i] = 0;
    }

    if((ELEMENT_SIZE - sub_shift) == 32) {
        for(i = 0; i < (max_length - full_element_shifts) - 1; i++) {
            dest[i] = src[i + full_element_shifts] << sub_shift | src[i + full_element_shifts + 1] >> (ELEMENT_SIZE - sub_shift);
        }
    } else {
        for (i = 0; i < (max_length - full_element_shifts) - 1; i++) {
            dest[i] = src[i + full_element_shifts] << sub_shift | src[i + full_element_shifts + 1] >> (ELEMENT_SIZE - sub_shift);
        }
    }

    dest[i] = src[max_length - 1] << sub_shift;
    i++;

    for (; i < max_length; i++) {
        dest[i] = 0;
    }

}

void or_equal(uint *dest, uint len, uint *src) {
    for(uint i = 0; i < len; i++) {
        dest[i] |= src[i];
    }
}

void or_single(uint *dest, uint len, uint number, uint ELEMENT_SIZE) {
    uint pos = number / ELEMENT_SIZE;
    uint tmp = number % ELEMENT_SIZE;

    dest[len - pos - 1] |= 1 << tmp;

}

int all_ones(uint *set, uint len, uint in_min, uint in_max, uint ELEMENT_SIZE, uint start){
    uint UINT32_MAX = 0xffffffff;
    uint min_pos = start + in_min / ELEMENT_SIZE;
    uint min_tmp = start + in_min % ELEMENT_SIZE;
    uint max_pos = start + in_max / ELEMENT_SIZE;
    uint max_tmp = start + in_max % ELEMENT_SIZE;
    if (min_pos == max_pos) {
        uint against = (UINT32_MAX >> (ELEMENT_SIZE - max_tmp)) & (UINT32_MAX << (min_tmp - 1));
        return against == (against & set[len - max_pos - 1]);
    } else {
        uint against =  UINT32_MAX << (min_tmp - 1);
        if (against != (against & set[len - min_pos - 1])) {
            return 0;
        }
//        fprintf(output_target, "min success\n");

        for (uint i = (len - min_pos - 2); i > (len - max_pos); i--) {
            if (UINT32_MAX != (UINT32_MAX & set[i])) {
                return 0;
            }
        }
//        fprintf(output_target, "mid success\n");

        if (max_tmp > 0) {
            against = UINT32_MAX >> (ELEMENT_SIZE - max_tmp);
            if (against != (against & set[len - max_pos - 1])) {
                return 0;
            }
        }
//        fprintf(output_target, "max success\n");

    }

    return 1;
}

__kernel void cl_kernel(__global uint *complete, __global uint *set, __global uint *set_size, __global uint *block_size) {
    //Size of each index in the sums array (128 bits)
    //This data type is 128 bits long. Current max 64 choose 34 (1674).
    //15 * 128 = 1792 bits.
    uint sums[53];
    uint new_sums[53];
    uint ELEMENT_SIZE = sizeof(uint) * 8;
    uint M = set[*set_size-1];
    uint max_length = (1/2) * (*set_size) *(2*M - *set_size + 1);
    //zero out the array
    for(int i = 0; i < 53; i++){
        sums[i]=0;
        new_sums[i]=0;
    }
    uint id = get_global_id(0);
    //make sure only the threads that need to run execute
    if(id < *block_size) {
        uint start = id*(*set_size);
        uint end = start + *set_size;
        uint max_sum = 0;
        for(uint i = start; i < end; i++){
            max_sum += set[i];
        }
        uint current;
        for(uint i = start; i < end; i++ ) {
            current = set[i];
            shift_left(new_sums, max_length, sums, current, ELEMENT_SIZE);
            or_equal(sums, max_length, new_sums);
            or_single(sums, max_length, current-1, ELEMENT_SIZE);

        }
        complete[id] = all_ones(sums, max_length, M, max_sum - M, ELEMENT_SIZE, start);
    }
}
