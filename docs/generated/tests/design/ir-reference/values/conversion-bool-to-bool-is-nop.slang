//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=The Bool-to-Bool cell of the conversion table emits no instruction at all: `bool(b)` returns the operand unchanged, so the named value flows straight into its consumer.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one carve-out in the conversion table that produces no
// opcode. Every other data-carrying cell names a cast instruction, but
// a bool-to-bool cast is a no-op, so lowering hands the consumer the
// original value. `b` is produced by a comparison against a dispatch
// id so it is a real SSA value rather than a literal, and the
// round-tripped `bool(b)` is what steers the `ifElse`. CHECK asserts
// that the branch condition is `%b` itself — the same name the
// comparison defined — which can only hold if no cast inst was
// inserted between them.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    bool b = (tid.y != 0);
    bool sameKind = bool(b);
    buf[0] = sameKind ? 2 : 0;
}

// IR-LABEL: func %main
// IR: let  %b	: Bool	= cmpNE(%{{[A-Za-z0-9_]+}}, 0 : UInt)
// IR: ifElse(%b,
