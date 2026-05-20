//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials
//META: doc_section_digest=96937cffa7eab5a700d8fe30b96ad6032aaa2693bb26bdc6db27157e8ac98d94
//META: purpose=Calling a generic function with a concrete type argument lowers to a `specialize` IR opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickFirst<int>(a, a + 1)` lowers to a `call specialize(%generic,
// Int)(%a, ...)` in the IR — the doc's "Applies generic arguments;
// hoistable" claim for the `specialize` opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = pickFirst(a, a + 1);
}

// IR-LABEL: func %main
// IR: specialize({{.*}})
