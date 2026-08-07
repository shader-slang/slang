//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[UnscopedEnum]` written by the user on a plain enum makes its cases resolvable without qualification.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute's scoping consequence: the enum's cases become
// visible in the enclosing scope. `Red` and `Blue` are named without the
// `Color.` prefix, and CHECK asserts each printed value matches its case, which
// proves the bare names bound to the right cases rather than merely compiling.

//TEST:INTERPRET(filecheck=CHECK):

[UnscopedEnum]
enum Color
{
    Red = 7,
    Blue = 42,
}

void main()
{
    //CHECK: red=7 blue=42
    printf("red=%d blue=%d\n", int(Red), int(Blue));
}
