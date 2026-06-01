//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E30067 (mutating-method-on-function-input-parameter-error) - mutating method called on `in` parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30067
//META: catalog_name=mutating-method-on-function-input-parameter-error
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Calling a mutating method on an `in` parameter raises the diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Counter
{
    int value;
    [mutating] void inc() { value++; }
};

void use(in Counter c)
{
    c.inc();
}

void main() { Counter c; c.value = 0; use(c); }

// CHECK: 30067
