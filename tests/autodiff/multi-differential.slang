//TEST_IGNORE_FILE:
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;
typedef float.Differential dfloat;

struct WrappedMultiDifferential : IDifferentiable
{
    float.Differential dx1;
    float.Differential dx2;
    float.Differential dx3;

    __init() { dx1 = 0; dx2 = 0; dx3 = 0; }
    __init(float.Differential _dx1, float.Differential _dx2, float.Differential _dx3)
    {
        dx1 = _dx1;
        dx2 = _dx2;
        dx3 = _dx3;
    }

};

struct WrappedFloat : IDifferentiable
{
    typealias Differential = WrappedMultiDifferential;

    [DerivativeMember(Differential.dx1)]
    float x;

    __init(float _x)
    {
        this.x = _x;
    }

    static Differential dzero() { return WrappedMultiDifferential(); }

    static Differential dadd(Differential a, Differential b)
    {
        Differential result;
        result.dx1 = a.dx1 + b.dx1;
        result.dx2 = a.dx2 + b.dx2;
        result.dx3 = a.dx3 + b.dx3;
        return result;
    }

    static Differential dmul(WrappedFloat a, Differential b)
    {
        Differential result;
        result.dx1 = a.x * b.dx1;
        result.dx2 = a.x * b.dx2;
        result.dx3 = a.x * b.dx3;
        return result;
    }

    // Operator overload for *
    [ForwardDerivative(fwd_mul)]
    static WrappedFloat operator*(WrappedFloat a, WrappedFloat b)
    {
        return WrappedFloat(a.x * b.x);
    }

    // Fwd-diff of operator overload for *
    static DifferentialPair<WrappedFloat> fwd_mul(DifferentialPair<WrappedFloat> a, DifferentialPair<WrappedFloat> b)
    {
        return DifferentialPair<WrappedFloat>(
            WrappedFloat(a.x * b.x),
            WrappedMultiDifferential(
                a.dx1 * b.x + a.x * b.dx1,
                a.dx2 * b.x + a.x * b.dx2,
                a.dx3 * b.x + a.x * b.dx3));
    }
};

[ForwardDifferentiable]
WrappedFloat f(WrappedFloat x)
{
    return x * x;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    {
        DifferentialPair<WrappedFloat> dpa = diffPair(WrappedFloat(1.0), WrappedMultiDifferential(1.0, 2.0, 3.0));

        var d = __fwd_diff(f)(dpa).d;

        outputBuffer[0] = d.dx1; // Expect: 1
        outputBuffer[1] = d.dx2; // Expect: 2
        outputBuffer[2] = d.dx3; // Expect: 3
    }
}
