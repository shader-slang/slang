//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#punctuation-and-structural-symbols
//META: doc_section_digest=ec873659d95b0f115041298559c29781da8ac778d588e899bee7e084b8ff56cb
//META: purpose=The three-character spelling `...` lexes as a single `Ellipsis` token, which the preprocessor consumes as a variadic macro parameter list.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `...` is one token and not `Dot`/`DotDot` fragments. The ellipsis is
// only meaningful to the preprocessor as the whole variadic-parameter spelling,
// so expanding a macro that forwards `__VA_ARGS__` to a three-argument function
// and printing the sum proves the parameter list was recognised.

//TEST:INTERPRET(filecheck=CHECK):

#define FORWARD_ALL(...) addThree(__VA_ARGS__)

int addThree(int a, int b, int c) { return a + b + c; }

void main()
{
    // CHECK: ellipsis=6
    printf("ellipsis=%d\n", FORWARD_ALL(1, 2, 3));
}
