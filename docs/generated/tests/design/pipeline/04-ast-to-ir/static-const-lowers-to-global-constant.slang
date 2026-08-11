//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A module-scope `static const int N = 42;` lowers to a `globalConstant(42 : Int)` IR value bound to a named global; reads of `N` in `main` reference the named global, not the inline literal.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: static-const folding edge — the symbol survives
// lowering as `%N` whose definition is `globalConstant(42 : Int)`,
// rather than being inlined as a raw `42 : Int` operand. Verifies
// both the global definition and the named reference in `main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

static const int N = 42;

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = N;
}

// CHECK: let  %N	: Int	= globalConstant(42 : Int)
// CHECK-LABEL: func %main
// CHECK: store({{.*}}, %N)
