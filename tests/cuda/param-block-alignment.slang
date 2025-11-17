//TEST(compute):COMPARE_COMPUTE_EX:-cuda -compute -output-using-type -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer

uniform uint frame;

struct Block {
    uint dummy;
};

ParameterBlock<Block> block;

RWStructuredBuffer<uint> outputBuffer;

// Helper struct to test layout
struct TestGlobalParams
{
    uint frame;
    Block* block;
};

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    TestGlobalParams gp = {};
    
    // Test the offset and alignment of the parameter block pointer
    outputBuffer[0] = __offsetOf(gp, gp.frame);       // Should be 0
    outputBuffer[1] = __offsetOf(gp, gp.block);       // Should be 8 (64-bit aligned)
    outputBuffer[2] = __sizeOf<TestGlobalParams>();   // Size of the struct
    outputBuffer[3] = __alignOf<Block*>();            // Alignment of pointer (should be 8)
    
    // Additional tests
    outputBuffer[4] = frame;        // Use the actual uniform
    outputBuffer[5] = block.dummy;  // Use the actual parameter block
}

