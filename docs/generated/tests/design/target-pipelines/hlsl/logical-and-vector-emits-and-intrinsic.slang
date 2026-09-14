//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#legalizelogicalandor
//META: doc_section_digest=73b5ceabc98acb655dd57afd93daf9f04c2ff40822ea582ae60ff23f4ba389d6
//META: purpose=A vector-result And/Or is written as the HLSL 2021 intrinsic call `and(a, b)` / `or(a, b)` rather than the infix operator, because `&&` and `||` are scalar-only in HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The second of the three operand shapes. `legalizeLogicalAndOr` casts a
// non-`bool` vector operand to `vector<bool,N>` and, when the result type is
// likewise a non-`bool` vector, rebuilds the And/Or with a `vector<bool,N>`
// result and casts back. That vector-result shape is what `tryEmitInstExprImpl`
// then writes as the intrinsic call.
//
// The form is conditional: it requires shader model 6.0 or newer *and*
// `isTargetHLSL2018`, and when either fails the emitter returns false and the
// infix operator is used after all. The directive therefore pins a profile
// that satisfies the gate, so the test is asserting the documented form rather
// than whichever default the driver happens to pick.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -profile sm_6_0

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool3 a = bool3(tid.x > 0, tid.y > 0, tid.z > 0);
    bool3 b = bool3(tid.x < 4, tid.y < 4, tid.z < 4);
    bool3 c = a && b;
    bool3 d = a || b;
    outBuf[0] = (c.x ? 1 : 0) + (d.y ? 2 : 0);
}

// A vector result is spelled as the HLSL 2021 intrinsic, not as `&&` / `||`.
// CHECK-DAG: and(
// CHECK-DAG: or(
