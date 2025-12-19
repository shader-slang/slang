//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-dx12 -compute -shaderobj -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-dx12 -compute -shaderobj -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-mtl -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-mtl -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-wgpu -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-wgpu -compute -output-using-type -xslang -matrix-layout-row-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-llvm -compute -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-llvm -compute -output-using-type -xslang -matrix-layout-row-major

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int selectDims<int N, int M>(bool cond)
{
    return select(
        matrix<bool, N, M>(cond),
        matrix<int, N, M>(1),
        matrix<int, N, M>(0)
    )[0][0];
}

int selectDimsDigit<int N, int M, int D>(int x)
{
    return selectDims<N, M>(((x >> D) & 0b1) == 0b1) << D;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = 324;

    int s = 0;
    s += selectDimsDigit<2, 2, 0>(x);
    s += selectDimsDigit<2, 3, 1>(x);
    s += selectDimsDigit<2, 4, 2>(x);
    s += selectDimsDigit<3, 2, 3>(x);
    s += selectDimsDigit<3, 3, 4>(x);
    s += selectDimsDigit<3, 4, 5>(x);
    s += selectDimsDigit<4, 2, 6>(x);
    s += selectDimsDigit<4, 3, 7>(x);
    s += selectDimsDigit<4, 4, 8>(x);

    // CHK: 324
    outputBuffer[0] = s;
} 
