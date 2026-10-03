// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Falcor's linked visibility-miss shader retains its exported light-sampler types, although
// their values/resources are unused in that entry point. References between types are not
// executable uses and must not make preflight demand a value/storage representation.
public struct Entry { public float probability; public uint alias; }
public struct Distribution { public uint size; public StructuredBuffer<Entry> entries; }
public export struct Sampler { public Distribution distribution; public float power; }

//TEST_INPUT: ubuffer(data=[0 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = 37;
}
// PTX: .entry

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}37{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
