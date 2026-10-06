//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugvar
//META: doc_section_digest=36d7febd0fe69b032d87ba181b780aaaa9c174da2ac29fb3b6a974c00865dc14
//META: purpose=A DebugVar for an entry-point parameter carries the optional argument-index operand after source, line and column.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the present case of the optional fourth operand. The entry point's
// only parameter is argument 0, so its variable record must take the
// four-operand form; the CHECK pins the closing parenthesis after the index so
// a three-operand record would not match, and the captured source id is
// required to be the module's `DebugSource` rather than a name string -- the
// point of the notable-opcode callout about the stale generated accessors.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: %[[SRC:[0-9]+]]{{.*}}= DebugSource(
// CHECK: = DebugVar(%[[SRC]], {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, 0 : UInt)
