//TEST(compute):SIMPLE(filecheck=SM5):-target hlsl -profile cs_5_1 -entry computeMain
//TEST(compute):SIMPLE(filecheck=SM6):-target hlsl -profile cs_6_0 -entry computeMain
//TEST(compute):SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -compute  -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cuda -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj

// SM6.0 and above require to use `and()` and `or()` when the operands are non-scalar.
// And SM below SM6.0 doesn't have the functions, `and()` and `or()`.

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int result = 0;

bool2 assignFunc(int index)
{
    result++;
    return bool2(true);
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    //SM5:!all({{.*}}&&
    //SM6:!all({{ *}}and(
    //WGSL:!all( select(vec2<bool>(false),
    if (!all(bool2(index < 2) && assignFunc(index)))
    {
        result++;
    }

    outputBuffer[index] = result;

    //CHK-COUNT-2: 1
    //CHK-COUNT-2: 2
}
