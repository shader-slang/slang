// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -DCASE=1
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -DCASE=1
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -DCASE=2
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -DCASE=3
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -DCASE=4

[host]
uint hostHelper(uint value)
{
    return value + 1;
}

uint indirectCall(uint value)
{
    return hostHelper(value);
}

interface IGeometry
{
    [host] uint build(uint amount);
}

struct Geometry<T> : IGeometry
{
    [host] uint build(uint amount)
    {
        return hostHelper(amount);
    }
}

uint genericCall<G : IGeometry>(G geometry, uint amount)
{
    return geometry.build(amount);
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
#if CASE == 1
    output[tid.x] = hostHelper(tid.x);
#elif CASE == 2
    output[tid.x] = indirectCall(tid.x);
#elif CASE == 3
    output[tid.x] = Geometry<uint>().build(tid.x);
#else
    output[tid.x] = genericCall(Geometry<uint>(), tid.x);
#endif
}

// CHECK: entrypoint 'computeMain' uses features that are not available in 'compute' stage
