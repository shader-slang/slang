//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#requiredloweringpassset-flags
//META: doc_section_digest=c3103ee2a38af2bf635bdd9987e457e5b1fbf9bc975f679755d345340e97d6dc
//META: purpose=An `Optional<T>` in the module sets the `optionalType` gate that selects `lowerOptionalType`, so no optional type reaches HLSL emit: it becomes a plain struct pairing the payload with a `bool` flag, and `none` becomes a brace-list with that flag false.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `optionalType` gate's pass has actually run by
// the time HLSL text is produced. HLSL has no optional type, so the
// only observable form is the lowered struct: the CHECKs pin its two
// members and both constructed values — the `true` arm carrying the
// payload and the `none` arm carrying a default payload with the
// flag cleared. The payload is derived from the dispatch id so the
// branch is not folded away before lowering.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

Optional<int> pick(int v)
{
    if (v > 0)
        return v;
    return none;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> o = pick(int(tid.x));
    outBuf[tid.x] = o.hasValue ? o.value : -1;
}

// CHECK: struct _slang_Optional_int_{{[0-9]+}}
// CHECK: int value_{{[0-9]+}};
// CHECK: bool hasValue_{{[0-9]+}};
// CHECK-DAG: = { v_{{[0-9]+}}, true };
// CHECK-DAG: = { int(0), false };
