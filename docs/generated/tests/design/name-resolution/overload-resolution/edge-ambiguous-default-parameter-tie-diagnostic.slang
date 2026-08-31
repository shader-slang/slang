//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=7aaa4b55b90bf9dc8832d907095aad278bba32f9533b97e07d8e09e58134c060
//META: purpose=Default-parameter counts are not consulted by the specificity step, so an overload pair that differs only by a trailing defaulted parameter ties and is reported ambiguous.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit carve-out that variadicness and default-parameter
// counts are not tie-breakers. Both `pick` overloads accept one int at identity
// cost and both have specialized-parameter count zero, so nothing separates
// them. CHECK pins the ambiguous call error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int pick(int a)             { return 1; }
int pick(int a, int b = 99) { return 2; }

void main()
{
    int tag = pick(5);
//CHECK:          ^ ambiguous call to 'pick'
    printf("%d\n", tag);
}
