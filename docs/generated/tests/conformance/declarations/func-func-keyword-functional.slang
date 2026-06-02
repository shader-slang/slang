//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#functions
//META: doc_section_digest=983e24a8bd9b3f5460dd9d28ca3ec2f20be57d342d6af677787a1627b1972246
//META: purpose=The `func` keyword declares a function with `name: type` parameter syntax and `-> result-type` result clause.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Functions are declared using the `func` keyword" with
// `name: type` parameter pairs and an arrow result-type clause.

//TEST:INTERPRET(filecheck=CHECK):

func add(x: int, y: float) -> float { return float(x) + y; }

void main()
{
    // CHECK: 5
    printf("%g\n", add(3, 2.0));
}
