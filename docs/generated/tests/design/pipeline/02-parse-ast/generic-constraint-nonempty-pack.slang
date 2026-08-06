//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=2adc733a81916b42380544b008460df5b366206d39b999775293a73fe57f1756
//META: purpose=The `where nonempty(Pack)` spelling parses into a pack constraint and a call supplying a non-empty pack satisfies it.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nonempty(Pack)` where-clause spelling. A variadic
// generic constrained with `where nonempty(T)` is specialized with a
// two-element pack; the printed sum proves the constraint parsed,
// was satisfied, and the specialization ran.

//TEST:INTERPRET(filecheck=CHECK):

int sumAll<each T>(expand each T args) where nonempty(T) where T == int
{
    int total = 0;
    expand(total = total + (each args));
    return total;
}

void main()
{
    // CHECK: sum=30
    printf("sum=%d\n", sumAll(10, 20));
}
