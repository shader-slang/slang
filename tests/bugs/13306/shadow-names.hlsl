//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -Gec -entry main -stage compute -target hlsl
//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -Gec -entry main -stage compute -target spirv
//TEST:COMPILE: tests/bugs/13306/shadow-names.hlsl -entry main -stage compute -target hlsl

// Both names are valid user identifiers, including `SLANG_uniformParameter_x`.
// The parser's internal `$uniformParameter_x` name cannot collide with either declaration.
uniform uint x;
uniform uint SLANG_uniformParameter_x;
RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = x + SLANG_uniformParameter_x;
}
