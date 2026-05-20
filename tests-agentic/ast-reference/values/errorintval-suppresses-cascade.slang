//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ast-reference/values.md#intval-family
//META: doc_section_digest=76a84a059e7493d5ffee00d494fa33d266abc65cffaa634a39c7c87cf041e48b
//META: purpose=An ErrorIntVal is the integer-value placeholder used when a compile-time integer cannot be computed; checking continues without cascading further integer-value diagnostics.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc: `ErrorIntVal` is an "error placeholder; lets checking
// continue when an integer value cannot be computed." When an
// array size expression refers to an undeclared identifier the
// integer-value computation fails; the checker still emits a
// single root-cause diagnostic about the undeclared name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int a[undeclaredN];
//CHECK:  ^^^^^^^^^^^ undefined identifier 'undeclaredN'.
}
