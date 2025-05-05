//TEST(compute):COMPARE_COMPUTE:-cpu -shaderobj

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Smoke test for function interfaces

interface IFoo
{
    static float bar(float a);
}

struct Zig : IDifferentiable
{
    float x;

    float zap(float x)
    {
        return 2 * x;
    }
};

extension __func_as_type(Zig::zap) : IDifferentiableFuncBase, IForwardDifferentiable<__func_as_type(Zig::zap)> { };

[numthreads(1, 1, 1)]
void computeMain(uint tig : SV_GroupIndex)
{
    Zig zig = {3};
    DifferentialPair<Zig> dpzig = diffPair(zig, zig);
    DifferentialPair<float> dpx = diffPair(2.0, 5.0);
    outputBuffer[tig] = zig.zap.fwd_diff(dpzig, dpx).d; // Expect: ?
}
