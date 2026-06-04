//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#global-state
//META: doc_section_digest=985b6b6d7f1d6d80faf5341c61142918f8a6c84c4b159badfdedc255cca3c5a8
//META: purpose=A `static const int` at module scope lowers to a globalConstant opcode wrapping the literal value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A module-scope `static const int gK = 42;` should emit
// `let %gK : Int = globalConstant(42 : Int)` — the value is wrapped
// by the `globalConstant` opcode, not emitted as a bare integer
// literal at module scope.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

static const int gK = 42;

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gK + n;
}

// CHECK: let  %gK	: Int	= globalConstant(42 : Int)
