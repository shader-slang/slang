//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#diagnostics-during-lowering
//META: doc_section_digest=8da9811f8488ad6ce5409491fafa35c23411a9a65708775cf2a3ad6c64768b12
//META: purpose=A `switch` body with no `case` or `default` label takes visitSwitchStmt's no-cases early return, which itself reports the unreachable-code warning (E41000) naming the first statement.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second lowering-time unreachability report: when the switch
// body contains no case/default label at all, lowering never reaches the
// per-statement tracking, so the early return reports UnreachableCode itself
// and names the first non-empty statement. The caret pins the statement
// `r = 7;` (not the `switch` keyword).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target spirv-asm -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    if (a > 0)
    {
        switch (a)
        {
            r = 7;
//CHECK:    ^ E41000
        }
    }
    buf[0] = r;
}
