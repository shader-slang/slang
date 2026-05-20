//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#global-state
//META: doc_section_digest=985b6b6d7f1d6d80faf5341c61142918f8a6c84c4b159badfdedc255cca3c5a8
//META: purpose=A module-scope `static int` variable lowers to a global_var opcode with a Ptr-typed result and a one-block initializer body.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A module-scope mutable global like `static int gCounter = 0;`
// should lower to `global_var %gCounter : Ptr(Int)` with a body that
// returns the initial value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

static int gCounter = 0;

[numthreads(1, 1, 1)]
void main()
{
    gCounter = gCounter + 1;
    buf[0] = gCounter;
}

// CHECK: global_var %gCounter	: Ptr(Int)
// CHECK: return_val(0 : Int)
