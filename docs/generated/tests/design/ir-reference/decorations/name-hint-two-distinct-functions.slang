//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#namehint--namehintdecoration
//META: doc_section_digest=398fede0e4de09435e2c385a07c84980390fc7f07aeaeb82a28ad54acb9e5215
//META: purpose=Two distinct user-named functions each carry their own nameHint decoration with the matching source identifier as the string operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `nameHint` is per-inst and exact: two helpers in one module
// keep their own identifiers rather than sharing or losing one. The
// CHECK-DAG lines assert both `"alpha"` and `"beta"` are present, which
// would fail if lowering reused a single name-hint inst or dropped the
// second one.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

int alpha(int x) { return x + 1; }
int beta(int x)  { return x * 2; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = alpha(int(tid.x)) + beta(int(tid.x));
}

// CHECK-DAG: [nameHint("alpha")]
// CHECK-DAG: [nameHint("beta")]
