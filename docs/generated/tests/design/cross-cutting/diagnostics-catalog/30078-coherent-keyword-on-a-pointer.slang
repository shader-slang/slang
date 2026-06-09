//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T07:55:45Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30078 (coherent-keyword-on-a-pointer) - cannot have coherent qualifier on pointer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30078
//META: catalog_name=coherent-keyword-on-a-pointer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A `globallycoherent` qualifier on a pointer type is invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    globallycoherent int* p;
}

// CHECK: 30078
