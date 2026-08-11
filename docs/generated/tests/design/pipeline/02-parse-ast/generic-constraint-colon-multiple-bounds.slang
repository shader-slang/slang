//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
//META: purpose=The `:` spelling of a where-clause bound accepts a comma-separated list and produces one constraint decl per bound, so a single `where T : IA, IB` grants both interfaces.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the comma list after `where T :` is split into separate bounds
// rather than parsed as one compound bound. The generic body calls a method
// from each of the two interfaces; both calls only resolve if two independent
// constraint decls were produced. The printed 17 = 10 + 7 shows both witnesses
// were available inside the body.

//TEST:INTERPRET(filecheck=CHECK):

interface IHasA { int a(); }
interface IHasB { int b(); }

struct Both : IHasA, IHasB
{
    int a() { return 10; }
    int b() { return 7; }
}

int sumBoth<T>(T v) where T : IHasA, IHasB
{
    return v.a() + v.b();
}

void main()
{
    Both v;
    // CHECK: sum=17
    printf("sum=%d\n", sumBoth(v));
}
