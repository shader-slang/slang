//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -stage compute -target spirv -emit-spirv-directly -entry main
// Confirm that all HLSL barrier operations can be used when cross-compiling.

[numthreads(1,1,1)]
void main()
{
    // SPIRV: OpMemoryBarrier %int_1 %int_2376
    // SPIRV: OpControlBarrier %int_2 %int_1 %int_2376
    // SPIRV: OpMemoryBarrier %int_1 %int_2120
    // SPIRV: OpControlBarrier %int_2 %int_1 %int_2120
    // SPIRV: OpMemoryBarrier %int_2 %int_264
    // SPIRV: OpControlBarrier %int_2 %int_2 %int_264
	AllMemoryBarrier();
	AllMemoryBarrierWithGroupSync();
	DeviceMemoryBarrier();
	DeviceMemoryBarrierWithGroupSync();
	GroupMemoryBarrier();
	GroupMemoryBarrierWithGroupSync();
}
