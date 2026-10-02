//TEST:SIMPLE(filecheck=PTX): -target ptx -entry raygenMain -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 80000
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry raygenMain -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 80100
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry raygenMain -stage raygeneration -emit-cuda-via-nvvm -capability cuda_sm_8_0 -optix-version 90000
//TEST:SIMPLE(filecheck=CUDA80): -target cuda -entry raygenMain -stage raygeneration -optix-version 80000
//TEST:SIMPLE(filecheck=CUDA81): -target cuda -entry raygenMain -stage raygeneration -optix-version 80100
//TEST:SIMPLE(filecheck=CUDA90): -target cuda -entry raygenMain -stage raygeneration -optix-version 90000

RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void raygenMain()
{
    output[DispatchRaysIndex().x] = DispatchRaysDimensions().x;
}

// PTX: .entry __raygen__raygenMain
// PTX: _optix_get_launch_index_x
// CUDA80: OPTIX_VERSION != 80000
// CUDA81: OPTIX_VERSION != 80100
// CUDA90: OPTIX_VERSION != 90000
