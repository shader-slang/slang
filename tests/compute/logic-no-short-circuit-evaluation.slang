//TEST(compute):SIMPLE(filecheck=SM5):-target hlsl -profile cs_5_1 -entry computeMain
//TEST(compute):SIMPLE(filecheck=SM6):-target hlsl -profile cs_6_0 -entry computeMain
//TEST(compute):SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-slang -compute -shaderobj -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -compute  -shaderobj -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cuda -compute -shaderobj -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj -xslang -Wno-30056

// Testnig logical-AND, logical-OR and ternary operator with non-scalar operands

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

    //SM5:(all({{.*}}&&
    //SM6:(all(and(
    //WGSL:(all(select(vec2<bool>(false),
    if (all(bool2(index >= 1) && assignFunc(index)))
    {
        result++;
    }

    //SM5:(all({{.*}}||
    //SM6:(all(or(
    //WGSL:(all(select({{.*}}vec2<bool>(true),
    if (all(bool2(index >= 2) || !assignFunc(index)))
    {
        result++;
    }

    //SM5:(all({{.*}}?{{.*}}:
    //SM6:(all(select({{ *}}
    //WGSL:(all(select(
    if (all(bool2(index >= 3) ? assignFunc(index) : bool2(false)))
    {
        result++;
    }

    outputBuffer[index] = result;

    //CHK:3
    //CHK-NEXT:4
    //CHK-NEXT:5
    //CHK-NEXT:6
}
