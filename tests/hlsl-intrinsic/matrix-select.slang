//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -dx12 -shaderobj -output-using-type -xslang -matrix-layout-column-major
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -dx12 -shaderobj -output-using-type -xslang -matrix-layout-row-major

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int selectDims<let N : int, let M : int>(bool cond)
{
    return select(
        matrix<bool, N, M>(cond),
        matrix<int, N, M>(1),
        matrix<int, N, M>(0)
    )[0][0];
}

int selectDimsDigit<let N : int, let M : int, let D : int>(int x)
{
    return selectDims<N, M>(((x >> D) & 0b1) == 0b1) << D;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int x = 12345;

    int s = 0;
    s += selectDimsDigit<1, 1, 0>(x);
    s += selectDimsDigit<1, 2, 1>(x);
    s += selectDimsDigit<1, 3, 2>(x);
    s += selectDimsDigit<1, 4, 3>(x);
    s += selectDimsDigit<2, 1, 4>(x);
    s += selectDimsDigit<2, 2, 5>(x);
    s += selectDimsDigit<2, 3, 6>(x);
    s += selectDimsDigit<2, 4, 7>(x);
    s += selectDimsDigit<3, 1, 8>(x);
    s += selectDimsDigit<3, 2, 9>(x);
    s += selectDimsDigit<3, 3, 10>(x);
    s += selectDimsDigit<3, 4, 11>(x);
    s += selectDimsDigit<4, 1, 12>(x);
    s += selectDimsDigit<4, 2, 13>(x);
    s += selectDimsDigit<4, 3, 14>(x);
    s += selectDimsDigit<4, 4, 15>(x);

    // CHK: 12345
    outputBuffer[0] = s;
}
