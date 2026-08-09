//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=83632b726fecd52003cb9a007e201049bb33c66f077ab6265b07278009caca02
//META: purpose=Stress: the sink's output buffer accumulates every diagnostic of a long run of undefined-identifier errors; none is dropped.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the sink's accumulating output does not drop diagnostics under
// volume. Eight distinct undeclared names are referenced in one entry point;
// CHECK asserts every one of the eight messages appears, so a cap or
// dedup that silently discarded later records would fail the test.

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
