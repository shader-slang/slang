//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6 -entry main_cs -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_10 -entry main_cs -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6 -entry main_cs -stage compute -validate-ir -DNOEXTRA
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -profile sm_6_10 -entry main_cs -stage compute -validate-ir -DNOEXTRA

// An empty pointer differential nested beside resource fields can become empty only after
// resource legalization and void cleanup. The final empty-type legalization must not leave
// void placeholders in the enclosing differential-pair constructor. The extra uint keeps the
// primal side nonempty; NOEXTRA also checks the case where the whole ordinary pair disappears.
// CHECK: result code = 0

RWStructuredBuffer<float> gOut;
RWStructuredBuffer<float> gParam;
RWStructuredBuffer<float> gGrad;

interface IMod : IDifferentiablePtrType
{
    [Differentiable]
    float forward(float x);

    This.Differential getDifferential();
}

struct BufDiff : IDifferentiablePtrType
{
    typealias Differential = BufDiff;
    RWStructuredBuffer<float> data;
}

struct Buf : IDifferentiablePtrType
{
    typealias Differential = BufDiff;
    RWStructuredBuffer<float> data;
}

[BackwardDerivative(scaleOp_bwd)]
float scaleOp(float x, Buf b)
{
    return x * b.data[0];
}

void scaleOp_bwd(inout DifferentialPair<float> x, DifferentialPtrPair<Buf> b, float d)
{
    b.d.data[0] += d * x.p;
    x = diffPair(x.p, d * b.p.data[0]);
}

// Parameter-less module: no fields, its own (empty) differential.
struct Encode : IMod
{
    typealias Differential = Encode;
    Differential getDifferential()
    {
        return this;
    }

    [Differentiable]
    float forward(float x)
    {
        return 2.0 * x;
    }
}

struct ScaleDiff : IDifferentiablePtrType
{
    typealias Differential = ScaleDiff;
    BufDiff b;
}

struct Scale : IMod
{
    typealias Differential = ScaleDiff;

    [DerivativeMember(ScaleDiff.b)]
    Buf b;
    no_diff BufDiff grad;
#ifndef NOEXTRA
    uint extra;
#endif

    Differential getDifferential()
    {
        return { grad };
    }

    [Differentiable]
    float forward(float x)
    {
        return scaleOp(x, b);
    }
}

struct ChainDiff<First : IDifferentiablePtrType, Second : IDifferentiablePtrType>
    : IDifferentiablePtrType
{
    typealias Differential = ChainDiff<First, Second>;
    First.Differential first;
    Second.Differential second;
}

struct Chain<First : IMod, Second : IMod> : IMod
{
    typealias Differential = ChainDiff<First, Second>;

    [DerivativeMember(ChainDiff<First, Second>.first)]
    First first;
    [DerivativeMember(ChainDiff<First, Second>.second)]
    Second second;

    Differential getDifferential()
    {
        return { first.getDifferential(), second.getDifferential() };
    }

    [Differentiable]
    float forward(float x)
    {
        return second.forward(first.forward(x));
    }
}

[Differentiable]
float Eval<M : IMod>(M m, no_diff float x)
{
    return m.forward(x);
}

void Train<M : IMod>(M m, float x, float d)
{
    bwd_diff(Eval)(DifferentialPtrPair<M>(m, m.getDifferential()), x, d);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main_cs(uint3 tid : SV_DispatchThreadID)
{
    Chain<Encode, Scale> c;
    c.second.b.data = gParam;
    c.second.grad.data = gGrad;
    Train(c, 0.5, 1.0);
    gOut[tid.x] = Eval(c, 0.5);
}
