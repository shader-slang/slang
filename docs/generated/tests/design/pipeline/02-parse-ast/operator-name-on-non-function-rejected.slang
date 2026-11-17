//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=35a0377e8327c387000594e690000a91232b821b59188e50e4c846f99b749334
//META: purpose=An `operator <op>` declarator name is rejected with E20020 for a variable, a typedef, and a parameter, because the check sits on the single declarator chokepoint every C-style declaration passes through.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the operator-name rule needs no per-declaration-kind handling.
// Three different declaration kinds each use a well-formed operator token as
// the declared name; none of them is a function, so all three must draw E20020,
// and the message names the exact set of rejected kinds. If the rule were
// implemented per kind instead of at the shared declarator unwrap, at least one
// of these sites would go silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int operator+ = 3;
//CHECK:    ^ error E20020
//CHECK:    ^ an operator name can only be used to declare an operator function, not a variable, parameter, typedef, or property

typedef int operator-;
//CHECK:            ^ error E20020
//CHECK:            ^ an operator name can only be used to declare an operator function, not a variable, parameter, typedef, or property

void takesOp(int operator*)
//CHECK:                 ^ error E20020
//CHECK:                 ^ an operator name can only be used to declare an operator function, not a variable, parameter, typedef, or property
{
}

void main()
{
}
