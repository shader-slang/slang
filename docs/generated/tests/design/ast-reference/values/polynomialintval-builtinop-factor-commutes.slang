//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:25Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/ast-reference/values.md#polynomial-helpers
//META: doc_section_digest=1da990ba8dd94f18220add0daa7dee4cb3febec1de73e1e7c2ea3ef1a4ab9eb7
//META: purpose=A PolynomialIntValFactor's parameter is a general IntVal, not just a generic parameter: sums built over the symbolic division N / 2 canonicalize, so the commuted bounds N / 2 + 1 and 1 + N / 2 name the same array type.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that polynomial canonicalization also works when a factor's
// base value is itself a symbolic builtin operation (the division
// N / 2) rather than a bare generic parameter. The two parameters use
// commuted spellings of the same sum; assigning one array into a local
// typed like the other checks only if both canonicalize to one type,
// and CHECK asserts the combined element sum.

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
