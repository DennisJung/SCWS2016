
__kernel void imgdiff_cal(__global const unsigned char *images1, __global const unsigned char *images2,
			__global double* result, int GROUP_WIDTH, int GROUP_HEIGHT, const int SAMPLE_COUNT)
{
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = GROUP_WIDTH;
	int idx = x + y*width;
	
	int lx = get_local_id(0);	
	int ly = get_local_id(1);
	int lwidth = get_local_size(0);
	int lheight = get_local_size(1);
	int lidx = lx + ly * lwidth;
	
	double diff = 0;
	const int TOTAL_WORK_COUNT = get_global_size(0) * get_global_size(1) / (get_local_size(0) * get_local_size(1));

	__local double 	l_sum[4096];

	int i;
	
	if((x >= GROUP_WIDTH) || (y >= GROUP_HEIGHT))
	{
		for(i=0; i<SAMPLE_COUNT; i++)
			l_sum[lidx + i*256] = 0.0;
	}
	else
	{
		for(i=0; i<SAMPLE_COUNT; i++)
		{
			int r = (images1[idx * 3] - images2[(idx + (GROUP_WIDTH * GROUP_HEIGHT * i))* 3]);
			int g = (images1[idx * 3 + 1] - images2[(idx + (GROUP_WIDTH * GROUP_HEIGHT * i))* 3 + 1]);

			int b =	(images1[idx * 3 + 2] - images2[(idx + (GROUP_WIDTH * GROUP_HEIGHT * i))* 3 + 2]);

			diff = sqrt((double)(r*r + g*g + b*b));
			l_sum[lidx + i*256] = diff;
			//l_sum[idx + i*256] = 1.0;
			//result[idx + GROUP_WIDTH * GROUP_HEIGHT * i] = diff;
			//result[idx] = diff;
		}
	}
	
	barrier(CLK_LOCAL_MEM_FENCE);
	for(i=0; i<SAMPLE_COUNT; i++)
	{
		for(int p = lwidth*lheight / 2; p >= 1; p = p >> 1)
		{
			if (lidx < p) l_sum[lidx + i*256] += l_sum[lidx + p + i*256];
			barrier(CLK_LOCAL_MEM_FENCE);
		}

	}	
	if(lidx == 0)
	{
		for(i=0; i<SAMPLE_COUNT; i++)
			result[(get_group_id(0) + (get_group_id(1) * (get_global_size(0) / get_local_size(0)))) + (TOTAL_WORK_COUNT * i)] = l_sum[i*256];
	}
	

}
