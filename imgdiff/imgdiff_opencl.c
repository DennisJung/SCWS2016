#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "imgdiff.h"


size_t ARRAY_SIZE = 100;

cl_device_id devices;

int createContext(cl_context *context, cl_device_type dev_type)
{
	cl_int err = 0;
	cl_uint numberOfPlatforms = 0;
	cl_platform_id firstPlatformID = 0;

	err = clGetPlatformIDs(1, &firstPlatformID, &numberOfPlatforms); //FIXME

	if( err != CL_SUCCESS )
	{
		printf("Error: Failed to retrieve clGetPlatformIds\n");
		return 0;
	}

	if( numberOfPlatforms <= 0 )
	{
		printf(" No OpenCL platforms found.\n");
		return 0;
	}


	cl_context_properties contextProperties [] = {CL_CONTEXT_PLATFORM, (cl_context_properties)firstPlatformID, 0};
	*context = clCreateContextFromType(contextProperties, dev_type, NULL, NULL, &err);

	if( err != CL_SUCCESS)
	{
		printf("Failed to create context\n");
		return 0;
	}


	return 1;
}


int getDeviceIDs(cl_context* context, cl_uint* num_devs, cl_device_id* devs, cl_command_queue* cmd_queues)
{
	cl_int err = 0;
	size_t deviceBufferSize = -1;
	printf("before context info\n");

	err = clGetContextInfo( *context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize );
	printf("after context info\n");
	if( err != CL_SUCCESS )
	{
		printf("Failed to get context \n");
		return 0;
	}

	if(deviceBufferSize == 0)
	{
		printf(" No OpenCL devices found.\n");
		return 0;
	}

	*num_devs = deviceBufferSize / sizeof(cl_device_id);

	printf("Devices Count : %d\n", *num_devs);

	devs = (cl_device_id*)malloc(sizeof(cl_device_id)*(*num_devs));

	err = clGetContextInfo( *context, CL_CONTEXT_DEVICES, sizeof(cl_device_id), &devices, NULL );

	if( err != CL_SUCCESS )
	{
		printf("Failed to get context\n");

		free(devs);
		return 0;
	}

	printf(" after context textinfo\n");

	return 1;

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

cl_uint CreateAndBuildProgram(cl_context* context, cl_program* program, cl_device_id* devs, cl_uint* num_devs)
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
	printf(" After readsource from file\n");
	// And now after you obtained a regular C string call clCreateProgramWithSource to create OpenCL program object.
	*program = clCreateProgramWithSource(*context, 1, (const char**)&source, &src_size, &err);
	if (CL_SUCCESS != err)
	{
		printf("Error: clCreateProgramWithSource returned %s.\n", err);
		goto Finish;
	}

	printf(" After create program\n");
	// Build the program
	// During creation a program is not built. You need to explicitly call build function.
	// Here you just use create-build sequence,
	// but there are also other possibilities when program consist of several parts,
	// some of which are libraries, and you may want to consider using clCompileProgram and clLinkProgram as
	// alternatives.
	printf("dev_num %d\n",*num_devs);

	err = clBuildProgram(*program, 1, &devices, "", NULL, NULL);

	if( err != CL_SUCCESS)
	{
		printf("Error: clBuildProgram() for source program returned \n");
	}
	
	printf(" After build program\n");
Finish:
	if (source)
	{
		free(source);
		source = NULL;
	}

	return err;
}

cl_uint CreateBufferArguments(cl_context* context, cl_mem* memObjects, int* a, int *b)
{

	cl_int err = CL_SUCCESS;

	// Create new OpenCL buffer objects
	// As these buffer are used only for read by the kernel, you are recommended to create it with flag CL_MEM_READ_ONLY.
	// Always set minimal read/write flags for buffers, it may lead to better performance because it allows runtime
	// to better organize data copying.
	// You use CL_MEM_COPY_HOST_PTR here, because the buffers should be populated with bytes at inputA and inputB.

	memObjects[0] = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int) * ARRAY_SIZE, a, &err);
	if (CL_SUCCESS != err)
	{
		printf("Error: clCreateBuffer for Lights returned %s\n", err);
		return err;
	}


	memObjects[1] = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int) * ARRAY_SIZE, b, &err);
	if (CL_SUCCESS != err)
	{
		printf("Error: clCreateBuffer for Shapes returned %s\n", err);
		return err;
	}

	memObjects[2] = clCreateBuffer(*context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(int) * ARRAY_SIZE, NULL, &err);
	if (CL_SUCCESS != err)
	{
		printf("Error: clCreateBuffer for Shapes returned %s\n", err);
		return err;
	}

	return CL_SUCCESS;
}


void imgdiff(size_t N, size_t width, size_t height, double* diff_matrix, unsigned char* images) 
{

	//// we need to fill in ////
	cl_platform_id *platform;
	cl_device_type dev_type = CL_DEVICE_TYPE_GPU;
	cl_device_id *devs;
	cl_context context;
	cl_command_queue *cmd_queues;
	cl_program program;
	cl_kernel *kernels;
	cl_uint num_platforms;
	cl_uint num_devs;

	int err = CL_SUCCESS;

	int i;

	// modify version
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	if(err != CL_SUCCESS)
	{
		printf("Error: platform error\n");
		return 0;
	}

	if(num_platforms == 0)
	{
		printf("Error: platform no count\n");
		return 0;
	}

	printf("Number of platforms: %u\n", num_platforms);

	platform = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
	err = clGetPlatformIDs(num_platforms, platform, NULL);
	if(err != CL_SUCCESS)
	{
		printf("Error: clGetPlatformIDs error\n");
		return 0;
	}

	for(i = 0; i<num_devs; i++)
	{
		err = clGetDeviceIDs(platform[i], dev_type, 0, NULL, &num_devs);
		if(err != CL_SUCCESS)
		{
			printf("Error: clGetDevice\n");
			return 0;
		}
		if(num_devs >= 1)
		{
			devs = (cl_device_id*)malloc(sizeof(cl_device_id) * num_devs);

			clGetDeviceIDs(platform[i], dev_type, num_devs, devs, NULL);
			break;
		}
	}

	printf("Device Get\n");

	context = clCreateContext(NULL, num_devs, devs, NULL, NULL, &err);
	if(err != CL_SUCCESS)
	{
		printf("Error: clCreateContext error\n");
		return 0;
	}

	printf("Context Create\n");

	char* source = NULL;
	size_t src_size = 0;
	err = ReadSourceFromFile("./hello.cl", &source, &src_size);
	if (CL_SUCCESS != err)
	{
		printf("Error: ReadSourceFromFile returned %s.\n", err);
		free(source);
		return 0;
	}
	
	program = clCreateProgramWithSource(context, 1, (const char**)&source, &src_size, &err);
	if(err != CL_SUCCESS)
	{
		printf("Error: clCreateProgram error\n");
		return 0;
	}

	free(source);
	printf("Create Program Success\n");

	err = clBuildProgram(program, num_devs, devs, "", NULL, NULL);
	if(err != CL_SUCCESS)
	{
		printf("Error: clBuildProgram\n");
		return 0;
	}

	printf("Build Program Success\n");
}
