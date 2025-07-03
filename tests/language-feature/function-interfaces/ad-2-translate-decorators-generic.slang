//TEST:COMPARE_COMPUTE(filecheck-buffer=CHK): -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

T bar<T : IFloat>(T x)
{
    return (T)2 * x;
}

[ForwardDerivativeOf(bar)]
DifferentialPair<T> bar_fwd<T : IFloat>(DifferentialPair<T> dpx)
{
    return DifferentialPair<T>((T)2 * dpx.p, __slang_noop_cast<T.Differential, T>((T)3 * __slang_noop_cast<T, T.Differential>(dpx.d)));
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    outputBuffer[tig] = bar<float>.fwd_diff(DifferentialPair<float>(2.f, 1.f)).d;
}
