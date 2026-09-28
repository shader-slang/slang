//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=66dc6a3273bdb09f22df74ff8762f282efaf73d8fca97fc9dd6e21da48a2419e
//META: purpose=Fires diagnostic E31010 (link-time-constant-array-size) -- Link-time constant sized arrays are a work in progress feature, some aspects of the reflection API may not work
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31010
//META: catalog_name=link-time-constant-array-size
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

extern static const int N;
struct S { int arr[N]; }
ParameterBlock<S> b;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31010
