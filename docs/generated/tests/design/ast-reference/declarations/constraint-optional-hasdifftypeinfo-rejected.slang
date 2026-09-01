//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=Every `where` spelling except `__hasDiffTypeInfo` accepts a leading `optional`; `optional __hasDiffTypeInfo(T)` is diagnosed.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out in the `optional` rule. The same leading keyword that
// the subtype, equality, nonempty and pack-count spellings accept is applied
// to a `__hasDiffTypeInfo` constraint; the CHECKs pin the dedicated rejection,
// which is what distinguishes this spelling from the others.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

T f<T>(T x) where optional __hasDiffTypeInfo(T)
//CHECK:                   ^^^^^^^^^^^^^^^^^ optional __hasDiffTypeInfo constraint invalid
//CHECK:                   ^^^^^^^^^^^^^^^^^ 'optional' is not supported on '__hasDiffTypeInfo(...)' constraints
{
    return x;
}

void main() {}
