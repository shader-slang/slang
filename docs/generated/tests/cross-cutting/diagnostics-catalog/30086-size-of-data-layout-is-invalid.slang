//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30086 (size-of-data-layout-is-invalid) - invalid data layout for sizeof/alignof
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30086
//META: catalog_name=size-of-data-layout-is-invalid
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Passing a non-IBufferDataLayout type to sizeof<Layout> triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct NotALayout {};

void main()
{
    int s = sizeof<NotALayout>(int);
}

// CHECK: 30086
