//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/compiler-core/slang-misc-diagnostic-defs.h
//META: purpose=Fires diagnostic E100002 (unbalancedDownstreamArguments) - unbalanced downstream arguments
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=100002
//META: catalog_name=unbalancedDownstreamArguments
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two `-Xdxc...` opens with no matching `-X.` close leave the
// downstream scope nested, triggering the unbalanced-args error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute -Xdxc... -Xdxc... --

[numthreads(1,1,1)]
void main() {}

// CHECK: 100002
