//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#two-stage-parsing
//META: doc_section_digest=ddfe5f62e9b1084664029d4e0b1a2903b23b23769c16d58583a7289396a20747
//META: purpose=A base that resolves to a non-generic function, to a type, or to an overload group with such a candidate commits to the generic reading of `<`, because such a name can never legally precede a comparison.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the body-stage `<` test is wider than "is this base a generic".
// All three lines have the shape `base < a > (b)`, which would parse as two
// chained comparisons if the base were an ordinary variable — that reading is
// what the sibling comparison test observes. Here each base is instead a
// function, a type, and an overload group, and each commits to the generic
// reading and is then rejected for not naming a generic. The parenthesised
// diagnostic text is the discriminator: it reports what the base actually
// resolved to, so it shows the checker was consulted and its answer is what
// selected the reading.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f(int x) { return x; }
struct Ty { int q; }
int h(int x) { return x; }
int h(float x) { return int(x); }

void main()
{
    int a = 1, b = 2;

    bool r1 = f < a > (b);
//CHECK:      ^ error E39999
//CHECK:      ^ expected a generic when using '<...>' (found: '(int) -> int')

    bool r2 = Ty < a > (b);
//CHECK:      ^^ error E39999
//CHECK:      ^^ expected a generic when using '<...>' (found: 'typeof(Ty)')

    bool r3 = h < a > (b);
//CHECK:      ^ error E39999
//CHECK:      ^ expected a generic when using '<...>' (found: 'overload group')

    printf("r=%d\n", int(r1) + int(r2) + int(r3));
}
