
__kernel void imgdiff_cal(__global const unsigned char *images1, __global const unsigned char *images2,
			__global double* result, __local double* l_sum, int GROUP_WIDTH, int GROUP_HEIGHT)
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
		
	if((x >= GROUP_WIDTH) || (y >= GROUP_HEIGHT))
	{	
		l_sum[lidx] = 0.0;
	}
	else
	{
		int r = (images1[idx*3] - images2[idx*3]);
		int g = (images1[idx*3 + 1] - images2[idx*3 + 1]);
		int b =	(images1[idx*3 + 2] - images2[idx*3 + 2]);
		diff = sqrt((double)(r*r + g*g + b*b));
		l_sum[lidx] = diff;
		//result[idx] = diff;
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	for(int p = lwidth*lheight / 2; p >= 1; p = p >> 1)
	{
		if (lidx < p) l_sum[lidx] += l_sum[lidx + p];
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	
	
	if(lidx == 0)
		result[get_group_id(0) + (get_group_id(1) * (get_global_size(0) / get_local_size(0)))] = l_sum[0];

	
}
