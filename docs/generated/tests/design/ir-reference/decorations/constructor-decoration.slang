//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#other
//META: doc_section_digest=25e94198cfd90f64f6e36c4d960d5bb85db8178b17b1ac5e0ed392b86d21a135
//META: purpose=The synthesized struct initializer carries a constructor decoration; the dump prints constructor(true) with a Bool literal operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `constructor` marks the function lowered from an `__init`
// declaration and that its operand list is non-empty. An aggregate
// initializer `S { ... }` invokes the struct's synthesized initializer, and
// the CHECK pins the printed operand form `constructor(true)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct S { int x; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s = { int(tid.x) + 7 };
    buf[0] = s.x;
}

// CHECK: [constructor(true)]
