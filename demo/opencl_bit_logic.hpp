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

cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem memSrc, memDest, memLength, memShift, memElm;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_platform_id platform_id = NULL;
cl_uint num_devices;
cl_uint num_platforms;
cl_int err;
FILE *fp;


void check_error(cl_int err, const char* fmt, ...) {
    va_list argp;
    va_start(argp, fmt);

    if (err < 0) {
        if(err == CL_BUILD_PROGRAM_FAILURE){
            //get build log size
            size_t log_size;
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
            char *build_log = (char *)malloc(log_size);
            //get log
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
            vfprintf(stderr, build_log, argp);
            fprintf(stderr, "\n");
        }
        vfprintf(stderr, fmt, argp);
        fprintf(stderr, "\n");
        exit(1);
    }
}


static inline void build_cl_program(const uint32_t max_length, const uint32_t subset_length) {
    char filename[] = "../demo/opencl_bit_logic.cl";
    fp = fopen(filename, "r");
    char *src_str;
    size_t src_size;
    src_str = (char*)malloc(MAX_SOURCE_SIZE);
    src_size = fread(src_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);
    err = clGetPlatformIDs(1, &platform_id, &num_platforms);
    if (err != CL_SUCCESS) {
        printf("Unable to get platform_id\n");
        exit(1);
    }
    if (clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices) != CL_SUCCESS) {
        printf("Unable to get device_id\n");
        exit(1);
    }

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    check_error(err, "Unable to create context %d", err);
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    check_error(err, "Unable to create command que %d", err);

    program = clCreateProgramWithSource(context, 1, (const char **)&src_str,
    (const size_t *)&src_size, &err);
    check_error(err, "Unable to create program %d", err);
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    check_error(err, "Unable to build program %d", err);
    kernel = clCreateKernel(program, "cl_shift_left", &err);
    check_error(err, "Unable to create kernel %d", err);
}

static inline void cl_shift_left(uint32_t *dest, const uint32_t *max_length,
     const uint32_t *src, const uint32_t *shift) {

    memSrc = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(dest), NULL, &err);
    check_error(err, "Unable to create buffer src %d", err);
    memDest = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(src), NULL, &err);
    check_error(err, "Unable to create buffer dest %d", err);
    memLength = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer length %d", err);
    memShift = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer shift %d", err);
    memElm = clCreateBuffer(context, CL_MEM_READ_WRITE,  ELEMENT, NULL, &err);
    check_error(err, "Unable to create buffer elm%d", err);
    err = clEnqueueWriteBuffer(command_queue, memDest, CL_TRUE, 0, sizeof(dest), (void *)dest, 0, NULL, NULL);
    check_error(err, "Unable to create buffer 1 %d", err);
    err = clEnqueueWriteBuffer(command_queue, memLength, CL_TRUE, 0, sizeof(uint32_t), (void *)max_length, 0, NULL, NULL);
    check_error(err, "Unable to create buffer 2 %d", err);
    err = clEnqueueWriteBuffer(command_queue, memSrc, CL_TRUE, 0, sizeof(src), (void *)src, 0, NULL, NULL);
    check_error(err, "Unable to create buffer 3 %d", err);
    err = clEnqueueWriteBuffer(command_queue, memShift, CL_TRUE, 0, sizeof(uint32_t), (void *)shift, 0, NULL, NULL);
    check_error(err, "Unable to create buffer 4 %d", err);
    err = clEnqueueWriteBuffer(command_queue, memElm, CL_TRUE, 0, (size_t)ELEMENT, (void *)ELEMENT, 0, NULL, NULL);
    check_error(err, "Unable to create buffer 5 %d", err);


    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memDest);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memLength);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memSrc);
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&memShift);
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&memElm);

    err = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    err = clEnqueueReadBuffer(command_queue, memDest, CL_TRUE, 0, sizeof(dest), dest, 0, NULL, NULL);

    err = clFlush(command_queue);
	err = clFinish(command_queue);

	err = clReleaseMemObject(memDest);
	err = clReleaseMemObject(memElm);
	err = clReleaseMemObject(memLength);
	err = clReleaseMemObject(memShift);
	err = clReleaseMemObject(memSrc);
}
