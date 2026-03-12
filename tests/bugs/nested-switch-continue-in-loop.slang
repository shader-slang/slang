//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry computeMain

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type -Xslang -DWHILE_LOOP
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -DWHILE_LOOP

// Disable dx11/FXC: "error X3708: 'continue' cannot be used in a switch"
//DISABLE_TEST(compute):COMPARE_COMPUTE:-dx11 -output-using-type
//DISABLE_TEST(compute):COMPARE_COMPUTE:-dx11 -output-using-type -Xslang -DWHILE_LOOP

// Test `continue` statement inside of switch inside of a loop.

// SPIRV: OpEntryPoint

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[[noinline]]
int test(int value)
{
    int result = 0;
#if defined(WHILE_LOOP)
    uint i = 0;
    while(i < 3)
    {
        i++;
#else
    for (int i = 0; i < 3; i++)
    {
#endif
        switch (value)
        {
        case 0:
            result += 1;
            continue; // triggers the bug: continue from switch inside for-loop
        case 1:
            result += 2;
            break;
        default:
            result += 3;
            break;
        }
        result += 10;
    }
    return result;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 3
    // test(0): case 0 hits, result += 1, continue (skips +10) -> 3 iterations -> result = 3
    outputBuffer[0] = test(0);

    // CHECK: 36
    // test(1): case 1 hits, result += 2, break, then +10 -> 3 iterations -> result = 36
    outputBuffer[1] = test(1);

    // CHECK: 39
    // test(2): default hits, result += 3, break, then +10 -> 3 iterations -> result = 39
    outputBuffer[2] = test(2);
}

