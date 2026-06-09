//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=9c56cd15864837ba13d0f3ca512f679e11adc378d62d03ad3125087b47fe77e6
//META: purpose=Boundary: the sink's severity filter upgrades a warning (41024) to error severity when -warnings-as-errors selects it by id.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the sink owns a Severity filter that can suppress,
// downgrade, or upgrade diagnostics by id. Verify the rendered
// severity word for the comma-operator warning (41024) becomes
// "error" when `-warnings-as-errors 41024` is supplied.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -warnings-as-errors 41024
int main()
{
    int x = (1, 2);
//CHECK:      ^ error
//CHECK:      ^ comma operator used in expression
// CHECK: W41024
    return x;
}
