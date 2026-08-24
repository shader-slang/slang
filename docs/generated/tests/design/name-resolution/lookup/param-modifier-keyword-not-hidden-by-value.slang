//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#concepts
//META: doc_section_digest=66115897ba3bf509762cdb486bd39ade9574d8d8ea06eb5213c6d9a683af6562
//META: purpose=A parameter's modifier list is read with a keyword-only mask, so a same-named value declaration does not stop the keyword from being recognised there while the identifier still names the value in an expression.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the keyword-category bit the parser asks for on its own.
// `nointerpolation` is both a keyword-introducing declaration in the
// core module and, here, a user constant. At the head of a parameter
// declaration only the keyword category is requested, so the parser
// still reads the token as a modifier — if the user constant had been
// visible there instead, `nointerpolation float x` would be two type
// names in a row and would not parse. In the function body the same
// spelling is looked up with the ordinary mask and finds the constant.
// CHECK asserts 7, which is 2 from the argument plus 5 from the
// constant, so both resolutions happened in the same file.

//TEST:INTERPRET(filecheck=CHECK):

static const int nointerpolation = 5;

int f(nointerpolation float x)
{
    return int(x) + nointerpolation;
}

void main()
{
    // CHECK: r=7
    printf("r=%d\n", f(2.0f));
}
