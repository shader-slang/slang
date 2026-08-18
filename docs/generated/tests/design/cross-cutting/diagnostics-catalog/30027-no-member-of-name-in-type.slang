//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=2b30c86167426fd55559b4fc196eeb44533296c167eb6520171b41ea15c9eaa5
//META: purpose=Fires diagnostic E30027 (no-member-of-name-in-type) - member not found
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30027
//META: catalog_name=no-member-of-name-in-type
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Member lookup is restricted to the members the type declares. Naming a member
// the struct does not have leaves lookup empty, and the CHECK pins the
// member-not-found error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
struct S { int a; }
void main() { S s; int b = s.notAMemberXyz; }

//CHECK: E30027
