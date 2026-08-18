//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
//META: purpose=The modern function spelling `func name(...) -> T` produces the same callable declaration as the C-style spelling.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `func` keyword with a trailing arrow return type declares
// an ordinary function. The function is called like any other and the CHECK
// asserts its result, so a spelling that failed to bind the return type would
// not type-check.

//TEST:INTERPRET(filecheck=CHECK):

func triple(int x) -> int { return x * 3; }

void main()
{
    //CHECK: 33
    printf("%d\n", triple(11));
}
