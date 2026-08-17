//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#modifier-parsing
//META: doc_section_digest=c08befb0cda99dc013ab0ceb9ba3b786beb2359fadcd06a399a784137456d81d
//META: purpose=Attribute parsing is modifier parsing in disguise, so a `[name]` group becomes an AttributeBase modifier both before a declaration and before a statement.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `[...]` group is collected by the same modifier-parsing path
// wherever modifiers are accepted. `[ForceInline]` precedes a function
// declaration and `[unroll]` precedes a for statement in the same file; if
// either bracket group were not recognised as a modifier the parser would stop
// at the `[` and the program would never reach the interpreter. The printed
// value 16 is 2*5 from the attributed function plus the three iterations of the
// attributed loop, so both attributed constructs had to parse and run.

//TEST:INTERPRET(filecheck=CHECK):

[ForceInline]
int twice(int v) { return v + v; }

void main()
{
    int s = 0;
    [unroll]
    for (int i = 0; i < 3; i = i + 1) { s = s + 2; }
    // CHECK: r=16
    printf("r=%d\n", twice(5) + s);
}
