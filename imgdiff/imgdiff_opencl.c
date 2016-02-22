#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "imgdiff.h"

cl_platform_id *platform;
cl_device_type dev_type = CL_DEVICE_TYPE_GPU;
cl_device_id *devs = NULL;
cl_context context;
cl_command_queue *cmd_queues;
cl_program program;
cl_kernel *kernels;
cl_mem *memA;
cl_mem *memB;
cl_mem *memC;
cl_uint num_platforms;
cl_uint num_devs = 0;
cl_int err;





bool createContext()
{
	cl_int err = 0;
	cl_uint numberOfPlatforms = 0;
	cl_platform_id firstPlatformID = 0;
	
	err = clGetPlatformIDs(1, &firstPlatformID, &numberOfPlatforms);

	if( err != CL_SUCCESS )
	{
		printf("Error: Failed to retrieve clGetPlatformIds\n");
		return false;
	}

	if( numberOfPlatforms <= 0 )
	{
		printf(" No OpenCL platforms found.\n");
		return false;
	}

	
	cl_context_properties contextProperties [] = {CL_CONTEXT_PLATFORM, (cl_context_properties)firstPlatformID, 0};
	context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
	
	if( err != CL_SUCCESS)
	{
		printf("Failed to create context\n");
		return false;
	}

	
	return true;
}


bool getDeviceIDs()
{
	cl_int err = 0;
	size_t deviceBufferSize = -1;

	err = clGetContextInfo( context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize );
	if( err != CL_SUCCESS )
	{
		printf("Failed to get context \n");
		return false;
	}

	if(deviceBufferSize == 0)
	{
		printf(" No OpenCL devices found.\n");
		return false;
	}

	num_devs = deviceBufferSize / sizeof(cl_device_id);
	
	printf("Devices Count : %d\n", num_devs);

	devs = (cl_device_id*)malloc(sizeof(cl_device_id)*num_devs);

	err = clGetContextInfo( context, CL_CONTEXT_DEVICES, deviceBufferSize, devs, NULL );

	if( err != CL_SUCCESS )
	{
		printf("Failed to get context\n");

		free(devs);
		return false;
	}

	return true;

}

int ReadSourceFromFile(const char* fileName, char** source, size_t* sourceSize)
{
    int errorCode = CL_SUCCESS;

    FILE* fp = NULL;
    fp = fopen(fileName, "rb");
    if (fp == NULL)
    {
        printf("Error: Couldn't find program source file");
        errorCode = CL_INVALID_VALUE;
    }
    else {
        fseek(fp, 0, SEEK_END);
        *sourceSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *source = (char*)malloc(sizeof(char)*(*sourceSize)); 
        if (*source == NULL)
        {
            printf("Error: Couldn't allocate for program source");
            errorCode = CL_OUT_OF_HOST_MEMORY;
        }
        else {
            fread(*source, 1, *sourceSize, fp);
        }
    }
    return errorCode;
}

cl_uint CreateAndBuildProgram()
{
    cl_int err = CL_SUCCESS;

    // Upload the OpenCL C source code from the input file to source
    // The size of the C program is returned in sourceSize
    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("hello.cl", &source, &src_size);
    if (CL_SUCCESS != err)
    {
        printf("Error: ReadSourceFromFile returned %s.\n", err);
        goto Finish;
    }

    // And now after you obtained a regular C string call clCreateProgramWithSource to create OpenCL program object.
    program = clCreateProgramWithSource(context, 1, (const char**)&source, &src_size, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateProgramWithSource returned %s.\n", err);
        goto Finish;
    }

    // Build the program
    // During creation a program is not built. You need to explicitly call build function.
    // Here you just use create-build sequence,
    // but there are also other possibilities when program consist of several parts,
    // some of which are libraries, and you may want to consider using clCompileProgram and clLinkProgram as
    // alternatives.
    err = clBuildProgram(program, 2, devs, "", NULL, NULL);
    if (CL_SUCCESS != err)
    {
        printf("Error: clBuildProgram() for source program returned %s.\n", err);
    }

Finish:
    if (source)
    {
        free(source);
        source = NULL;
    }

    return err;
}




void imgdiff(size_t N, size_t width, size_t height, double* diff_matrix, unsigned char* images) 
{

	//// we need to fill in ////


}
