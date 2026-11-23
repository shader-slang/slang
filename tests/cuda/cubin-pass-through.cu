//TEST:SIMPLE: -target cubin -pass-through nvrtc -entry computeMain -Xnvrtc -arch=sm_80 -o cubin-pass-through.cu.0.actual.cubin
//TEST:SIMPLE(filecheck=PTX): -target ptx -pass-through nvrtc -entry computeMain -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=TARGET,exit-code=2): -target dxil -pass-through nvrtc -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=NOARTIFACT,exit-code=2): -target ptx -pass-through nvdisasm -entry computeMain

// PTX: .target sm_80
// TARGET: NVRTC cannot compile to target 'dxil'
// NOARTIFACT: Downstream compiler returned no output artifact.

#ifdef SLANG_CUDA_ENABLE_OPTIX
#error Selecting a CUDA SM for an ordinary kernel must not enable OptiX.
#endif

extern "C" __global__ void computeMain()
{
}
