//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`RequireCapabilityStmt` takes parentheses and a comma-separated atom list, so it is the spelling that can name several atoms at once.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement-level capability spelling, which the doc contrasts
// with the declaration-level `__require_capability` covered by
// decl-require-capability.slang: only this one is parenthesised and only this
// one accepts a comma-separated list. Naming two atoms is therefore the whole
// observation — if the `,` were rejected the way it is in the declaration
// form, the compile would stop at a parse error and nothing would be emitted.
// `-profile cs_5_0` is chosen so both named atoms are already in the profile
// and no implicit-upgrade warning muddies the output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -profile cs_5_0

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    __requireCapability(hlsl, sm_5_0);
    outputBuffer[0] = 7;
}

// CHECK: void main()
// CHECK: outputBuffer_{{[0-9]+}}[int(0)] = int(7);
