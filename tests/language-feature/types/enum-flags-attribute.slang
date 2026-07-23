// enum-flags-attribute.slang

// Test that [Flags] attribute assigns power-of-2 values starting at 1.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Flags]
enum Permissions
{
    None = 0,
    Read,     // = 1
    Write,    // = 2
    Execute,  // = 4
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 1
    outputBuffer[0] = int(Permissions.Read);

    // CHECK: 2
    outputBuffer[1] = int(Permissions.Write);

    // CHECK: 4
    outputBuffer[2] = int(Permissions.Execute);

    // Combined flags: Read | Write | Execute = 7
    // CHECK: 7
    outputBuffer[3] = int(Permissions.Read | Permissions.Write | Permissions.Execute);
}
