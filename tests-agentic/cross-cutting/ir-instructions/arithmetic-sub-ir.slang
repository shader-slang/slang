//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions
//META: doc_section_digest=ddd065b774b7979630d27ec19315b658865b6fb853c55bd6e6ee0a0aced77aba
//META: purpose=An integer subtraction expression lowers to the `sub` IR opcode and emits as `-` on every C-like text-emit target.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `uniform` integer globals are subtracted and stored. The IR
// dump contains `sub(%a, %b)`; every C-like target emits the same
// arithmetic as `a - b`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main

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
