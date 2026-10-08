//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute

// `[ForceUnroll]` is semantics-bearing, and `unrollLoopsInModule`'s only
// pipeline call site is the cleanup group in the specialize fixpoint
// (slang-ir-specialize.cpp), which must run on every round. This shader is
// deliberately non-generic and non-differentiating so the fixpoint has no
// other reason to take a second round: if unrolling is ever re-gated on
// "the previous round performed specialization work", this loop stays rolled
// and the CHECK-NOT below fails. (The existing loop-unroll tests all involve
// generics or GPU-only targets, which mask exactly that regression.)

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; i++)
        sum += i * i;
    outputBuffer[0] = sum;
}

// CHECK: void computeMain
// CHECK-NOT: for(
// CHECK-NOT: [unroll]
