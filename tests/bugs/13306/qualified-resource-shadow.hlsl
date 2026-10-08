//TEST:SIMPLE_EX(filecheck=PRESERVE): tests/bugs/13306/qualified-resource-shadow.hlsl -Gec -target hlsl -entry main
//TEST:SIMPLE_EX(filecheck=REBIND): tests/bugs/13306/qualified-resource-shadow.hlsl -Gec -target hlsl -entry main -no-codegen -DTRY_REBIND

// A memory qualifier constrains operations on the resource supplied by the application. `-Gec`
// must therefore keep the parameter as an immutable alias instead of copying the resource value
// into an unqualified mutable shadow.
globallycoherent RWTexture2D<uint> input;
RWTexture2D<uint> replacement;
RWStructuredBuffer<uint> output;

// PRESERVE: globallycoherent
// PRESERVE-NEXT: RWTexture2D

// REBIND: error[E30011]: left of '=' is not an l-value
// REBIND-NOT: error[

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
#ifdef TRY_REBIND
    input = replacement;
#endif
    output[0] = input[uint2(0, 0)];
}
