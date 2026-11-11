//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#switch-and-targetswitch
//META: doc_section_digest=9ed6898dc4712155dd5c86ec8e6556e0db11da6e13cde8988194d8247ca4141f
//META: purpose=A switch on an enum : bool is legalized like a bool switch on SPIR-V, GLSL and WGSL (case keys become integers 1 and 0), while HLSL and Metal keep the case true / case false labels.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says an `enum : bool` switch reaches legalizeBoolSwitch with
// IRBoolLit keys (lowerEnumType canonicalizes them), and that only the
// GLSL/SPIRV and WGSL target arms run the pass. SPIR-V gets integer keys; HLSL
// falls through without the pass and keeps its bool labels.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute

enum B : bool
{
    No = false,
    Yes = true
}

uniform B g;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (g)
    {
    case B.Yes:
        r = 1;
        break;
    case B.No:
        r = 2;
        break;
    }
    outputBuffer[0] = r;
}

// SPV: OpSwitch %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} 1 %{{[A-Za-z0-9_]+}} 0 %{{[A-Za-z0-9_]+}}

// GLSL: switch(int(
// GLSL: case 1:
// GLSL: case 0:

// WGSL: switch(select(i32(0), i32(1),
// WGSL: case i32(1):
// WGSL: case i32(0):

// HLSL: case true:
// HLSL: case false:

// METAL: case true:
// METAL: case false:
