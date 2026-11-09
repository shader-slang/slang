// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

import slang.pipeline_rt;

typealias RTTypes = HWRaytracingContext<>;

[shader("compute")]
[numthreads(1, 1, 1)]
void store<T : __BuiltinArithmeticType>(RWStructuredBuffer<T> output)
{
    output[0] = T(42);
}

[host]
void main()
{
    // These logical allocations emit heap events without requiring native ray-tracing support.
    let tracker = makeRaytracingPipeline(1);
    let blas = RTTypes.BottomLevelAccelerationStructure::alloc();
    let scene = RaytracingAccelerationStructure::alloc();
    var output = RWStructuredBuffer<uint>::alloc(1);
    dispatch(store<uint>, uint3(1, 1, 1))(output);
    print(readBack(output, 0));
}
