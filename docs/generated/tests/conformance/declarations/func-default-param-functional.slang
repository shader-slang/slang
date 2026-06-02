//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#parameters
//META: doc_section_digest=00c2961b7ec63f08cdde68ad692eff760da88176385d08086919fbf37306beee
//META: purpose=Parameters with default values use the default when the caller omits the argument, and the supplied value when provided.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "Parameters may be given a _default value_ by including an
// initial-value-expression clause."

//TEST:INTERPRET(filecheck=CHECK):

func add(x: int, y: float = 1.0f) -> float { return float(x) + y; }

void main()
{
    // CHECK: 6
    // CHECK: 7
    printf("%g\n", add(5));
    printf("%g\n", add(5, 2.0));
}
