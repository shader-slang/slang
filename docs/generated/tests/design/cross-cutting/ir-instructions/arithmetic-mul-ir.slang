//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=31b4dd520f90edc4b02037b40da6ae302b38c57ff5b86d114951a278fb8a52e7
//META: purpose=An integer multiplication expression lowers to the `mul` IR opcode and emits as `*` on C-like text-emit targets.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `mul`/`Mul` value-instruction row. Two non-constant
// `uniform` operands are multiplied so the IR dump shows `mul(%a, %b)`
// rather than a folded constant, and the emitted text is the C-like
// `a * b`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a * b;
}

// IR-LABEL: func %main
// IR: mul(%{{.*}}, %{{.*}})

// HLSL: a_{{[0-9]+}} * {{.*}}b_
// GLSL: a_{{[0-9]+}} * {{.*}}b_
// METAL: a_{{[0-9]+}} * {{.*}}b_
// WGSL: a_{{[0-9]+}} * {{.*}}b_
// CUDA: {{[_A-Za-z][_A-Za-z0-9]*}} * {{[_A-Za-z][_A-Za-z0-9]*}}
// CPP: {{[_A-Za-z][_A-Za-z0-9]*}} * {{.*}}b_
