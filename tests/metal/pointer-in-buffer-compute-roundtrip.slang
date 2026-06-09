//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=BUF):-metal -render-features argument-buffer-tier-2 -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -emit-spirv-directly -output-using-type

// Runtime validation of three-level pointer chain (int*** -> int** -> int* -> int).
// Builds the pointer chain in a setup function using __getAddress on struct
// fields accessed through uniform pointers, then dereferences through all
// three levels in a separate function to verify correctness.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Level1 { int* ptr; };
struct Level2 { int** ptr; };
struct Level3 { int*** ptr; };

//TEST_INPUT: set intData = ubuffer(data=[42], stride=4)
uniform int* intData;

//TEST_INPUT: set l1 = ubuffer(data=[0 0 0 0], stride=4)
uniform Level1* l1;

//TEST_INPUT: set l2 = ubuffer(data=[0 0 0 0], stride=4)
uniform Level2* l2;

//TEST_INPUT: set l3 = ubuffer(data=[0 0 0 0], stride=4)
uniform Level3* l3;

//TEST_INPUT: ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void buildChain(Level1* s1, Level2* s2, Level3* s3, int* data)
{
    s1->ptr = data;                  // int* -> int
    s2->ptr = __getAddress(s1->ptr); // int** -> int* (address of Level1.ptr)
    s3->ptr = __getAddress(s2->ptr); // int*** -> int** (address of Level2.ptr)
}

int derefTriple(Level3* s3)
{
    int*** ppp = s3->ptr;
    int** pp = *ppp;
    int* p = *pp;
    return *p;
}

// Level2 and Level3 pointer fields should be lowered to ulong.
// CHECK: ulong ptr_
// CHECK: ulong ptr_

// Triple deref with cast chain.
// CHECK-LABEL: derefTriple
// CHECK: int device* device*

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    buildChain(l1, l2, l3, intData);
    outputBuffer[0] = derefTriple(l3);
}

// BUF: 42
