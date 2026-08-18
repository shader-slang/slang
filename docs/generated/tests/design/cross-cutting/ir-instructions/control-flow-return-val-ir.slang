//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#control-flow-instructions
//META: doc_section_digest=c0238650e6ba499ae56294af00d41ede4ea9a7b4f8215393080c0974a1553619
//META: purpose=A value-returning function terminates its block with the `return_val` terminator.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's `return_val` terminator row. A helper function that
// returns a computed value ends its block with `return_val(%v)` in the
// IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

int addThem(int x, int y) { return x + y; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = addThem(a, b);
}

// IR: return_val(%{{.*}})
