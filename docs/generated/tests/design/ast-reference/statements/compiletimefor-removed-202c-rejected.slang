//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ast-reference/statements.md#compiletimeforstmt
//META: doc_section_digest=08fbefd5efa0200e11e5b0f265a4371772d26792ec01888568068161b71d8913
//META: purpose=From language version 202c on, the compile-time `for` is reported as the error E30526 (removed from the language) while the node is still built.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `$for` removal error: under `-std 202c` the same loop is
// rejected with E30526.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute -std 202c

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int s = 0;
    $for(i in Range(3)) { s += i; }
/*CHECK:
     ^^^ compile-time for has been removed
     ^^^ compile-time for has been removed from the language. Use '[ForceUnroll] for' instead.
*/
    outputBuffer[0] = s;
}
