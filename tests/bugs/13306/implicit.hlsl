//TEST:SIMPLE_EX(filecheck=CHECK): tests/bugs/13306/implicit.hlsl -Gec -target hlsl -entry main

// Bare numeric parameters also become mutable copies. Diagnostics must name the original
// parameter rather than its generated lookup name.
uint x;
cbuffer CB { uint y; };
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    x += tid.x;
    y++;
    output[tid.x] = x + y;
}

// CHECK: result code = 0
// CHECK: warning[E39019]
// CHECK: 'x' is implicitly a global shader parameter
// CHECK-NOT: SLANG_uniformParameter
