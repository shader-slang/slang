//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#select
//META: doc_section_digest=e40d6712a2443477b9dd87fd4146f5ac3852042d7d571c4f7aa5a1ef274fdbc1
//META: purpose=A ternary `?:` whose condition is a `vector<bool,N>` (not a `BasicExpressionType`) lowers to the branch-free `select` opcode rather than to `ifElse` plus a join-block param.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the condition-type carve-out that decides whether `?:`
// becomes `select` or a branch. A `bool3` condition cannot pick one
// side per lane with a branch, so lowering must fall through to the
// `__intrinsic_op(select)` route. CHECK asserts a single `select` whose
// three operands are the `uniform` condition vector and the two
// `Vec(Float, 3 : Int)` arms, and that no join-block `param` of that
// vector type was introduced — the branch-free property under test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool3 c;
uniform float3 x;
uniform float3 y;
uniform RWStructuredBuffer<float3> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = c ? x : y;
}

// IR-LABEL: func %main
// IR: : Vec(Float, 3 : Int)	= select(%c, %x, %y)
// IR-NOT: param %{{[A-Za-z0-9_]+}}	: Vec(Float, 3 : Int)
