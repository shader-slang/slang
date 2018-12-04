//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage compute

// Confirm that all HLSL barrier operations can be used when cross-compiling.

[numthreads(1,1,1)]
void main()
{
	AllMemoryBarrier();
	AllMemoryBarrierWithGroupSync();
	DeviceMemoryBarrier();
	DeviceMemoryBarrierWithGroupSync();
	GroupMemoryBarrier();
	GroupMemoryBarrierWithGroupSync();
}
