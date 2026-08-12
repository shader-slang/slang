//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=f0771e16a4cd3fa366077436469b559fb6852160c25c69aae5f00e8413738f6b
//META: purpose=The parser maps a floating-point literal's classification onto the literal expression's suffix type, so `h` yields half, `f` yields float, `lf` yields double, and an unsuffixed literal yields float.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the classification-to-suffix-type mapping without inspecting the
// AST, by using overload resolution as the probe: three overloads of `probe`
// differ only in their floating-point parameter type, so the one selected
// reports which type the literal expression carried. The unsuffixed literal is
// included because the mapping's default arm is a separate decision from the
// three suffixed arms.

//TEST:INTERPRET(filecheck=CHECK):

int probe(half x) { return 1; }
int probe(float x) { return 2; }
int probe(double x) { return 3; }

void main()
{
    // CHECK: h=1 f=2 lf=3 none=2
    printf(
        "h=%d f=%d lf=%d none=%d\n",
        probe(1.5h),
        probe(1.5f),
        probe(1.5lf),
        probe(1.5));
}
