
__kernel void imgdiff_cal(__global const unsigned int *images, __global double* result)
{
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int width = get_global_size(0);

	result[0] = 1.0;
	
}
