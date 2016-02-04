#include "stdint.h"

#include <limits>
#include <climits>

#include <stdio.h>
#include <stdlib.h>

//required for va_start, etc.
#include <stdarg.h>

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

//common data for opencl
cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem memSrc, memDest, memLength, memShift;
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
        //outputs the error log when compiling the cl program
        if(err == CL_BUILD_PROGRAM_FAILURE){
            //get build log size
            size_t log_size;
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
            char *build_log = (char *)malloc(log_size);
            //get log
            clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
            vfprintf(stderr, build_log, argp);
            fprintf(stderr, "\n");
            exit(1);
        }
        //print out the error number
        vfprintf(stderr, fmt, argp);
        fprintf(stderr, "\n");
        exit(1);
    }
}

// ???? params 'max_length' and 'subset_length' are not used ????
static inline void build_cl_program(const uint32_t max_length, const uint32_t subset_length) {
    //path to opencl file
    char filename[] = "../demo/opencl_bit_logic.cl";
    fp = fopen(filename, "r");
    //read in the opencl source file
    char *src_str;
    size_t src_size;
    //printf("before string size\n");
    src_str = (char*)malloc(MAX_SOURCE_SIZE);
    //printf("after string size\n");
    src_size = fread(src_str, 1, MAX_SOURCE_SIZE, fp);
    fclose(fp);

    //Get device and platform ids
    err = clGetPlatformIDs(1, &platform_id, &num_platforms);
    check_error(err, "Failed to aquire platform id: ", err);
    clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices);
    check_error(err, "Failed to acquire device id: ", err);


    //Create context and command queue
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    check_error(err, "Unable to create context %d", err);
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    check_error(err, "Unable to create command que %d", err);

    //Create the program from the source file
    program = clCreateProgramWithSource(context, 1, (const char **)&src_str,
    (const size_t *)&src_size, &err);
    check_error(err, "Unable to create program %d", err);
    printf("after creating the program\n");
    printf("%ld\n", (long) device_id);
    //build the program
     err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
     check_error(err, "Unable to build program %d", err);
     //Create the kernel to be queued
     kernel = clCreateKernel(program, "cl_shift_left", &err);
     check_error(err, "Unable to create kernel %d", err);
     //printf("after kernel");

}

static inline void release_cl_program(){

    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);
}
/*TODO
figure out what calculations need to be done on the cpu
implement other bit logic functions
fix random seg fault (not consistant)
*/

static inline void cl_shift_left(uint32_t *dest, uint32_t *max_length, const uint32_t *src, const uint32_t *shift) {
    //Create the buffer for the object to be copied to device memory
    size_t sum_size = (size_t) (*max_length+1);
    sum_size *= sizeof(uint32_t);
    size_t set_size = (size_t) *shift;
    set_size *= sizeof(uint32_t);
    uint32_t sums[*max_length + 1];
    printf("before zeroing the array\n");
    for(int i = 0; i < *max_length; i++) {
        sums[i] = 0;
    }
    printf("after zeroing the array\n");
    printf("%ld, %d\n", sum_size, err);
    memDest = clCreateBuffer(context, CL_MEM_READ_WRITE, sum_size, NULL, &err);
    printf("after mem dest.\n");
    check_error(err, "Unable to create buffer src %d", err);
    memSrc = clCreateBuffer(context, CL_MEM_READ_WRITE, set_size, NULL, &err);
    check_error(err, "Unable to create buffer dest %d", err);
    memLength = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer length %d", err);
    memShift = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer shift %d", err);
    printf("after memory load\n");

    //Queue the buffer to be written to memory
    err = clEnqueueWriteBuffer(command_queue, memLength, CL_TRUE, 0, sizeof(uint32_t), max_length, 0, NULL, NULL);
    check_error(err, "Unable to write length %d", err);
    err = clEnqueueWriteBuffer(command_queue, memDest, CL_TRUE, 0, sum_size, sums, 0, NULL, NULL);
    check_error(err, "Unable to write dest %d", err);
    err = clEnqueueWriteBuffer(command_queue, memSrc, CL_TRUE, 0, set_size, src, 0, NULL, NULL);
    check_error(err, "Unable to write src %d", err);
    err = clEnqueueWriteBuffer(command_queue, memShift, CL_TRUE, 0, sizeof(uint32_t), shift, 0, NULL, NULL);
    check_error(err, "Unable to write shift %d", err);
    //printf("after kernel load\n");


    //Set the arguments for the kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memDest);
    check_error(err, "Unable to unable to set arg 0 %d", err);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memLength);
    check_error(err, "Unable to unable to set arg 1 %d", err);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memSrc);
    check_error(err, "Unable to unable to set arg 2 %d", err);
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&memShift);
    check_error(err, "Unable to unable to set arg 3 %d", err);
    //Queue the task to run
    err = clEnqueueTask(command_queue, kernel, 0, NULL, NULL);
    check_error(err, "Unable to execute %d", err);
    err = clFinish(command_queue);
    check_error(err, "Unable to read buffer %d", err);
    //printf("after execution\n");


    //Retrive the buffer of the output
    err = clEnqueueReadBuffer(command_queue, memDest, CL_TRUE, 0, sum_size, sums, 0, NULL, NULL);
    //check_error(err, "Unable to read buffer %d", err);
    err = clEnqueueReadBuffer(command_queue, memLength, CL_TRUE, 0, sizeof(uint32_t), (void *) max_length, 0, NULL, NULL);
    check_error(err, "Unable to read buffer %d", err);
    //printf("after memory copy\n");

    err = clFlush(command_queue);
    check_error(err, "Unable to read buffer %d", err);
	err = clFinish(command_queue);
    check_error(err, "Unable to read buffer %d", err);
    printf("here\n");

    //free the memory
	err = clReleaseMemObject(memDest);
    check_error(err, "Unable to read buffer %d", err);
	err = clReleaseMemObject(memLength);
    check_error(err, "Unable to read buffer %d", err);
	err = clReleaseMemObject(memShift);
    check_error(err, "Unable to read buffer %d", err);
	err = clReleaseMemObject(memSrc);
    check_error(err, "Unable to read buffer %d", err);
    clFlush(command_queue);
    clFinish(command_queue);
    //Need to find a better way of copying values
    //printf("After kernel release\n");
    for(int i = 0; i < *max_length; i++){
        dest[i] = sums[i];
    }
}

static inline bool cl_all_ones(const uint32_t *subset, const uint32_t length, const uint32_t min, const uint32_t max) {
    for(int i = min; i <= max; i ++){
        if(subset[i] == 0){
            return false;
        }
    }
    return true;
}
