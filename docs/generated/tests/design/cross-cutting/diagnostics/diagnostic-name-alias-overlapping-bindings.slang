//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnostic-definitions
//META: doc_section_digest=6fc6f1fd5609757fcbaf6b6cf8aada297ee28b149b44b5846eb72fb0e4ebc7be
//META: purpose=The lookup table carries the single alias `overlappingBindings` for `parameterBindingsOverlap`, so both spellings resolve by name.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the alias registered alongside the generated catalog resolves.
// `-warnings-disable` reports E31111 for a name it cannot look up, so passing
// the alias spelling and then the canonical spelling and asserting E31111 never
// appears proves both names reach the same entry. The comma-operator warning is
// left untouched by either request and is asserted to still fire, which shows
// the compile ran rather than bailing out on an option error.

//TEST:SIMPLE(filecheck=ALIAS):-target hlsl -entry main -stage compute -warnings-disable overlappingBindings
//TEST:SIMPLE(filecheck=CANON):-target hlsl -entry main -stage compute -warnings-disable parameterBindingsOverlap

RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = (1, 2);
    buf[0] = x;
}

// ALIAS-NOT: E31111
// ALIAS: E41024
// CANON-NOT: E31111
// CANON: E41024
