//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions
//META: doc_section_digest=784218f4c390fad67e31ccc3c7b8d48cc6fcafc60da93f9e66b0de850b7bb63b
//META: purpose=A local `var` produces a value whose IR type is `Ptr<T>`, as the doc states for the memory family.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `var` row reads "Local variable allocation; result is
// `Ptr<T>`." A local struct variable that gets written through a
// field access keeps the `var` form: the IR shows
// `let %f : Ptr(%Foo) = var`. The doc's `Ptr` type-row claim
// (operand: `valueType, ...`) is observable on the same line.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Foo { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Foo f;
    f.x = a;
    f.y = a + 1;
    buf[0] = f.x + f.y;
}

// IR-LABEL: func %main
// IR: %f : Ptr(%Foo) = var
