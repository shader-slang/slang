//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E40001 (ray-payload-invalid-stage-in-access-qualifier) - invalid stage name in ray payload access qualifier
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=40001
//META: catalog_name=ray-payload-invalid-stage-in-access-qualifier
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using an invalid stage name (`bogus`) in a ray payload access
// qualifier triggers E40001.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage closesthit -entry main

struct [raypayload] Payload
{
    float c : write(bogus) : read(caller);
}

[shader("closesthit")]
void main(inout Payload p, in BuiltInTriangleIntersectionAttributes attrs)
{
    p.c = 0.0;
}

// CHECK: E40001
