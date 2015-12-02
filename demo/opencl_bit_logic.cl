__kernel void cl_shift_left(__global uint *sums, __global uint *max_length,
     __global uint *set, __global uint *set_size) {
    /*TODO
    implement pivot on src array.
    implement bit shifting
    Consider other functions for optimization
    */
    sums[0] = 1;
    for(uint i = 0; i < *set_size; i++){
         for(uint j = *max_length - set[i]; j >= 0; j--){
             sums[j + set[i]] = (sums[j + set[i]]) + sums[j];
         }
     }

};
