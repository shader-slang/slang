//TEST(compute):SIMPLE(filecheck=SM5):-target hlsl -profile cs_5_1 -entry computeMain
//TEST(compute):SIMPLE(filecheck=SM6):-target hlsl -profile cs_6_0 -entry computeMain
//TEST(compute):SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-slang -compute -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-vk -compute  -shaderobj -output-using-type -xslang -Wno-30056
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK):-mtl -compute  -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cuda -compute -shaderobj -output-using-type -xslang -Wno-30056
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj -output-using-type -xslang -Wno-30056

// Testnig logical-AND, logical-OR and ternary operator with non-scalar operands

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

static int result = 0;

bool2 assignFunc(int index)
{
    result += 10;
    return bool2(true);
}

[numthreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = dispatchThreadID.x;

    // No short-circuiting for vector types

    //SM5:(all({{.*}}&&
    //SM6:(all(and(
    //WGSL:(all( select(vec2<bool>(false),
    if (all(bool2(index >= 1) && assignFunc(index)))
    {
        result++;
    }

    // Intentionally using non-boolean type for testing.

    //SM5:(all({{.*}}||
    //SM6:(or(vector<bool,2>(
    //WGSL:( select({{.*}}, vec2<bool>(true), vec2<bool>(
    if (all(int2(index >= 2) || !assignFunc(index)))
    {
        result++;
    }

    //SM5:(all({{.*}}?{{.*}}:
    //SM6:(all(select(
    //WGSL:(all(select(vec2<bool>(false),
    if (all(bool2(index >= 3) ? assignFunc(index) : bool2(false)))
    {
        result++;
    }

    outputBuffer[index] = result;

    //CHK:30
    //CHK-NEXT:31
    //CHK-NEXT:32
    //CHK-NEXT:33
}
