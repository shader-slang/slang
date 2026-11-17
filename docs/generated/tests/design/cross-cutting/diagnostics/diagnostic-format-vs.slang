//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#source-locations-and-message-rendering
//META: doc_section_digest=b56b816820af1c15450b34e955f5435969bcf8f2936c6dbb068f8ab21ec3dca3
//META: purpose=With -diagnostic-format vs the sink renders a diagnostic as file(line,col): severity E<id>: message instead of the boxed layout.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the Visual Studio diagnostic format. CHECK-DAG asserts the
// file(line,col) location prefix followed by the severity word and the code, and
// CHECK-NOT asserts the boxed header form with brackets is not produced.

//TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target hlsl -entry main -stage compute -diagnostic-format vs

void f(int a) {}
void f(float a, int b) {}

[numthreads(1, 1, 1)]
void main()
{
    f("x", 1, 2);
}

// CHECK-DAG: diagnostic-format-vs.slang({{[0-9]+}},{{[0-9]+}}): error E39999
// CHECK-DAG: diagnostic-format-vs.slang({{[0-9]+}},{{[0-9]+}}): note E40011
// CHECK-NOT: error[E39999]
