
__kernel void cl_shift_left(__global uint *complete, __global uint *set, __global uint *set_size, __global uint *block_size) {

    /*TODO
    implement pivot on src array.
    implement bit shifting
    Consider other functions for optimization
    */

    //Size of each index in the sums array (128 bits)
    //This data type is 128 bits long. Current max 64 choose 34 (1674).
    //15 * 128 = 1792 bits.
    uint sums[53];
    uint new_sums[53];
    uint ELEMENT_SIZE = sizeof(uint) * 8;
    //zero out the array
    for(int i = 0; i < 53; i++){
        sums[i]=0;
        new_sums[i]=0;
    }
    uint id = get_global_id(0);
    //make sure only the threads that need to run execute
    if(id < *block_size) {
        int start = *set_size;
        start = start * id;
        int end = start + (*set_size);
        uint max_length = 0;
        for(uint i = start; i < end; i++){
            max_length += set[i];
        }
        for(uint i = 0; i < ) {

        }
    }
}

void shift(uint *dest, uint max_length, uint *src, uint shift, uint ELEMENT_SIZE){
    uint full_element_shifts = shift / ELEMENT_SIZE;
    uint sub_shift = shift % ELEMENT_SIZE;

    uint i;
    for(i = 0; i < max_length; i++) dest[i] = 0;

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
