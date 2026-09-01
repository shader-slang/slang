//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e58e38fb1f247bcbdd1135a7c5ad62d04b17ee380b108189a7fe920e8c0fb2d8
//META: purpose=Fires diagnostic E30013 (subscript-non-array) - invalid subscript expression
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30013
//META: catalog_name=subscript-non-array
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Subscripting requires an array, vector, matrix, or a type with a subscript
// operator. Applying `[]` to a scalar `int` has no such operation, and the
// CHECK pins the invalid-subscript error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void main() { int x = 0; int y = x[0]; }

//CHECK: E30013
