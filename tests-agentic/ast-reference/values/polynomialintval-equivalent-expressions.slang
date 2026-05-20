//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ast-reference/values.md#polynomialintval-and-polynomial-canonicalization
//META: doc_section_digest=fa04f2d0ce71e5d44afb3c1b38b9a31924509865d3e9e1e05624b53bc4402263
//META: purpose=A PolynomialIntVal stores a polynomial in unsubstituted parameters in a canonical form; type-equality for dependent array types treats commuted or re-associated expressions of the same polynomial as the same type.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc: "equations like `2*N + 3 == 3 + 2*N` resolve to the same
// hash-consed PolynomialIntVal*, which is essential for type
// equality on dependent array types." Here `T[2*N+3]` and
// `T[3+2*N]` are the same type and a value of one is assignable
// to the other inside a generic.

//TEST:INTERPRET(filecheck=CHECK):

int sumBoth<let N : int>(int a[2*N+3], int b[3+2*N])
{
    // The two parameters have the same array type because the
    // index polynomials are canonicalized to a common form. We
    // can assign one into the other and read it back.
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
