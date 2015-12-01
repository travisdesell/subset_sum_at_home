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
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);

    program = clCreateProgramWithSource(context, 1, (const char **)&src_str,
    (const size_t *)&src_size, &err);
}

static inline void cl_shift_left(uint32_t *dest, const uint32_t *max_length,
     const uint32_t *src, const uint32_t *shift) {

    memSrc = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(dest), NULL, &err);
    memDest = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(src), NULL, &err);
    memLength = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    memShift = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    memElm = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t) * 8, NULL, &err);
    err = clEnqueueWriteBuffer(command_queue, memDest, CL_TRUE, 0, sizeof(dest), (void *)dest, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(command_queue, memLength, CL_TRUE, 0, sizeof(uint32_t), (void *)max_length, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(command_queue, memSrc, CL_TRUE, 0, sizeof(src), (void *)src, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(command_queue, memShift, CL_TRUE, 0, sizeof(shift), (void *)shift, 0, NULL, NULL);
    err = clEnqueueWriteBuffer(command_queue, memShift, CL_TRUE, 0, sizeof(uint32_t) * 8, (void *)ELEMENT, 0, NULL, NULL);

    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "cl_shift_left", &err);

    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memDest);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memLength);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memSrc);
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&memShift);
    err = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *)&memElm);

    err = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    err = clEnqueueReadBuffer(command_queue, memDest, CL_TRUE, 0, sizeof(dest), dest, 0, NULL, NULL);

    err = clFlush(command_queue);
	err = clFinish(command_queue);
    err = clReleaseKernel(kernel);

    err = clReleaseProgram(program);
	err = clReleaseMemObject(memDest);
	err = clReleaseMemObject(memElm);
	err = clReleaseMemObject(memLength);
	err = clReleaseMemObject(memShift);
	err = clReleaseMemObject(memElm);
	err = clReleaseCommandQueue(command_queue);
	err = clReleaseContext(context);
}
