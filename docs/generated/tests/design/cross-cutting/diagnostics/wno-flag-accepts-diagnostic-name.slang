//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#severity-levels
//META: doc_section_digest=a2c5b6b64f38a8b7fd3692290be87fe1f26f4a31a5f12e076913b5c584acc7f3
//META: purpose=The -W<id> / -Wno-<id> option operand goes through overrideDiagnostic, which accepts a diagnostic name as well as an integer id, so -Wno-comma-operator-used-in-expression suppresses E41024.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The option table spells these flags with an id, but the operand is
// resolved by the same routine `-warnings-disable` uses, so a name works
// there too. The shader raises the comma-operator warning. The BASE
// directive compiles it with no flag and asserts E41024 is reported, so
// the suppression assertions below cannot pass vacuously. NAME then
// passes the kebab-case entry name to `-Wno-` and asserts the code is
// gone, and ID does the same with the integer, showing both spellings
// reach the same override.

//TEST:SIMPLE(filecheck=BASE):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=NAME):-target hlsl -entry main -stage compute -Wno-comma-operator-used-in-expression
//TEST:SIMPLE(filecheck=ID):-target hlsl -entry main -stage compute -Wno-41024

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int x = (1, int(tid.x));
    outBuf[tid.x] = x;
}

// BASE: E41024

// NAME-NOT: E41024
// NAME: void main
// NAME-NOT: E41024

// ID-NOT: E41024
// ID: void main
// ID-NOT: E41024
