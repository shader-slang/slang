//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -Gec -entry main -stage compute -target hlsl
//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -Gec -entry main -stage compute -target spirv
//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -entry main -stage compute -target hlsl

// Both names are valid user identifiers. Renaming the underlying parameter for `x` must
// not introduce a declaration that conflicts with the second parameter's shadow.
uniform uint x;
uniform uint SLANG_uniformParameter_x;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = x + SLANG_uniformParameter_x;
}
