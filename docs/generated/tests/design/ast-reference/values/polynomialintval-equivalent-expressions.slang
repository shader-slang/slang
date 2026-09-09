//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#polynomialintval-and-polynomial-canonicalization
//META: doc_section_digest=17fa4894f1273e7614f2fca10ae7e0fd30e8f753b02c39a565511130eb1f2cfc
//META: purpose=Polynomial canonicalization makes 2*N + 3 and 3 + 2*N the same compile-time value, which is what gives dependent array types over those expressions a single identity.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies polynomial canonicalization at the type-equality surface.
// The two parameters are spelled with the terms in opposite order, and
// a local declared with the first spelling is initialized from the
// argument declared with the other; that assignment is checked before
// substitution, so it succeeds only if both spellings canonicalize to
// one value. CHECK asserts the combined element sum.

//TEST:INTERPRET(filecheck=CHECK):

int sumBoth<let N : int>(int a[2*N+3], int b[3+2*N])
{
    // The two parameters have the same array type because the
    // index polynomials are canonicalized to a common form.
    int c[2*N+3] = a;
    int total = 0;
    for (int i = 0; i < 2*N+3; ++i) { total += c[i] + b[i]; }
    return total;
}

void main()
{
    // With N = 1 the array length is 5.
    int aa[5] = { 1, 2, 3, 4, 5 };
    int bb[5] = { 10, 20, 30, 40, 50 };
    // 1+2+3+4+5 + 10+20+30+40+50 = 15 + 150 = 165
    // CHECK: total=165
    printf("total=%d\n", sumBoth<1>(aa, bb));
}
