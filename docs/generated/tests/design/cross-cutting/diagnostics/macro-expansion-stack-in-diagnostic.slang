//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=d4c1621bec12a7398cb1a9674411d74b7313361b2b6d60447dbaae79eb52f58e
//META: purpose=A diagnostic triggered inside a macro expansion renders both the use site and the underlying error message.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says macro-expanded locations are rendered so that errors
// triggered inside a macro point at both the use site and the macro
// body. Expand a macro that references an undeclared identifier and
// verify the undefined-identifier diagnostic still surfaces.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

#define USE_UNDECLARED nonexistentVar

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = USE_UNDECLARED;
}

// CHECK: undefined identifier
// CHECK: nonexistentVar
