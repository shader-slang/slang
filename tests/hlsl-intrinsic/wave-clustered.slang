//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl -profile sm_6_0 -Xslang... -capability GL_KHR_shader_subgroup_clustered -X.

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

__generic<T : __BuiltinLogicalType>
bool test1Logical()
{
    return true
        & WaveClusteredBitAnd(T(1), 4) == T(1)
        & WaveClusteredBitOr(T(1), 4) == T(1)
        & WaveClusteredBitXor(T(1), 4) == T(0)
        ;
}

__generic<T : __BuiltinLogicalType, let N : int>
bool testVLogical()
{
    typealias gvec = vector<T, N>;

    return true
        & all(WaveClusteredBitAnd(gvec(T(1)), 4) == gvec(T(1)))
        & all(WaveClusteredBitOr(gvec(T(1)), 4) == gvec(T(1)))
        & all(WaveClusteredBitXor(gvec(T(1)), 4) == gvec(T(0)))
        ;
}

bool testLogical()
{
    return true
        & test1Logical<int>()
        & testVLogical<int, 2>()
        & testVLogical<int, 3>()
        & testVLogical<int, 4>()
        & test1Logical<int8_t>()
        & testVLogical<int8_t, 2>()
        & testVLogical<int8_t, 3>()
        & testVLogical<int8_t, 4>()
        & test1Logical<int16_t>()
        & testVLogical<int16_t, 2>()
        & testVLogical<int16_t, 3>()
        & testVLogical<int16_t, 4>()
        & test1Logical<int64_t>()
        & testVLogical<int64_t, 2>()
        & testVLogical<int64_t, 3>()
        & testVLogical<int64_t, 4>()
        & test1Logical<uint>()
        & testVLogical<uint, 2>()
        & testVLogical<uint, 3>()
        & testVLogical<uint, 4>()
        & test1Logical<uint8_t>()
        & testVLogical<uint8_t, 2>()
        & testVLogical<uint8_t, 3>()
        & testVLogical<uint8_t, 4>()
        & test1Logical<uint16_t>()
        & testVLogical<uint16_t, 2>()
        & testVLogical<uint16_t, 3>()
        & testVLogical<uint16_t, 4>()
        & test1Logical<uint64_t>()
        & testVLogical<uint64_t, 2>()
        & testVLogical<uint64_t, 3>()
        & testVLogical<uint64_t, 4>()
        & test1Logical<bool>()
        & testVLogical<bool, 2>()
        & testVLogical<bool, 3>()
        & testVLogical<bool, 4>()
        ;
}

__generic<T : __BuiltinArithmeticType>
bool test1Arithmetic()
{
    return true
        & WaveClusteredSum(T(1), 4) == T(4)
        & WaveClusteredProduct(T(1), 4) == T(1)
        & WaveClusteredMin(T(1), 4) == T(1)
        & WaveClusteredMax(T(1), 4) == T(1)
        ;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool testVArithmetic()
{
    typealias gvec = vector<T, N>;

    return true
        & all(WaveClusteredSum(gvec(T(1)), 4) == gvec(T(4)))
        & all(WaveClusteredProduct(gvec(T(1)), 4) == gvec(T(1)))
        & all(WaveClusteredMin(gvec(T(1)), 4) == gvec(T(1)))
        & all(WaveClusteredMax(gvec(T(1)), 4) == gvec(T(1)))
        ;
}

bool testArithmetic()
{
    return true
        & test1Arithmetic<float>()
        & testVArithmetic<float, 2>()
        & testVArithmetic<float, 3>()
        & testVArithmetic<float, 4>()
        & test1Arithmetic<double>() // WARNING: intel GPU's lack FP64 support
        & testVArithmetic<double, 2>()
        & testVArithmetic<double, 3>()
        & testVArithmetic<double, 4>()
        & test1Arithmetic<half>()
        & testVArithmetic<half, 2>()
        & testVArithmetic<half, 3>()
        & testVArithmetic<half, 4>()
        & test1Arithmetic<int>()
        & testVArithmetic<int, 2>()
        & testVArithmetic<int, 3>()
        & testVArithmetic<int, 4>()
        & test1Arithmetic<int8_t>()
        & testVArithmetic<int8_t, 2>()
        & testVArithmetic<int8_t, 3>()
        & testVArithmetic<int8_t, 4>()
        & test1Arithmetic<int16_t>()
        & testVArithmetic<int16_t, 2>()
        & testVArithmetic<int16_t, 3>()
        & testVArithmetic<int16_t, 4>()
        & test1Arithmetic<int64_t>()
        & testVArithmetic<int64_t, 2>()
        & testVArithmetic<int64_t, 3>()
        & testVArithmetic<int64_t, 4>()
        & test1Arithmetic<uint>()
        & testVArithmetic<uint, 2>()
        & testVArithmetic<uint, 3>()
        & testVArithmetic<uint, 4>()
        & test1Arithmetic<uint8_t>()
        & testVArithmetic<uint8_t, 2>()
        & testVArithmetic<uint8_t, 3>()
        & testVArithmetic<uint8_t, 4>()
        & test1Arithmetic<uint16_t>()
        & testVArithmetic<uint16_t, 2>()
        & testVArithmetic<uint16_t, 3>()
        & testVArithmetic<uint16_t, 4>()
        & test1Arithmetic<uint64_t>()
        & testVArithmetic<uint64_t, 2>()
        & testVArithmetic<uint64_t, 3>()
        & testVArithmetic<uint64_t, 4>()
        ;
}

[numthreads(32, 1, 1)]
void computeMain()
{
    outputBuffer[0] = uint(true & testLogical());
    outputBuffer[1] = uint(true & testArithmetic());

    // CHECK: 1
    // CHECK-NEXT: 1
}
