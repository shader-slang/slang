//TEST:COMPARE_COMPUTE(filecheck-buffer=CHK): -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
void bar(inout float x)
{
    x = 2 * x;
}

[Differentiable]
void zap(inout float y)
{
    bar(y);
}

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    {
        float val = 1.f;
        var bwdCallable = zap.apply_bwd(val);
        outputBuffer[tig] = bwdCallable.val(); // CHK: 2.0

        var dx = 1.f;

        bwdCallable(dx);
        outputBuffer[tig + 1] = dx; // CHK: 2.0

        dx = 2.f;
        bwdCallable(dx);
        outputBuffer[tig + 2] = dx; // CHK: 4.0
    }

    {
        DifferentialPair<float> dpx = diffPair(1.f, 1.f);
        zap.bwd_diff(dpx);
        outputBuffer[tig + 3] = dpx.d; // Expect: 2.0
    }
}