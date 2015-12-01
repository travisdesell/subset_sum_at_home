__kernel void cl_shift_left(__global uint32_t dest, __global uint32_t max_length,
     __global uint32_t src, __global uint32_t shift, __global uint32_t ELEMENT_SIZE) {
    uint32_t full_element_shifts = shift / ELEMENT_SIZE;
    uint32_t sub_shift = shift % ELEMENT_SIZE;
    uint32_t i;
    for (i = 0; i < max_length; i++) dest[i] = 0;

    if ((ELEMENT_SIZE - sub_shift) == 32) {
        for (i = 0; i < (max_length - full_element_shifts) - 1; i++) {
            dest[i] = src[i + full_element_shifts] << sub_shift;
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
};
__kernel void cl_or_equal(__global uint32_t *src, __global uint32_t *dest) {
};
__kernel void cl_or_single(__global uint32_t *src, __global uint32_t *dest) {
};
