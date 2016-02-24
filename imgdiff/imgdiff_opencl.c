#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "imgdiff.h"

static const char* TranslateOpenCLError(cl_int errorCode)
{
	switch (errorCode)
	{
		case CL_SUCCESS:                            return "CL_SUCCESS";
		case CL_DEVICE_NOT_FOUND:                   return "CL_DEVICE_NOT_FOUND";
		case CL_DEVICE_NOT_AVAILABLE:               return "CL_DEVICE_NOT_AVAILABLE";
		case CL_COMPILER_NOT_AVAILABLE:             return "CL_COMPILER_NOT_AVAILABLE";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE:      return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES:                   return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY:                 return "CL_OUT_OF_HOST_MEMORY";
		case CL_PROFILING_INFO_NOT_AVAILABLE:       return "CL_PROFILING_INFO_NOT_AVAILABLE";
		case CL_MEM_COPY_OVERLAP:                   return "CL_MEM_COPY_OVERLAP";
		case CL_IMAGE_FORMAT_MISMATCH:              return "CL_IMAGE_FORMAT_MISMATCH";
		case CL_IMAGE_FORMAT_NOT_SUPPORTED:         return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
		case CL_BUILD_PROGRAM_FAILURE:              return "CL_BUILD_PROGRAM_FAILURE";
		case CL_MAP_FAILURE:                        return "CL_MAP_FAILURE";
		case CL_MISALIGNED_SUB_BUFFER_OFFSET:       return "CL_MISALIGNED_SUB_BUFFER_OFFSET";                              //-13
		case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:    return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_L    IST";   //-14
								      /*
									 33                                                                  case CL_COMPILE_PROGRAM_FAILURE:            re    turn "CL_COMPILE_PROGRAM_FAILURE";                               //-15
									 34                                                                  case CL_LINKER_NOT_AVAILABLE:               re    turn "CL_LINKER_NOT_AVAILABLE";                                  //-16
									 35                                                                  case CL_LINK_PROGRAM_FAILURE:               re    turn "CL_LINK_PROGRAM_FAILURE";                                  //-17
									 36                                                                  case CL_DEVICE_PARTITION_FAILED:            re    turn "CL_DEVICE_PARTITION_FAILED";                               //-18
									 37                                                                  case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:      re    turn "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";                         //-19
									 38                                                                */
		case CL_INVALID_VALUE:                      return "CL_INVALID_VALUE";
		case CL_INVALID_DEVICE_TYPE:                return "CL_INVALID_DEVICE_TYPE";
		case CL_INVALID_PLATFORM:                   return "CL_INVALID_PLATFORM";
		case CL_INVALID_DEVICE:                     return "CL_INVALID_DEVICE";
		case CL_INVALID_CONTEXT:                    return "CL_INVALID_CONTEXT";
		case CL_INVALID_QUEUE_PROPERTIES:           return "CL_INVALID_QUEUE_PROPERTIES";
		case CL_INVALID_COMMAND_QUEUE:              return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_HOST_PTR:                   return "CL_INVALID_HOST_PTR";
		case CL_INVALID_MEM_OBJECT:                 return "CL_INVALID_MEM_OBJECT";
		case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:    return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
		case CL_INVALID_IMAGE_SIZE:                 return "CL_INVALID_IMAGE_SIZE";
		case CL_INVALID_SAMPLER:                    return "CL_INVALID_SAMPLER";
		case CL_INVALID_BINARY:                     return "CL_INVALID_BINARY";
		case CL_INVALID_BUILD_OPTIONS:              return "CL_INVALID_BUILD_OPTIONS";
		case CL_INVALID_PROGRAM:                    return "CL_INVALID_PROGRAM";
		case CL_INVALID_PROGRAM_EXECUTABLE:         return "CL_INVALID_PROGRAM_EXECUTABLE";
		case CL_INVALID_KERNEL_NAME:                return "CL_INVALID_KERNEL_NAME";
		case CL_INVALID_KERNEL_DEFINITION:          return "CL_INVALID_KERNEL_DEFINITION";
		case CL_INVALID_KERNEL:                     return "CL_INVALID_KERNEL";
		case CL_INVALID_ARG_INDEX:                  return "CL_INVALID_ARG_INDEX";
		case CL_INVALID_ARG_VALUE:                  return "CL_INVALID_ARG_VALUE";
		case CL_INVALID_ARG_SIZE:                   return "CL_INVALID_ARG_SIZE";
		case CL_INVALID_KERNEL_ARGS:                return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_DIMENSION:             return "CL_INVALID_WORK_DIMENSION";
		case CL_INVALID_WORK_GROUP_SIZE:            return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE:             return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_OFFSET:              return "CL_INVALID_GLOBAL_OFFSET";
		case CL_INVALID_EVENT_WAIT_LIST:            return "CL_INVALID_EVENT_WAIT_LIST";
		case CL_INVALID_EVENT:                      return "CL_INVALID_EVENT";
		case CL_INVALID_OPERATION:                  return "CL_INVALID_OPERATION";
		case CL_INVALID_GL_OBJECT:                  return "CL_INVALID_GL_OBJECT";
		case CL_INVALID_BUFFER_SIZE:                return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_MIP_LEVEL:                  return "CL_INVALID_MIP_LEVEL";
		case CL_INVALID_GLOBAL_WORK_SIZE:           return "CL_INVALID_GLOBAL_WORK_SIZE";                               //-63
		case CL_INVALID_PROPERTY:                   return "CL_INVALID_PROPERTY";                                       //-64
							    /*
							       75                                                        case CL_INVALID_IMAGE_DESCRIPTOR:           return "CL_I    NVALID_IMAGE_DESCRIPTOR";                           //-65
							       76                                                        case CL_INVALID_COMPILER_OPTIONS:           return "CL_I    NVALID_COMPILER_OPTIONS";                           //-66
							       77                                                        case CL_INVALID_LINKER_OPTIONS:             return "CL_I    NVALID_LINKER_OPTIONS";                             //-67
							       78                                                        case CL_INVALID_DEVICE_PARTITION_COUNT:     return "CL_I    NVALID_DEVICE_PARTITION_COUNT";                     //-68
							       79                                                      */
							    //    case CL_INVALID_PIPE_SIZE:                  return "C    L_INVALID_PIPE_SIZE";                                  //-69
							    //    case CL_INVALID_DEVICE_QUEUE:               return "C    L_INVALID_DEVICE_QUEUE";                               //-70
		default:
							    return "UNKNOWN ERROR CODE";
	}
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

	cl_mem* m_image1;
	cl_mem* m_image2;
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

	for(i = 0; i<num_platforms; i++)
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

	printf("Device Count %d\n", num_devs);

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
	int LOCAL_WIDTH = 16;
	int LOCAL_HEIGHT = 16;
	int WORK_WIDTH = ceil((double)width / LOCAL_WIDTH)*LOCAL_WIDTH;
	int WORK_HEIGHT = ceil((double)height/LOCAL_HEIGHT) * LOCAL_HEIGHT / 4;
	int WORK_AMOUNT = width * height / 4;
	int WORK_GROUP_COUNT = WORK_WIDTH * WORK_HEIGHT / (LOCAL_WIDTH * LOCAL_HEIGHT);
	printf("WORK_WIDTH %d\tWORK_HEIGHT %d\t WORK_AMOUNT %d\t WORK_GROUP_COUNT %d\n", 
		WORK_WIDTH, WORK_HEIGHT, WORK_AMOUNT, WORK_GROUP_COUNT);
	
	m_image1 = (cl_mem*)malloc(sizeof(cl_mem)* num_devs);
	m_image2 = (cl_mem*)malloc(sizeof(cl_mem)* num_devs);
	m_result = (cl_mem*)malloc(sizeof(cl_mem)* num_devs);


	for(i=0; i<num_devs; i++)
	{
		m_image1[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned char) * WORK_AMOUNT, NULL, NULL);
		m_image2[i] = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned char) * WORK_AMOUNT, NULL, NULL);
		
		m_result[i] = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(double) * WORK_AMOUNT, NULL, NULL);
	}

	printf("Create Buffer Success\n");

	for(i=0; i<num_devs; i++)
	{		
		clSetKernelArg(kernels[i], 0, sizeof(cl_mem), (void*)&m_image1[i]);
		clSetKernelArg(kernels[i], 1, sizeof(cl_mem), (void*)&m_image2[i]);
		clSetKernelArg(kernels[i], 2, sizeof(cl_mem), (void*)&m_result[i]);
	}

	printf("Set Kernel Arguments\n");
	
	ev_kernels  = (cl_event*)malloc(sizeof(cl_event)*num_devs);
	
	
	
	int row, col;
	
	row = 0;
	col = 1;
	//for(row = 0; row < N; row++)
	//{
		diff_matrix[row*N + row] = 0;
		for(i=0; i<num_devs; i++)
		{

			clEnqueueWriteBuffer(cmd_queues[i], m_image1[i], CL_TRUE, 0, 
				WORK_AMOUNT*sizeof(unsigned char)*3, (void*)(images + 
				((row * width*height) + (WORK_AMOUNT * i))), 0, NULL, NULL);

		}
		//for(col=row+1; col< N; col++)
		//{


			size_t gws[2] = { WORK_WIDTH, WORK_HEIGHT};

			size_t lws[2] = { 16, 16 };


			for(i=0; i<num_devs; i++)
			{

				clEnqueueWriteBuffer(cmd_queues[i], m_image2[i], CL_TRUE, 0, 
					WORK_AMOUNT*sizeof(unsigned char)*3, (void*)(images + 
					((col * width*height) + (WORK_AMOUNT * i))), 0, NULL, NULL);


			}

			//printf("Write Buffer Success\n");

			for( i=0; i < num_devs; i++ )
			{

				err = clEnqueueNDRangeKernel(cmd_queues[i], kernels[i], 2, NULL, gws, lws, 0, NULL, NULL);
				if(err != CL_SUCCESS)
				{
					printf("Error: clEnqueueNDRangeKernel %d error\n", i);
					printf("%s\n", TranslateOpenCLError(err));
					return 0;
				}
			}

			
			for( i =0; i < num_devs; i++ )
			{
				err = clEnqueueReadBuffer( cmd_queues[i], m_result[i], CL_TRUE, 0, sizeof(double), diff_matrix + i, 0, NULL, NULL); 
				if(err != CL_SUCCESS)
				{
					printf("Error: clEnqueueReadBuffer%d error\n", i);
					return 0;
				}
			}

			//printf("%lf %lf %lf %lf\n", diff_matrix[0], diff_matrix[1], diff_matrix[2], diff_matrix[3]);
		//}
	//}
	/*
	for( i =0; i < num_devs; i++)
	{
		printf("test%d\n", i);
		clReleaseMemObject( m_image1[i] );	
		clReleaseMemObject( m_image2[i] );
		clReleaseMemObject( m_result[i] );
		clReleaseKernel(kernels[i]);
		clReleaseCommandQueue(cmd_queues[i]);
		clReleaseEvent(ev_kernels[i]);
	}
	
	clReleaseProgram(program);
	clReleaseContext(context);
	free(platform);
	free(m_image1);
	free(m_image2);
	free(m_result);
	free(cmd_queues);
	free(kernels);
	free(devs);
	free(ev_kernels);
	*/
	printf("OpenCL End\n");

}
