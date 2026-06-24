// Runtime functional complement to `descriptor-heap-unified-stride.slang`: that test statically
// checks the `-spirv-unified-descriptor-heap-stride` emission, while this one executes a bindless
// descriptor-handle shader and checks its result. The unified-stride path itself needs
// `SPV_EXT_descriptor_heap` driver support, so it is verified statically there; here we confirm the
// descriptor-handle resources resolve and index correctly at runtime via the bindless render
// feature. `input` is read at two offsets to exercise indexing, not just a bare load.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -vk -output-using-type -render-feature bindless -emit-spirv-directly

// RUNTIME: type: uint
// RUNTIME-NEXT: 30

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[10 20], stride=4):name=input
uniform RWStructuredBuffer<uint>.Handle input;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = input[0] + input[1];
}
