//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=d7075765bda80113ade59214eaa103fee6ef8daf55af7cdaae950e1bb4aec52e
//META: purpose=The enum class marker exists to detect an explicit UnscopedEnum on an enum class, and that combination is rejected as a conflicting modifier.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one conflicting shape the `enum class` marker is there to
// catch. The implicit attachment of the unscoped attribute only ever happens
// for an enum *not* written as `enum class`, so the only way the two can meet
// is if the user writes both — which is exactly what this test does. CHECK
// pins E31202 on the attribute token and the message naming `class` as the
// modifier it conflicts with; a compiler that simply let one spelling win
// would emit nothing here.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[UnscopedEnum]
/*CHECK:
 ^^^^^^^^^^^^ E31202
 ^^^^^^^^^^^^ modifier 'UnscopedEnum' is redundant or conflicting with existing modifier 'class'
*/
enum class Color
{
    Red,
    Green
}

void main()
{
    printf("%d\n", int(Color.Red));
}
