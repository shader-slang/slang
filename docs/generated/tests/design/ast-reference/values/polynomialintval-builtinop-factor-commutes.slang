//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#polynomial-helpers
//META: doc_section_digest=b5eb05716e03d485ee334da2f955b3860ecb75a00316fb77c3e6d6a07ddaf801
//META: purpose=A polynomial factor's parameter is a general compile-time integer rather than only a generic parameter, so sums built over the symbolic division N / 2 still canonicalize across commuted spellings.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies canonicalization at the boundary where a factor's base value
// is itself a symbolic builtin operation rather than a bare generic
// parameter. The two parameters use commuted spellings of the same sum
// over (N / 2); assigning one array into a local typed like the other
// checks only if both canonicalize to one type, and CHECK asserts the
// combined element sum.

//TEST:INTERPRET(filecheck=CHECK):

int sumBothHalves<let N : int>(int a[N / 2 + 1], int b[1 + N / 2])
{
    // Same canonical polynomial over the factor (N / 2), so the
    // assignment is exact.
    int c[N / 2 + 1] = a;
    int total = 0;
    for (int i = 0; i < N / 2 + 1; ++i) { total += c[i] + b[i]; }
    return total;
}

void main()
{
    // With N = 6 both bounds are 6 / 2 + 1 == 4.
    int aa[4] = { 1, 2, 3, 4 };
    int bb[4] = { 10, 20, 30, 40 };
    // 1+2+3+4 + 10+20+30+40 = 110
    // CHECK: total=110
    printf("total=%d\n", sumBothHalves<6>(aa, bb));
}
