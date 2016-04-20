#include "opencl_bit_logic.hpp"
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#define MAX_SOURCE_SIZE (0x100000)

cl_device_id device_id = NULL;
cl_context context = NULL;
cl_command_queue command_queue = NULL;
cl_mem memBlock, memComplete, memSet_Size, memBlock_Size, memSums;
cl_program program = NULL;
cl_kernel kernel = NULL;
cl_platform_id platform_id = NULL;
cl_uint num_devices;
cl_uint num_platforms;
cl_int err;
FILE *fp;

uint32_t *block_size;

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
void build_cl_program(const uint32_t subset_length) {
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

    uint32_t *max_mem_size = new uint32_t;
    clGetDeviceInfo(device_id, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(size_t), max_mem_size, NULL);

    //block_size is the total number of sets in a single execution
    block_size = new uint32_t;
    *block_size = *max_mem_size - (*max_mem_size % (subset_length * (sizeof(uint32_t))));
    *block_size /= (subset_length * sizeof(uint32_t));

    //Create context and command queue
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    check_error(err, "Unable to create context %d", err);
    command_queue = clCreateCommandQueue(context, device_id, 0, &err);
    check_error(err, "Unable to create command que %d", err);

    //Create the program from the source file
    program = clCreateProgramWithSource(context, 1, (const char **)&src_str,
    (const size_t *)&src_size, &err);
    check_error(err, "Unable to create program %d", err);
    //build the program
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    check_error(err, "Unable to build program %d", err);
    //Create the kernel to be queued
    kernel = clCreateKernel(program, "cl_shift_left", &err);
    check_error(err, "Unable to create kernel %d", err);
    //printf("after kernel");

}

void release_cl_program(){

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

void cl_shift_left(uint32_t *block, uint32_t subset_size, int *complete) {
    //Create the buffer for the object to be copied to device memory
    size_t block_mem = (size_t) ((*block_size) * subset_size * sizeof(uint32_t));
    size_t complete_size = (size_t) ((*block_size) * sizeof(int));
    uint32_t m = block[subset_size -1];
    size_t sum_size = (size_t) ((1/2) * subset_size * (2*m - subset_size + 1));
    sum_size *= (sizeof(uint32_t));
    memComplete = clCreateBuffer(context, CL_MEM_READ_WRITE, complete_size, NULL, &err);
    check_error(err, "Unable to create buffer complete_size %d", err);
    memBlock = clCreateBuffer(context, CL_MEM_READ_WRITE, block_mem, NULL, &err);
    check_error(err, "Unable to create buffer dest %d", err);
    memSet_Size = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer length %d", err);
    memBlock_Size = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(uint32_t), NULL, &err);
    check_error(err, "Unable to create buffer shift %d", err);

    //Queue the buffer to be written to memory
    //opencl perfers points for refrencing values
    uint32_t *t_set_size = new uint32_t(subset_size);
    err = clEnqueueWriteBuffer(command_queue, memBlock_Size, CL_TRUE, 0, sizeof(uint32_t), block_size, 0, NULL, NULL);
    check_error(err, "Unable to write length %d", err);
    err = clEnqueueWriteBuffer(command_queue, memComplete, CL_TRUE, 0, complete_size, complete, 0, NULL, NULL);
    check_error(err, "Unable to write dest %d", err);
    err = clEnqueueWriteBuffer(command_queue, memBlock, CL_TRUE, 0, block_mem, block, 0, NULL, NULL);
    check_error(err, "Unable to write src %d", err);
    err = clEnqueueWriteBuffer(command_queue, memSet_Size, CL_TRUE, 0, sizeof(uint32_t), t_set_size, 0, NULL, NULL);
    check_error(err, "Unable to write shift %d", err);
    //printf("after kernel load\n");


    //Set the arguments for the kernel
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&memComplete);
    check_error(err, "Unable to unable to set arg 0 %d", err);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&memBlock);
    check_error(err, "Unable to unable to set arg 1 %d", err);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&memSet_Size);
    check_error(err, "Unable to unable to set arg 2 %d", err);
    err = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&memBlock_Size);
    check_error(err, "Unable to unable to set arg 2 %d", err);
    check_error(err, "Unable to unable to set arg 3 %d", err);
    //Queue the task to run

    size_t local;
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    size_t global = ceil((double)*block_size/(double)local);
    global *= local;

    err = clEnqueueNDRangeKernel(command_queue, kernel,1, NULL, &global, &local, 0, NULL, NULL);
    check_error(err, "Unable to execute %d", err);
    //printf("after execution\n");


    //Retrive the buffer of the output
    err = clEnqueueReadBuffer(command_queue, memComplete, CL_TRUE, 0, complete_size, complete, 0, NULL, NULL);
    //check_error(err, "Unable to read buffer %d", err);
    check_error(err, "Unable to read buffer %d", err);
    //printf("after memory copy\n");

    err = clFlush(command_queue);
    check_error(err, "Unable to read buffer %d", err);
  	err = clFinish(command_queue);
    check_error(err, "Unable to read buffer %d", err);

    //free the memory
  	err = clReleaseMemObject(memComplete);
    check_error(err, "Unable to read buffer %d", err);
  	err = clReleaseMemObject(memBlock_Size);
    check_error(err, "Unable to read buffer %d", err);
  	err = clReleaseMemObject(memSet_Size);
    check_error(err, "Unable to read buffer %d", err);
  	err = clReleaseMemObject(memBlock);
    check_error(err, "Unable to read buffer %d", err);
    clFlush(command_queue);
    clFinish(command_queue);
    //Need to find a better way of copying values
    //printf("After kernel release\n");
    delete t_set_size;
}

static inline bool cl_all_ones(const uint32_t *subset, const uint32_t length, const uint32_t min, const uint32_t max) {
    for(uint32_t i = min; i <= max; i ++){
        if(subset[i] == 0){
            return false;
        }
    }
    return true;
}
