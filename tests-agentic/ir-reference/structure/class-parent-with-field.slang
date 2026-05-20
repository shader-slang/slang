//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#struct-internals
//META: doc_section_digest=512fdef1e28b21588ddc2979311636aaac69bfc4ae19f152ea896102f2378207
//META: purpose=A class declaration lowers to a class parent opcode (distinct spelling from struct) owning field and key children.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `class Counter { int n; }` should emit a `class %Counter : Type`
// parent with `field(%n, Int)` as its child. The opcode spelling is
// `class`, distinct from `struct`, even though the field-and-key
// structural shape is the same.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

class Counter
{
    int n;
};

[numthreads(1, 1, 1)]
void main()
{
    Counter c = new Counter();
    c.n = 7;
    buf[0] = c.n;
}

// CHECK: class %Counter	: Type
// CHECK: field(%n, Int)
