//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b9896b8baa4040e37fb85169591c99cd39f6947e9e3be40d3de3aa4d9806be7e
//META: purpose=Fires diagnostic E31010 (link-time-constant-array-size) -- Link-time constant sized arrays are a work in progress feature, some aspects of the reflection API may not work
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31010
//META: catalog_name=link-time-constant-array-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

extern static const int N;
struct S { int arr[N]; }
ParameterBlock<S> b;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31010
