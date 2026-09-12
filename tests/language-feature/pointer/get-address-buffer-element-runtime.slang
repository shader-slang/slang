// Runtime coverage for the subtly-different accepted forms of `__getAddress(buf[i])`
// (shader-slang/slang#12581): a member-of-element address with write-back, and cross-element
// pointer arithmetic on a struct-element buffer. This executes the paths so a wrong field
// offset or element stride is caught rather than silently miscompiled. (ROV element access
// runs on Vulkan in the sibling `-ordered.slang` test; the SPIR-V pointer stride for a
// non-default layout is checked in `tests/spirv/get-address-buffer-element-layout.slang`.)

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Pair
{
    int x;
    int y;
}

//TEST_INPUT:ubuffer(data=[10 11 20 21 30 31], stride=8):name=pairBuffer
RWStructuredBuffer<Pair> pairBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Member-of-element address: read the second field of element 0.
    int* py = __getAddress(pairBuffer[0].y);
    // CHECK: 11
    outputBuffer[0] = *py;

    // Write back through a member-of-element address.
    *py = 77;
    // CHECK: 77
    outputBuffer[1] = pairBuffer[0].y;

    // Cross-element arithmetic on a struct-element pointer: (base + 2) must land on
    // element 2, exercising the buffer's element stride.
    Pair* pp = __getAddress(pairBuffer[0]);
    // CHECK: 30
    outputBuffer[2] = (pp + 2).x;
    // CHECK: 31
    outputBuffer[3] = pp[2].y;
}
