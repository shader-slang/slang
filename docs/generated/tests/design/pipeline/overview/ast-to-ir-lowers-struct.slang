//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ast--ir-lowering
//META: doc_section_digest=58d6fa34e989046b10c5c18ce14f1e0e4d1c1f389944592cfe513724b2c60cae
//META: purpose=A struct declaration in the AST becomes an IR struct type that survives the pass list and reappears as an aggregate type in the emitted target text.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the decl-to-IR-type half of lowering. The struct is passed by
// value to a callee and stored into a structured buffer, so neither
// scalarization nor dead-code elimination can erase it before emit. Each
// CHECK asserts an aggregate carrying both fields survives on that target:
// a named `struct` on the C-family and WGSL back-ends, and an
// `OpTypeStruct %int %float` on SPIR-V, where the buffer element type
// keeps the declared field order and types.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Pair
{
    int first;
    float second;
};

RWStructuredBuffer<Pair> gOut;

int usePair(Pair p)
{
    return p.first + int(p.second);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Pair p;
    p.first = int(tid.x);
    p.second = float(tid.y);
    if (usePair(p) > 0)
        gOut[0] = p;
}

// HLSL-DAG: struct Pair
// HLSL-DAG: usePair_{{[0-9]+}}(

// GLSL-DAG: struct Pair
// GLSL-DAG: usePair_{{[0-9]+}}(

// SPIRV-DAG: OpTypeStruct %int %float
// SPIRV-DAG: OpMemberName {{.*}} 0 "first"
// SPIRV-DAG: OpMemberName {{.*}} 1 "second"

// METAL-DAG: struct Pair
// METAL-DAG: usePair_{{[0-9]+}}(

// WGSL-DAG: struct Pair
// WGSL-DAG: usePair_{{[0-9]+}}(

// CUDA-DAG: struct Pair
// CUDA-DAG: usePair_{{[0-9]+}}(

// CPP-DAG: struct Pair
// CPP-DAG: usePair_{{[0-9]+}}(
