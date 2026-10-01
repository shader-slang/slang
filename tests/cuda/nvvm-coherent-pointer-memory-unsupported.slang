// Unsupported contracts must not fall through to ordinary loads or stores.
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry selected -stage compute -capability cuda_sm_8_0 -o nvvm-coherent-unsupported-cuda.cu
//DIAGNOSTIC_TEST:SIMPLE(diag=SM,non-exhaustive):-target ptx -emit-cuda-via-nvvm -entry selected -stage compute -capability cuda_sm_6_0 -o nvvm-coherent-unsupported-sm60.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry globalWorkgroup -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-global-workgroup.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry sharedDevice -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-shared-device.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry floating -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-float.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry vectorValue -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-vector.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry weakAlignment -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-alignment.ptx
//DIAGNOSTIC_TEST:SIMPLE(diag=BAD):-target ptx -emit-cuda-via-nvvm -entry unknownRoot -stage compute -capability cuda_sm_8_0 -O3 -o nvvm-coherent-unsupported-root.ptx

uniform int* ints;
uniform float* floats;
uniform int2* vectors;
uniform uint64_t address;
RWStructuredBuffer<int> output;
groupshared int sharedValue;

[numthreads(1, 1, 1)]
void selected() { output[0] = loadCoherent<4, MemoryScope::Device>(ints); }
[numthreads(1, 1, 1)]
void globalWorkgroup() { output[0] = loadCoherent<4, MemoryScope::Workgroup>(ints); }
[numthreads(1, 1, 1)]
void sharedDevice() { storeCoherent<4, MemoryScope::Device>(__getAddress(sharedValue), 1); }
[numthreads(1, 1, 1)]
void floating() { output[0] = int(loadCoherent<4, MemoryScope::Device>(floats)); }
[numthreads(1, 1, 1)]
void vectorValue() { output[0] = loadCoherent<8, MemoryScope::Device>(vectors).x; }
[numthreads(1, 1, 1)]
void weakAlignment() { output[0] = loadCoherent<2, MemoryScope::Device>(ints); }
[numthreads(1, 1, 1)]
void unknownRoot() { output[0] = loadCoherent<4, MemoryScope::Device>(Ptr<int>(address)); }

//CUDA: result code = {{-?[1-9][0-9]*}}
//CUDA: error[E41011]
//SM: direct NVVM lowering does not support Slang IR instruction or shape 'scoped memory requires SM 7.0'
//BAD: direct NVVM lowering does not support Slang IR instruction or shape
