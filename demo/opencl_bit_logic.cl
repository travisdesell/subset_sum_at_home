__kernel void cl_shift_left(__global uint *complete, __global uint *set, __global uint *set_size, __global uint *block_size) {
    /*TODO
    implement pivot on src array.
    implement bit shifting
    Consider other functions for optimization
    */

    //1674 is the max value for 64 choose 34. This is hardcoded because __Local
    //prefix share it amoungst threads
    int sums[1674];
    for(int i = 0; i < 1674; i++){
        sums[i]=0;
    }
    int id = get_global_id(0);
    int start = id*(*set_size);
    int end = start + (*set_size);
    //make sure only the threads that need to run execute
    if(id < *block_size){
        uint max_length = 0;
        for(int i = start; i < end; i++){
            max_length += set[i];
        }

        sums[0] = 1;
        for(int i = start; i < end; i++){
            for(int j = max_length - set[i]; j >= 0; j--){
                sums[j + set[i]] = sums[j + set[i]] + sums[j];
            }
        }
        int covered = 1;
        int lower = set[end - 1];
        int upper = max_length - lower;
        for(int i = lower; i <= upper; i ++) {
            covered = covered && sums[i];
        }
        complete[id] = covered;
    }
}
