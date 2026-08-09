//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// The default value for a field of associated type used to lower as a zero-operand
// makeStruct; once specialization resolved the type to a scalar, that emitted an
// OpConstantComposite with a scalar result type, which spirv-val rejects.

interface IReal : IDifferentiable
{
}

extension float : IReal
{
}

struct P<Real : IReal> : IDifferentiable
{
    float3 pos;
    Real area;
}

[noinline]
float use<Real : IReal>(P<Real>.Differential d)
{
    return d.pos.x;
}

float get<Real : IReal>(float3 v)
{
    return use<Real>(P<Real>.Differential(v));
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = get<float>(float3(tid.x, 1, 2));
}

// CHECK: OpEntryPoint
// CHECK-NOT: OpConstantComposite %float{{$}}
