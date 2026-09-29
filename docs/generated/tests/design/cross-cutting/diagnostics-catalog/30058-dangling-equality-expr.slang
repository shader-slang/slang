//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=67817fa4198c647f6ce020d1700b7f01b73eb1826fa3cc1635e40ad7f6ec9e16
//META: purpose=Fires diagnostic W30058 (dangling-equality-expr) - result of '==' not used
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30058
//META: catalog_name=dangling-equality-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An `==` whose result is discarded as a statement fires the warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int a = 1;
    int b = 2;
    a == b;
}

//CHECK: 30058
