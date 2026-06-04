//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T11:10:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E30080 (ambiguous-conversion) - ambiguous conversion
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30080
//META: catalog_name=ambiguous-conversion
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two implicit init paths from int to S produce an ambiguous conversion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    int x;
    __init(int v) { x = v; }
    __init(uint v) { x = int(v); }
};

void f(S s) {}

void main()
{
    f(0);
}

// CHECK: 30080
