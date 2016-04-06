__kernel void cl_shift_left(__global uint *complete, __global uint *max_length, __global uint *set, __global uint *set_size) {
    /*TODO
    implement pivot on src array.
    implement bit shifting
    Consider other functions for optimization
    */
    int id = get_global_id(0);
    int start = id*(*set_size);
    int end = start + (*set_size);
    uint sums[*max_length + 1];
    sums[0] = 1;
    for(int i = start; i < *end; i++){
        for(int j = *max_length - set[i]; j >= 0; j--){
            sums[j + set[i]] = sums[j + set[i]] + sums[j];
        }
    }
    int covered = 1;
    int lower = set[end - 1];
    int upper = *max_length - lower;
    for(int i = lower; i <= upper; i ++) {
        covered = covered && sums[i];
    }
    complete[id];
}
