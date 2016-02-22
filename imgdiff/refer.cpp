#include <pthread.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <fstream>
#include <sstream>

#include "raycl.h"


RayCL* raycl;

ocl_args_d_t ocl;

SphereSet masterSet;
Camera cam;

cl_command_queue commandQueue1;
cl_command_queue commandQueue2;

int DeviceCount;

unsigned int WorkAmount = WORK_AMOUNT;
unsigned int LocalSize = LOCAL_SIZE;
unsigned int Width = WIDTH;
unsigned int Height = HEIGHT;
unsigned int SampleCount = kNumSampleCount;

unsigned int ImagePixel[WIDTH * HEIGHT];

typedef struct PASSING_OCL
{
    ocl_args_d_t ocl;
    int device_index;
    int workNDRangeStart;
    int workNDRangeEnd;
    int localSize;

}PASSING_OCL;

bool createContext()
{
    cl_int err = 0;
    cl_uint numberOfPlatforms = 0;
    cl_platform_id firstPlatformID = 0;

    /* Retrieve a single platform ID. */
    err = clGetPlatformIDs(1, &firstPlatformID, &numberOfPlatforms);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed clGetPlatformIds\n");
        return false;
    }

    if (numberOfPlatforms <= 0)
    {
        printf("No OpenCL platforms found.\n");
        return false;
    }

    /* Get a context with a GPU device from the platform found above. */
    cl_context_properties contextProperties [] = {CL_CONTEXT_PLATFORM, (cl_context_properties)firstPlatformID, 0};
    ocl.context = clCreateContextFromType(contextProperties, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Creating an OpenCL context failed.\n");
        return false;
    }

    return true;

}

bool getDeviceIDs()
{
    cl_int err = 0;
    size_t deviceBufferSize = -1;

    /* Retrieve the size of the buffer needed to contain information about the devices in this OpenCL context. */
    err = clGetContextInfo(ocl.context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBufferSize);
    if (err != CL_SUCCESS)
    {
        printf("Failed to get OpenCL context information.\n");
        return false;
    }

    if(deviceBufferSize == 0)
    {
        printf("No OpenCL devices found.\n");
        return false;
    }

    /* Retrieve the list of devices available in this context. */
    DeviceCount = deviceBufferSize / sizeof(cl_device_id);;

    printf("Device Count %d\n", DeviceCount);

    ocl.device = new cl_device_id[DeviceCount];
    err = clGetContextInfo(ocl.context, CL_CONTEXT_DEVICES, deviceBufferSize, ocl.device, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Failed to get the OpenCL context information.\n");
        delete [] ocl.device;
        return false;
    }

    return true;
}

void generateArgument()
{
    masterSet.m_rectLight = new RectangleLight[2];

    masterSet.m_plane = new Plane[1];
    masterSet.PlaneCount = 1;
    Point tmp_p = { 0.0f, -2.0f, 0.0f };
    Vector tmp_v1 = { 0.0f, 1.0f, 0.0f };
    Vector tmp_v2 = { 0.0f, 0.0f, 0.0f };
    Color tmp_c = { 1.0f, 1.0f, 1.0f };

    float tmp_power = 0.0;
    int i = 0;

    // Plane Set
    masterSet.m_plane[0] = { tmp_p, tmp_v1, tmp_c };

    // Rectangle Light Set
    masterSet.LightCount = 2;
    tmp_p = { 2.5f, 2.0f, -2.5f };
    tmp_v1 = { 5.0f, 0.0f, 0.0f };
    tmp_v2 = { 0.0f, 0.0f, 5.0f };
    tmp_c = { 1.0f, 0.5f, 1.0f };
    tmp_power = 3.0f;
    masterSet.m_rectLight[0] = { tmp_p, tmp_v1, tmp_v2, tmp_c, tmp_power };

    tmp_p = { -2.0f, -1.0f, -2.0f };
    tmp_v1 = { 4.0f, 0.0f, 0.0f };
    tmp_v2 = { 0.0f, 0.0f, 4.0f };
    tmp_c = { 1.0f, 1.0f, 0.5f };
    tmp_power = 0.75f;
    masterSet.m_rectLight[1] = { tmp_p, tmp_v1, tmp_v2, tmp_c, tmp_power };

    Point tmp_p1 = { 0.0f, 5.0f, 15.0f };
    Point tmp_p2 = { 0.0f, 0.0f, 0.0f };
    Point tmp_p3 = { 0.0f, 1.0f, 0.0f };

    cam = {45.0f, tmp_p1, tmp_p2, tmp_p3};

}

