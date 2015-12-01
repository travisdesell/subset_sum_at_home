#include "stdint.h"

#include <limits>
#include <climits>

#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#define MAX_SOURCE_SIZE (0x100000)


const uint32_t ELEMENT = sizeof(uint32_t) * 8;
extern unsigned long int max_sums_length;
extern uint32_t *sums;
extern uint32_t *new_sums;

using std::numeric_limits;

static inline void cl_shift_left(uint32_t *dest, const uint32_t max_length,
    const uint32_t subset_length, const uint32_t *src, const uint32_t shift) {
    cl_device_id device_id = NULL;
    cl_context context = NULL;
    cl_command_queue command_queue = NULL;
    cl_mem memSrc, memDest;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_platform_id platform_id = NULL;
    cl_uint num_devices;
    cl_uint num_platforms;
    cl_int err;

    FILE *fp;
    char filename[] = "./opencl_bit_logic.cl";
    fp = fopen(filename, "r");
    char *src_str;
    size_t src_size;
    src_str = (char*)malloc(MAX_SOURCE_SIZE);
    src_size = fread(src_str, 1, MAX_SOURCE_SIZE, fp);


    if (clGetPlatformIDs(1, &platform_id, &num_platforms)!= CL_SUCCESS) {
        printf("Unable to get platform_id\n");
        exit(1);
    }
    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices) != CL_SUCCESS) {
        printf("Unable to get device_id\n");
        exit(1);
    }

    memSrc = clCreateBuffer(context, CL_MEM_READ_WRITE,subset_length * ELEMENT, NULL, &err);
    memDest = clCreateBuffer(context, CL_MEM_READ_WRITE, max_sums_length * ELEMENT, NULL, &err);
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);

    program = clCreateProgramWithSource(context, 1, (const char **)&src_str,
    (const size_t *)&src_size, &err);

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "opencl_bit_logic", &err);

    for(int i = 0; i < max_length; i++) {
        dest[i] = 1;
    }



}
