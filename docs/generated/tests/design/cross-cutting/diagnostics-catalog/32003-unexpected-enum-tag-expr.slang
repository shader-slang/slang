//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E32003 (unexpected-enum-tag-expr) - unexpected enum tag expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=32003
//META: catalog_name=unexpected-enum-tag-expr
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An enum case value that references another arbitrary identifier
// rather than a literal/constant expression triggers E32003.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int g() { return 1; }

enum E
{
    A = g(),
}

// CHECK: E32003
