//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target hlsl -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target metal -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target cpp -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target hlsl -entry main -no-mangle -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -target dxil -profile cs_6_0 -entry main -validate-ir
//TEST:COMPILE: tests/bugs/13306/data.hlsl -Gec -no-codegen -o tests/bugs/13306/data.slang-module -verify-debug-serial-ir
//TEST:COMPILE: tests/bugs/13306/data.slang-module -target dxil -profile cs_6_0 -entry main -validate-ir

// We check reads along the path that does not call `replaceValues`, as well as writes to
// private copies through assignment and `inout` arguments, including legacy `cbuffer` fields.
// Reloading the saved module must retain this behavior without requiring `-Gec` again.
uniform uint x;
uniform uint values[2];
uniform uint a, b;

cbuffer Settings : register(b2)
{
    uint y;
    float4 z;
};

namespace N
{
    uniform uint n;
}

void increment(inout uint value)
{
    value++;
}

void replaceValues(uint value)
{
    x = value;
    y = value + 1;
    values[1] = value + 2;
    z.xy = float2(value, value + 3);
    increment(a);
    b = a;
    N::n = value;
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x != 0)
        replaceValues(tid.x);
    output[tid.x] = x + y + values[1] + uint(z.x) + a + b + N::n;
}
