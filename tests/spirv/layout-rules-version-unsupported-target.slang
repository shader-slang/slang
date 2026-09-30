//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-via-glsl -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c -skip-codegen
//TEST:REFLECTION(filecheck=REFL):-target glsl -stage compute -entry computeMain -fvk-use-scalar-layout -layout-rules-version 202c

// Layout rules version 202c rounds scalar-layout struct sizes up to their alignment. Slang can only
// honor that when it emits SPIR-V directly; for GLSL output, glslang assigns the offsets of a
// `layout(scalar)` block without the rounding. Generating GLSL (directly, or as the intermediate
// for SPIR-V) with 202c must therefore be rejected rather than silently produce a layout that
// disagrees with reflection and with the requested rules. The rejection happens when the layout
// is computed, so it also applies when only reflection is requested (`-skip-codegen`, or the
// `getLayout()` call that the REFLECTION test makes).

struct S
{
    double x;
    int y;
};

struct U
{
    S z;
    float w;
};

ConstantBuffer<U> cb;
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = cb.w;
}

// The diagnostic has no source position to anchor a caret to, so we match on its code.
//CHECK: 00023
// REFL: E00023
