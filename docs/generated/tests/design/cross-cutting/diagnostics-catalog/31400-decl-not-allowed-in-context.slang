//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c3da2cd08f8897cb71c5a931106edbbb3dc29ad453c3bf64e000488949e7e3f4
//META: purpose=Fires diagnostic E31400 (decl-not-allowed-in-context) - declaration not allowed in this context
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31400
//META: catalog_name=decl-not-allowed-in-context
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A non-static instance variable inside an interface is allowed
// by the parser but rejected by the semantic-check nesting table:
// instance variables may not appear in interfaces.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo
{
    int x;
}

//CHECK: E31400
