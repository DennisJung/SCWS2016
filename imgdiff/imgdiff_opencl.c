#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "imgdiff.h"


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

	cl_mem* m_image;
	cl_mem* m_result;

	cl_event* ev_kernels;
	
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
	err = ReadSourceFromFile("./imgdiff_cal.cl", &source, &src_size);
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

	kernels = (cl_kernel*)malloc(sizeof(cl_kernel)*num_devs);
	for(i = 0; i<num_devs; i++)
	{
		kernels[i] = clCreateKernel(program, "imgdiff_cal", NULL);
	}


	printf("Create Kernel Success\n");

	cmd_queues = (cl_command_queue*)malloc(sizeof(cl_command_queue)*num_devs);
	for(i=0; i<num_devs; i++)
	{
		cmd_queues[i] = clCreateCommandQueue(context, devs[i], 0, &err);
		if(err != CL_SUCCESS)
		{
			printf("Error: clCreateCommandQueue error\n");
			return 0;
		}

	}

	printf("Create commandQueue Success\n");

	int WORK_WIDTH = width;
	int WORK_HEIGHT = height;
	int WORK_AMOUNT = WORK_WIDTH * WORK_HEIGHT;
	printf("WORK_WIDTH %d\tWORK_HEIGHT %d\t WORK_AMOUNT %d\n", WORK_WIDTH, WORK_HEIGHT, WORK_AMOUNT);
	
	m_image = (cl_mem*)malloc(sizeof(cl_mem)* num_devs);
	m_result = (cl_mem*)malloc(sizeof(cl_mem)* num_devs);


	for(i=0; i<num_devs; i++)
	{
		m_image[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, WORK_AMOUNT * sizeof(unsigned char)*3, NULL, NULL);
		m_result[i] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double), NULL, NULL);
	}

	printf("Create Buffer Success\n");

	for(i=0; i<num_devs; i++)
	{		
		clSetKernelArg(kernels[i], 0, sizeof(cl_mem), (void*)&m_image[i]);
		clSetKernelArg(kernels[i], 1, sizeof(cl_mem), (void*)&m_result[i]);
	}

	printf("Set Kernel Arguments\n");
	/*
	int row, col;
	for(row = 0; row < N; row++)
	{
		for(col=0; col<)
	*/	
	size_t gws[2] = { WORK_WIDTH, WORK_HEIGHT };
	
	size_t lws[2] = { 256, 256 };

	ev_kernels  = (cl_event*)malloc(sizeof(cl_event)*num_devs);
	
	for(i=0; i<num_devs; i++)
	{

		clEnqueueWriteBuffer(cmd_queues[i], m_image[i], CL_FALSE, 0, WORK_AMOUNT*sizeof(unsigned char)*3, (void*)(images + i * WORK_AMOUNT), 0, NULL, NULL);

	}

	printf("Write Buffer Success\n");

	for( i=0; i < num_devs; i++ )
	{

		err = clEnqueueNDRangeKernel(cmd_queues[i], kernels[i], 2, NULL, gws, lws, 0, NULL, NULL);
	}


	diff_matrix[0] = 0;	
	for( i =0; i < num_devs; i++ )
	{
		clEnqueueReadBuffer( cmd_queues[i], m_result[i], CL_TRUE, 0, sizeof(double), diff_matrix + i, 0, NULL, NULL); 
	}
	
	printf("%lf %lf %lf %lf\n", diff_matrix[0], diff_matrix[1], diff_matrix[2], diff_matrix[3]);
	diff_matrix[0] = 2;

	diff_matrix[1] = 2;
	
	diff_matrix[2] = 2;
	for( i =0; i < num_devs; i++)
	{
		clReleaseMemObject( m_image[i] );
		clReleaseMemObject( m_result[i] );
		clReleaseKernel(kernels[i]);
		clReleaseCommandQueue(cmd_queues[i]);
		clReleaseEvent(ev_kernels[i]);
	}
	
	clReleaseProgram(program);
	clReleaseContext(context);
	free(platform);
	free(m_image);
	free(m_result);
	free(cmd_queues);
	free(kernels);
	free(devs);
	free(ev_kernels);
	printf("OpenCL End\n");

}
