// Test that a struct inheriting from another struct is rejected as an
// entry-point varying on the targets where it used to crash the compiler
// (issue #12463).
//
// On SPIR-V and GLSL every varying use is rejected: inputs, outputs, and
// structs nested inside another varying struct. On Metal and WGSL only
// inputs are rejected, because outputs already compile correctly there.

//DIAGNOSTIC_TEST:SIMPLE(diag=SPV_IN,non-exhaustive):-target spirv -entry fragmentInput
//DIAGNOSTIC_TEST:SIMPLE(diag=SPV_OUT,non-exhaustive):-target spirv -entry vertexOutput
//DIAGNOSTIC_TEST:SIMPLE(diag=SPV_NESTED,non-exhaustive):-target spirv -entry nestedInput
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSL_IN,non-exhaustive):-target glsl -entry fragmentInput
//DIAGNOSTIC_TEST:SIMPLE(diag=METAL_IN,non-exhaustive):-target metal -entry fragmentInput
//DIAGNOSTIC_TEST:SIMPLE(diag=WGSL_IN,non-exhaustive):-target wgsl -entry nestedInput

struct Base
{
    float4 a : A;
};

struct Derived : Base
{
    float4 b : B;
};

struct Outer
{
    Derived d;
    float4 c : C;
};

[shader("fragment")]
float4 fragmentInput(Derived i) : SV_Target
//SPV_IN: a struct that inherits from another struct is not a valid varying type
//GLSL_IN: a struct that inherits from another struct is not a valid varying type
//METAL_IN: a struct that inherits from another struct is not a valid varying input type
{
    return i.a + i.b;
}

struct VertexOut : Base
{
    float4 pos : SV_Position;
};

[shader("vertex")]
VertexOut vertexOutput(float4 p : P)
//SPV_OUT: a struct that inherits from another struct is not a valid varying type
{
    VertexOut o;
    o.a = p;
    o.pos = p;
    return o;
}

[shader("fragment")]
float4 nestedInput(Outer i) : SV_Target
//SPV_NESTED: a struct that inherits from another struct is not a valid varying type
//WGSL_IN: a struct that inherits from another struct is not a valid varying input type
{
    return i.d.a + i.d.b + i.c;
}
