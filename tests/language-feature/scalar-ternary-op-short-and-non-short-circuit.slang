// Verify that:
// - The ternary conditional operator with a scalar condition
//   evaluates only the taken branch.
// - The ternary conditional operator with a vector condition
//   evaluates both branches
//
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -compute  -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -compute  -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type -xslang -Wno-30056


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

static uint thenBranches = 0U;
static uint elseBranches = 0U;

static uint4 thenBranchesV = { 0, 0, 0, 0 };
static uint4 elseBranchesV = { 0, 0, 0, 0 };

uint4 incV(inout uint4 v)
{
    v = v + uint4(1U, 0U, 0U, 0U);
    return v;
}


[numthreads(4,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // first: check that '?:' does what it's supposed to do for a scalar
    output[tid.x] = tid.x >= 2U ? 4242U : 0U;

    //CHECK: 0
    //CHECK-NEXT: 0
    //CHECK-NEXT: 4242
    //CHECK-NEXT: 4242

    // then: check that '?:' for a scalar is short-circuiting
    (void)(tid.x >= 2U ? thenBranches++ : elseBranches++);
    output[tid.x + 4U] = (thenBranches * 1000U) + (elseBranches * 2U);

    // CHECK-NEXT: 2
    // CHECK-NEXT: 2
    // CHECK-NEXT: 1000
    // CHECK-NEXT: 1000

    // now check that '?:' does what it's suppose to do for a vector
    output[tid.x + 8U] = (uint4(tid.x) >= uint4(2U) ? 6767U * incV(thenBranchesV) : 1111U * incV(elseBranchesV)).x;
    // CHECK-NEXT: 1111
    // CHECK-NEXT: 1111
    // CHECK-NEXT: 6767
    // CHECK-NEXT: 6767

    // and finally, check that '?:' for a vector is non-short-circuiting
    output[tid.x + 12U] = (thenBranchesV.x * 1000U) + (elseBranchesV.x * 2U);

    // CHECK-NEXT: 1002
    // CHECK-NEXT: 1002
    // CHECK-NEXT: 1002
    // CHECK-NEXT: 1002
}