void* RenderDisplay(void* arguments)
{
    int err = 0;

    struct PASSING_OCL *ocl_info = (struct PASSING_OCL *)arguments;
    cl_command_queue commandQueue;
    cl_uint RandomSeeds[WorkAmount * 2];
    cl_mem Pixels;
    cl_mem Seeds;
    unsigned int PixelData[WorkAmount];

    for(int i = 0; i< WorkAmount*2; i++)
    {
        RandomSeeds[i] = rand();
        if (RandomSeeds[i] < 2)
            RandomSeeds[i] = 2;
    }
    commandQueue = clCreateCommandQueue(ocl_info->ocl.context, ocl_info->ocl.device[ocl_info->device_index], CL_QUEUE_PROFILING_ENABLE, &err);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed clCreateCommandQueue\n");
        return NULL;
    }

    Pixels = clCreateBuffer(ocl_info->ocl.context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_uint) * WorkAmount, NULL, &err);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed pixels create buffer\n");
        return NULL;
    }

    Seeds = clCreateBuffer(ocl_info->ocl.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * WorkAmount*2, RandomSeeds, &err);
    if(err != CL_SUCCESS)
    {
        printf("Error: Failed Seeds create buffer\n");
        return NULL;
    }

    for(int i = ocl_info->workNDRangeStart; i<ocl_info->workNDRangeEnd; i++)
    {
        size_t globalWorkSize[1] = {WorkAmount};
        size_t localWorkSize[1] = {ocl_info->localSize};

        err = clSetKernelArg(ocl_info->ocl.kernel, 8, sizeof(cl_uint), (void*)&Seeds);
        if(CL_SUCCESS != err)
        {
            printf("Error: Failed to set argument Seeds\n");
            return NULL;
        }

        err = clSetKernelArg(ocl_info->ocl.kernel, 9, sizeof(cl_mem), (void*)&Pixels);
        if(CL_SUCCESS != err)
        {
            printf("Error: Failed to set argument Pixels\n");
            return NULL;
        }

        err = clSetKernelArg(ocl_info->ocl.kernel, 10, sizeof(cl_uint), (void*)&i);
        if(CL_SUCCESS != err)
        {
            printf("Error: Failed to set argument i\n");
            return NULL;
        }

        err = clEnqueueNDRangeKernel(commandQueue, ocl_info->ocl.kernel, 1, NULL, globalWorkSize, localWorkSize, NULL, 0, NULL);
        if(CL_SUCCESS != err)
        {
            printf("Error: Failed to run kernel to run\n");
            return NULL;
        }
        clEnqueueReadBuffer(commandQueue, Pixels, CL_TRUE, 0, WorkAmount * sizeof(cl_uint), PixelData, 0, NULL, NULL);
        for(int j=0; j<WorkAmount; j++)
        {
            ImagePixel[j + i*WorkAmount] = PixelData[j];
        }

    }

    if(clFinish(commandQueue) != CL_SUCCESS)
    {
        printf("Error: Failed to Finish");
        return NULL;
    }
    return NULL;
}
cl_uint CreateBufferArguments()
{

    cl_int err = CL_SUCCESS;

    // Create new OpenCL buffer objects
    // As these buffer are used only for read by the kernel, you are recommended to create it with flag CL_MEM_READ_ONLY.
    // Always set minimal read/write flags for buffers, it may lead to better performance because it allows runtime
    // to better organize data copying.
    // You use CL_MEM_COPY_HOST_PTR here, because the buffers should be populated with bytes at inputA and inputB.

    ocl.Lights = clCreateBuffer(ocl.context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(RectangleLight) * masterSet.LightCount, masterSet.m_rectLight, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateBuffer for Lights returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    ocl.LightCount = masterSet.LightCount;

    ocl.Shapes = clCreateBuffer(ocl.context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(Plane) * masterSet.PlaneCount, masterSet.m_plane, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateBuffer for Shapes returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    ocl.ShapeCount = masterSet.PlaneCount;

    ocl.sampleCount = SampleCount;

    ocl.cam = clCreateBuffer(ocl.context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(Camera), &cam, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateBuffer for cam returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    ocl.width = Width;
    ocl.height = Height;
    /*
    ocl.Pixels = clCreateBuffer(ocl.context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(cl_uint) * WorkAmount, NULL, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateBuffer for Pixels returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    ocl.Seeds = clCreateBuffer(ocl.context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_uint) * WorkAmount * 2, Seeds, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateBuffer for Seeds returned %s\n", TranslateOpenCLError(err));
        return err;
    }
    */
    return CL_SUCCESS;
}

cl_uint CreateAndBuildProgram()
{
    cl_int err = CL_SUCCESS;

    // Upload the OpenCL C source code from the input file to source
    // The size of the C program is returned in sourceSize
    char* source = NULL;
    size_t src_size = 0;
    err = ReadSourceFromFile("ray_algorithm.cl", &source, &src_size);
    if (CL_SUCCESS != err)
    {
        printf("Error: ReadSourceFromFile returned %s.\n", TranslateOpenCLError(err));
        goto Finish;
    }

    // And now after you obtained a regular C string call clCreateProgramWithSource to create OpenCL program object.
    ocl.program = clCreateProgramWithSource(ocl.context, 1, (const char**)&source, &src_size, &err);
    if (CL_SUCCESS != err)
    {
        printf("Error: clCreateProgramWithSource returned %s.\n", TranslateOpenCLError(err));
        goto Finish;
    }

    // Build the program
    // During creation a program is not built. You need to explicitly call build function.
    // Here you just use create-build sequence,
    // but there are also other possibilities when program consist of several parts,
    // some of which are libraries, and you may want to consider using clCompileProgram and clLinkProgram as
    // alternatives.
    err = clBuildProgram(ocl.program, 2, ocl.device, "", NULL, NULL);
    if (CL_SUCCESS != err)
    {
        printf("Error: clBuildProgram() for source program returned %s.\n", TranslateOpenCLError(err));
    }

Finish:
    if (source)
    {
        delete[] source;
        source = NULL;
    }

    return err;
}

long timeval_diff(struct timeval *end_time,
        struct timeval *start_time)
{
    struct timeval temp_diff;

    temp_diff.tv_sec =end_time->tv_sec -start_time->tv_sec ;
    temp_diff.tv_usec=end_time->tv_usec-start_time->tv_usec;

    return 1000000LL*temp_diff.tv_sec+
        temp_diff.tv_usec;
}


cl_uint SetKernelArguments()
{
    cl_int err = CL_SUCCESS;

    err = clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), (void *)&ocl.Lights);
    if (CL_SUCCESS != err)
    {
        printf("error: Failed to set argument Lights, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 1, sizeof(cl_uint), (void *)&ocl.LightCount);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument LightCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 2, sizeof(cl_mem), (void *)&ocl.Shapes);
    if (CL_SUCCESS != err)
    {
        printf("error: Failed to set argument Shapes, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 3, sizeof(cl_uint), (void *)&ocl.ShapeCount);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument ShapeCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 4, sizeof(cl_uint), (void *)&ocl.sampleCount);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument ShapeCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 5, sizeof(cl_uint), (void *)&ocl.width);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument ShapeCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 6, sizeof(cl_uint), (void *)&ocl.height);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument ShapeCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    err = clSetKernelArg(ocl.kernel, 7, sizeof(cl_mem), (void *)&ocl.cam);
    if (CL_SUCCESS != err)
    {
        printf("Error: Failed to set argument ShapeCount, returned %s\n", TranslateOpenCLError(err));
        return err;
    }

    return err;
}

int main(int argc, char *argv[])
{
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    printf("OpenCL Initialization\n");

    cl_int err = CL_SUCCESS;
    
    struct timeval begin_init, end_init;
    gettimeofday(&begin_init, NULL);


    if(!createContext())
    {
        printf("Error: createContext\n");
        return 0;
    }

    if(!getDeviceIDs())
    {
        printf("Error: getDeviceIDs\n");
        return 0;
    }

    generateArgument();

    if(CreateAndBuildProgram() != CL_SUCCESS)
    {
        printf("Error: CreateAndBuildProgram\n");
        return 0;
    }

    if(CreateBufferArguments() != CL_SUCCESS)
    {
        printf("Error: CreateBufferArguments\n");
        return 0;
    }

    ocl.kernel = clCreateKernel(ocl.program, "ray_cal", &err);
    if(err != CL_SUCCESS)
    {
        printf("Error: clCreateKernel\n");
        return 0;
    }

    if(SetKernelArguments() != CL_SUCCESS)
    {
        printf("Error: SetKernelArguments\n");
        return 0;
    }

    gettimeofday(&end_init, NULL);
    printf("init elapsed time : %lfs\n", (double)timeval_diff(&end_init, &begin_init)/1000000);
    
    struct timeval begin_kernel, end_kernel;
    gettimeofday(&begin_kernel, NULL);

    srand((unsigned int)time(NULL));
    pthread_t t1, t2;

    int tmpvalue = WIDTH*HEIGHT/WorkAmount;

    printf("total recursive %d\n", tmpvalue);

    //PASSING_OCL ocl_info[2] = {{ocl, 0, 0, tmpvalue*5/8, 256}, {ocl, 1, tmpvalue*5/8, tmpvalue, 256}};
    
    PASSING_OCL ocl_info[2] = {{ocl, 0, 0, 8, 256}, {ocl, 1, 8, 16, 256}};
    //int joinstatus
    pthread_create(&t1, NULL, RenderDisplay, (void *)&ocl_info[0]);
    pthread_create(&t2, NULL, RenderDisplay, (void *)&ocl_info[1]);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL); 
    
    gettimeofday(&end_kernel, NULL);
    printf("render elapsed time : %lfs\n", (double)timeval_diff(&end_kernel, &begin_kernel)/1000000);
     
    /* 
    std::ostringstream headerStream;
    headerStream << "P6\n";
    headerStream << Width << ' ' << Height << '\n';
    headerStream << "255\n";
    std::ofstream fileStream("out.ppm", std::ios::out | std::ios::binary);

    fileStream << headerStream.str();

    for (unsigned int j = 0; j < Width*Height; j++)
    {
        unsigned char r, g, b;
        unsigned int tmp = ImagePixel[j];
        r = (unsigned char)((tmp >> 16) & 0xFF);
        g = (unsigned char)((tmp >> 8) & 0xFF);
        b = (unsigned char)((tmp)& 0xFF);
        fileStream << r << g << b;
    }
    
    fileStream.flush();
    fileStream.close();
    */
    //gettimeofday(&end_kernel, NULL);
    //printf("render elapsed time : %lfs\n", (double)timeval_diff(&end_kernel, &begin_kernel)/1000000);
     
    gettimeofday(&end, NULL);
    printf("elapsed time : %lfs\n", (double)timeval_diff(&end, &begin)/1000000);
    
    printf("End Success\n");

    return 0;
}
