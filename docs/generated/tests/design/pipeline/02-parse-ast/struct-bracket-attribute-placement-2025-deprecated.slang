//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=c08befb0cda99dc013ab0ceb9ba3b786beb2359fadcd06a399a784137456d81d
//META: purpose=At language version 2025 a bracketed attribute list written after the `struct` keyword draws the deprecated-placement warning W31204, while an attribute written before the keyword is unaffected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the middle arm of the placement gate together with its carve-out.
// Two structs differ only in which side of the `struct` keyword the identical
// attribute sits on. The after-keyword one is annotated with the deprecation
// warning; the before-keyword one carries no annotation, and exhaustive mode is
// what turns that silence into an assertion — if the gate ever started firing
// on the canonical placement too, the unannotated diagnostic would fail the
// test rather than passing unnoticed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2025

struct [__NonCopyableType] Deprecated { int x; };
/*CHECK:
       ^ warning E31204
       ^ deprecated placement of bracketed attributes list.
*/

[__NonCopyableType] struct Canonical { int y; };

void main()
{
    Deprecated d;
    d.x = 1;
    Canonical c;
    c.y = 2;
    printf("v=%d\n", d.x + c.y);
}
