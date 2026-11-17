//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T22:00:02Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=b9a1edc11e7f4ef12c141b50f73b3a36a9eda5811402d0c118a09dc03161e42b
//META: purpose=`$for` is deprecated from language version 2025 (E30525 warning) and removed in 202c (E30526 error).
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lifecycle of the legacy `$for`: line 0 (-std 2025) compiles with
// the E30525 deprecation warning; line 1 (-std 202c) is rejected with E30526.

//TEST:SIMPLE(filecheck=DEPR):-lang slang -std 2025 -target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=REMOVED,exit-code=-1):-lang slang -std 202c -target hlsl -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    $for (i in Range(0, 2))
    {
        s += i;
    }
    outputBuffer[0] = s;
}

// DEPR: E30525
// REMOVED: E30526
