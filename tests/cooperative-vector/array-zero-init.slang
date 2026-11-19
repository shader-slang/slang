// A cooperative vector must never lower to an empty (zero-constituent)
// composite: zero-initializing an array of them, (CoopVec<T,N>[M])0, has to
// splat each element to N constituents, not an empty OpConstantComposite.
// Companion to tests/cooperative-matrix/array-zero-init.slang (issue #12556);
// both shapes route through the same getDefaultVal / emitDefaultConstruct fix.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    //CHECK: %[[VECTY:[0-9A-Za-z_]+]] = OpTypeVectorIdEXT %float %int_8
    CoopVec<float, 8> v[2] = (CoopVec<float, 8>[2])0;

    // The default must be an 8-constituent splat of that type, never the empty
    // form (an OpConstantComposite of the coop-vector type with no constituents).
    //CHECK-NOT: OpConstantComposite %[[VECTY]]{{$}}
    //CHECK: OpCompositeConstruct %[[VECTY]] %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0 %float_0
    //CHECK-NOT: OpConstantComposite %[[VECTY]]{{$}}

    outputBuffer[0] = v[0][0] + v[1][7];
}
