// `-layout-rules-version 202c` takes effect only when SPIR-V is emitted directly. Other targets
// accept the option without a diagnostic and keep the unrounded layout, so the pointee of `Q*`
// still has `Q.v` at offset 16 rather than 20.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -layout-rules-version 202c
//TEST:REFLECTION(filecheck=CHECK):-target cuda -stage compute -entry computeMain -layout-rules-version 202c

struct S
{
    double x;
    int y;
};

struct Q
{
    S z;
    float w;
    float3 v;
};

struct Params
{
    Q* pQ;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    output[0] = asuint(params.pQ.v.x);
}

// CHECK-NOT: E00023
// CHECK: "name": "params"
// CHECK: "name": "v"
// CHECK: "binding": {"kind": "uniform", "offset": 16, "size": 12
