//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=9c56cd15864837ba13d0f3ca512f679e11adc378d62d03ad3125087b47fe77e6
//META: purpose=Stress: the sink emits every diagnostic from a sequence of unrelated undefined-identifier conditions; no documented cap drops later messages.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says the sink owns a buffered list of Diagnostic records.
// Stress: trigger many distinct undefined-identifier diagnostics in
// a single translation unit; the sink must emit each one with its
// own id and span (no silent dropping).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = undef_a;
    int b = undef_b;
    int c = undef_c;
    int d = undef_d;
    int e = undef_e;
    int f = undef_f;
    int g = undef_g;
    int h = undef_h;
}

// CHECK: undefined identifier 'undef_a'
// CHECK: undefined identifier 'undef_b'
// CHECK: undefined identifier 'undef_c'
// CHECK: undefined identifier 'undef_d'
// CHECK: undefined identifier 'undef_e'
// CHECK: undefined identifier 'undef_f'
// CHECK: undefined identifier 'undef_g'
// CHECK: undefined identifier 'undef_h'
