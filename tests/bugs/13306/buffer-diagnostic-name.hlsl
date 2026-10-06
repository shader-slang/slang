//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -no-codegen
//TEST:SIMPLE(filecheck=CHECK): -Gec -target hlsl -entry main -no-codegen

// Buffer diagnostics must name `First` and `Second`, including when no shadow is created.
cbuffer First : register(b0) { uint x; };
cbuffer Second : register(b0) { uint y; };
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    output[0] = x + y;
}

// CHECK: parameter 'Second' overlaps with parameter 'First'
// CHECK-NOT: SLANG_parameterGroup
// CHECK-NOT: SLANG_uniformParameter
