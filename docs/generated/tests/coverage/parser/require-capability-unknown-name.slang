//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies `__requireCapability` parses a comma-separated list and rejects an unrecognized atom name with E36105 while still consuming the rest of the list.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `Parser::ParseRequireCapabilityStatement` reads
// `__requireCapability` '(' identifier (',' identifier)* ')' ';', looking each
// identifier up with `findCapabilityName`. An identifier that is not a known
// capability atom is diagnosed and dropped, and the loop keeps going through
// the comma, so the trailing valid atom is still accepted and the statement
// still parses to completion. CHECK pins E36105 and the observed message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

[numthreads(1, 1, 1)]
void main()
{
    __requireCapability(no_such_capability_xyz, spirv_1_3);
//CHECK: E36105
//CHECK: unknown capability name 'no_such_capability_xyz'.
}
