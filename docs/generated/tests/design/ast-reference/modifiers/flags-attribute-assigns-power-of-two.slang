//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops--branches--opt-levels
//META: doc_section_digest=2884135594e88fad2ff6c4fdc20d6bf694888dfaa9f666f2ac51e906fc88f826
//META: purpose=`[Flags]` on an enum gives successive cases consecutive power-of-two values instead of consecutive integers.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `[Flags]` changes how implicit enum case values are assigned, which
// is what makes the cases usable as a bit set. `Read` / `Write` / `Exec` have no
// explicit initialisers, and CHECK asserts they are 1, 2, 4 -- a plain enum would
// have numbered them 1, 2, 3.

//TEST:INTERPRET(filecheck=CHECK):

[Flags]
enum Perm
{
    None = 0,
    Read,
    Write,
    Exec,
}

void main()
{
    //CHECK: r=1 w=2 e=4
    printf("r=%d w=%d e=%d\n", int(Perm.Read), int(Perm.Write), int(Perm.Exec));
}
