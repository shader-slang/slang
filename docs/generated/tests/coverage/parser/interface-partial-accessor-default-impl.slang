//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies an interface property that gives a default body to some but not all of its accessors is rejected with E30318.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: after parsing a callable inside an interface, the parser
// runs `maybeReparseInterfaceCallableAsExplicitGeneric`, which classifies the
// requirement's default-implementation bodies as None, Partial or complete.
// For a complete default it re-parses the declaration as an explicit generic
// so the body becomes a default implementation; for None it does nothing. The
// Partial case is the error case: a property whose `get` has a body while its
// `set` is left as a bare requirement cannot be turned into one default
// implementation, so the parser reports it and strips the body it did find so
// that later phases see a plain requirement.
//
// The diagnostic is attached to the accessor that carried the body, which is
// why it points at `get` rather than at the property.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

interface IFoo
{
    property int val
    {
        get { return 1; }
//CHECK:^^^ partial interface accessor default implementation
//CHECK:^^^ interface accessor default implementation is incomplete; define all accessors or none.
        set;
    }
}

[numthreads(1, 1, 1)]
void main() {}
