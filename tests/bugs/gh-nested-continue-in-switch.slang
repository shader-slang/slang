//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain

// Regression test for nested continue inside a switch within a while loop
// This tests the case where a continue statement inside a switch that is inside a loop
// should correctly target the loop's continue block, not cause a compiler exception.

//TEST_INPUT:ubuffer(data=[0 1 2 3], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint bitmap = dispatchThreadID.x;
    uint value = 0;
    uint iterations = 0;

    // Test while loop with continue in switch
    while (bitmap < 3 && iterations < 10) {
        switch (bitmap) {
        case 0:
            if (dispatchThreadID.x < 10) {
                bitmap++;
                iterations++;
                continue;
            }
            value += 1;
            break;
        case 1:
            value += 2;
            break;
        default:
            value += 3;
            break;
        }
        bitmap++;
        iterations++;
    }

    outputBuffer[dispatchThreadID.x] = value;
    // CHECK: OpEntryPoint
}
