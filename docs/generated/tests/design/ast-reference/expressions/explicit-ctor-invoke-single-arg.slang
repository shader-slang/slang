//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that a one-argument `T(x)` invokes the user-declared constructor instead of being re-read as a coercion of `x` to `T`.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The one-argument form is the ambiguous case: `T(x)` could be a constructor
// call or a conversion. The constructor here multiplies its argument, so the
// two readings give different answers. The CHECK asserts the multiplied value,
// which is only produced if the constructor ran rather than a coercion.

//TEST:INTERPRET(filecheck=CHECK):

struct Scale
{
    int k;
    __init(int v) { k = v * 3; }
}

void main()
{
    Scale s = Scale(4);
    // CHECK: k=12
    printf("k=%d\n", s.k);
}
