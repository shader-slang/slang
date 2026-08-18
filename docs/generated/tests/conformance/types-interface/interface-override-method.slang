//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=Inheriting types may override interface default-implementation member functions using the `override` modifier; the overriding implementation is called instead of the default.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `override` modifier causes a struct's method to replace
// the interface default. DefaultUser inherits compute() as base()+100=105;
// OverrideUser replaces it with base()*3=15. CHECK asserts both values,
// confirming the override takes precedence for OverrideUser only.

//TEST:INTERPRET(filecheck=CHECK):

interface IComputable
{
    int base();
    int compute() { return base() + 100; }
}

struct DefaultUser : IComputable
{
    int base() { return 5; }
}

struct OverrideUser : IComputable
{
    int base() { return 5; }
    override int compute() { return base() * 3; }
}

void main()
{
    DefaultUser du;
    OverrideUser ou;
    //CHECK: default=105 overridden=15
    printf("default=%d overridden=%d\n", du.compute(), ou.compute());
}
