//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#logical
//META: doc_section_digest=a02f1013f3939bcd733b43d3c9c2644ac90b70974d479007af65779a682e1e59
//META: purpose=The free `and(x, y)` / `or(x, y)` core-module functions lower to the `logicalAnd` / `logicalOr` opcodes, which print in the IR dump as `and` / `or` on a `Bool` result.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the non-short-circuiting boolean helpers are what
// produce `logicalAnd` / `logicalOr`, and that both operands are
// evaluated before the opcode runs. Two `bool` values are computed from
// `uniform` ints (so neither folds), then combined with `and(...)` /
// `or(...)`. The dump prints both the bitwise and the logical opcode as
// `and` / `or`, so CHECK pins the `Bool` result type — the only thing
// that distinguishes `logicalAnd` from the bitwise `and`. The `cmpGT`
// lines above them prove both operands were materialised first.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    bool x = a > 0;
    bool y = b > 0;
    buf[0] = and(x, y) ? 1 : 0;
    buf[1] = or(x, y) ? 1 : 0;
}

// IR-LABEL: func %main
// IR-DAG: let  %x{{[A-Za-z0-9_]*}}	: Bool	= cmpGT(%a, 0 : Int)
// IR-DAG: let  %y{{[A-Za-z0-9_]*}}	: Bool	= cmpGT(%b, 0 : Int)
// IR-DAG: : Bool	= and(%x, %y)
// IR-DAG: : Bool	= or(%x, %y)
