//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=180e0aa25246196c89b6dc01c680b751b33a813ef1597e4ac18dd9fef40c0943
//META: purpose=An integer subtraction expression lowers to the `sub` IR opcode and emits as `-` on C-like text-emit targets.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `sub`/`Sub` value-instruction row: subtracting two
// non-constant operands lowers to `sub`. `uniform` inputs keep the
// operands out of the constant folder so the IR dump shows `sub(%a, %b)`,
// and the emitted text is the C-like `a - b`.

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
    buf[0] = a - b;
}

// IR-LABEL: func %main
// IR: sub(%{{.*}}, %{{.*}})

// HLSL: a_{{[0-9]+}} - {{.*}}b_
// GLSL: a_{{[0-9]+}} - {{.*}}b_
// METAL: a_{{[0-9]+}} - {{.*}}b_
// WGSL: a_{{[0-9]+}} - {{.*}}b_
// CUDA: {{[_A-Za-z][_A-Za-z0-9]*}} - {{[_A-Za-z][_A-Za-z0-9]*}}
// CPP: {{[_A-Za-z][_A-Za-z0-9]*}} - {{.*}}b_
