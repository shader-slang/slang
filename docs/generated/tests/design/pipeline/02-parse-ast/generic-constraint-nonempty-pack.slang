//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#generics-ambiguity
//META: doc_section_digest=078844f9de8080a010d3c70d4af44f40667b8ce158b13e5adda01e318abce99c
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
