//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj -output-using-type

// Runs the WGSL lowering of QuadReadLaneAt, which broadcasts from all four quad lanes and then
// selects one (WGSL's quadBroadcast needs a const-expression lane). WGPU only: a lane that differs
// between the invocations of a quad is allowed by this lowering, but not by SPIR-V's
// OpGroupNonUniformQuadBroadcast.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    const int idx = int(tid.x);

    // A different lane in each invocation: invocation i reads lane 3 - i.
    const int crossed = QuadReadLaneAt(idx * 10, uint(3 - idx));
    // An out-of-range lane wraps: 6 reads lane 2.
    const int wrapped = QuadReadLaneAt(idx * 10, 6u);
    // A bool and a bool vector go through 0/1.
    const bool fromLane1 = QuadReadLaneAt(idx == 1, 1u);
    const bool2 fromLane2 = QuadReadLaneAt(bool2(idx == 2, idx != 2), 2u);

    outputBuffer[idx * 4 + 0] = crossed;
    outputBuffer[idx * 4 + 1] = wrapped;
    outputBuffer[idx * 4 + 2] = fromLane1 ? 1 : 0;
    outputBuffer[idx * 4 + 3] = (fromLane2.x ? 1 : 0) + (fromLane2.y ? 2 : 0);
}

// CHECK: 30
// CHECK-NEXT: 20
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 20
// CHECK-NEXT: 20
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 10
// CHECK-NEXT: 20
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 20
// CHECK-NEXT: 1
// CHECK-NEXT: 1
