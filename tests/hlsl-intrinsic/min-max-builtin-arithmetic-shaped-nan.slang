//TEST(compute):COMPARE_COMPUTE_EX:-cpu -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX:-llvm -compute -shaderobj

// Exercise the LLVM fallback's established behavior when exactly one operand is NaN. GPU-native
// min/max operations do not all guarantee this result, so this regression is intentionally limited
// to the CPU/LLVM paths implemented by `__min_impl` and `__max_impl`.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
//TEST_INPUT:ubuffer(data=[0x7fc00000], stride=4):name=inputBuffer
RWStructuredBuffer<int> outputBuffer;
StructuredBuffer<uint> inputBuffer;

T genericMinimum<T : __BuiltinArithmeticType>(T left, T right)
{
    return min(left, right);
}

T genericMaximum<T : __BuiltinArithmeticType>(T left, T right)
{
    return max(left, right);
}

vector<T, N> genericMinimum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return min(left, right);
}

vector<T, N> genericMaximum<T : __BuiltinArithmeticType, let N : int>(
    vector<T, N> left,
    vector<T, N> right)
{
    return max(left, right);
}

matrix<T, R, C, L> genericMinimum<
    T : __BuiltinArithmeticType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(
    matrix<T, R, C, L> left,
    matrix<T, R, C, L> right)
{
    return min(left, right);
}

matrix<T, R, C, L> genericMaximum<
    T : __BuiltinArithmeticType,
    let R : int,
    let C : int,
    let L : MatrixLayoutMode>(
    matrix<T, R, C, L> left,
    matrix<T, R, C, L> right)
{
    return max(left, right);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float nanValue = asfloat(inputBuffer[0]);
    float2 nanMinimum = genericMinimum(float2(nanValue, 5.0), float2(3.0, nanValue));
    float2 nanMaximum = genericMaximum(float2(nanValue, -5.0), float2(7.0, nanValue));
    outputBuffer[0] = int(nanMinimum.x);
    outputBuffer[1] = int(nanMinimum.y);
    outputBuffer[2] = int(nanMaximum.x);
    outputBuffer[3] = int(nanMaximum.y);
    outputBuffer[4] = int(genericMinimum(nanValue, 3.0));
    outputBuffer[5] = int(genericMaximum(7.0, nanValue));

    matrix<float, 2, 2, MatrixLayoutMode.RowMajor> nanMatrixMinimum = genericMinimum(
        matrix<float, 2, 2, MatrixLayoutMode.RowMajor>(nanValue, 5.0, nanValue, 9.0),
        matrix<float, 2, 2, MatrixLayoutMode.RowMajor>(3.0, nanValue, 7.0, 8.0));
    matrix<float, 2, 2, MatrixLayoutMode.RowMajor> nanMatrixMaximum = genericMaximum(
        matrix<float, 2, 2, MatrixLayoutMode.RowMajor>(nanValue, -5.0, 7.0, nanValue),
        matrix<float, 2, 2, MatrixLayoutMode.RowMajor>(3.0, nanValue, nanValue, 8.0));
    outputBuffer[6] = int(nanMatrixMinimum[0][0]);
    outputBuffer[7] = int(nanMatrixMinimum[0][1]);
    outputBuffer[8] = int(nanMatrixMinimum[1][0]);
    outputBuffer[9] = int(nanMatrixMinimum[1][1]);
    outputBuffer[10] = int(nanMatrixMaximum[0][0]);
    outputBuffer[11] = int(nanMatrixMaximum[0][1]);
    outputBuffer[12] = int(nanMatrixMaximum[1][0]);
    outputBuffer[13] = int(nanMatrixMaximum[1][1]);
}
