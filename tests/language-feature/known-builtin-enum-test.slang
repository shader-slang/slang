//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Test that KnownBuiltin attribute works with enum-based system  
// Using GeometryStreamAppend enum value (0) for testing
static const int GEOMETRY_STREAM_APPEND_BUILTIN = 0;

[KnownBuiltin(GEOMETRY_STREAM_APPEND_BUILTIN)]
void testKnownBuiltin()
{
    // This function just needs to exist to test attribute processing
}

[numthreads(1,1,1)]
void computeMain()
{
    // Simple test that the code compiles and runs
    outputBuffer[0] = 42.0f;
    // CHECK: 42.0
}