//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=An integer addition expression lowers to the `add` IR opcode and emits as `+` on every C-like text-emit target.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `uniform` integer globals are summed and stored into a buffer.
// In the IR dump the addition appears as `add(%a, %b)` because the
// operands are not compile-time constants. The same addition emits as
// `a + b` on the text targets that lower IR `add` to the C-like `+`
// operator.

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
    buf[0] = a + b;
}

// IR-LABEL: func %main
// IR: add(%{{.*}}, %{{.*}})

// HLSL: a_{{[0-9]+}} + {{.*}}b_
// GLSL: a_{{[0-9]+}} + {{.*}}b_
// METAL: a_{{[0-9]+}} + {{.*}}b_
// WGSL: a_{{[0-9]+}} + {{.*}}b_
// CUDA: {{[_A-Za-z][_A-Za-z0-9]*}} + {{[_A-Za-z][_A-Za-z0-9]*}}
// CPP: {{[_A-Za-z][_A-Za-z0-9]*}} + {{.*}}b_
