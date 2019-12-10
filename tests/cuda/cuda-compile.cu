//TEST(smoke):SIMPLE: -pass-through nvrtc -target ptx -entry hello

__global__ 
void hello(char *a, int *b) 
{
	a[threadIdx.x] += b[threadIdx.x];
}
