//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30075 (cannot-specialize-generic) - cannot specialize generic
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30075
//META: catalog_name=cannot-specialize-generic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Supplying the wrong number of type arguments to a generic triggers the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Foo<T> { T value; };

void main()
{
    Foo<int, float> x;
}

// CHECK: E30075
