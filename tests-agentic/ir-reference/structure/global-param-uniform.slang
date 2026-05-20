//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/ir-reference/structure.md#global-state
//META: doc_section_digest=985b6b6d7f1d6d80faf5341c61142918f8a6c84c4b159badfdedc255cca3c5a8
//META: purpose=A module-scope uniform parameter lowers to a global_param opcode at module scope.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A module-scope `uniform int gIn;` declaration should lower to a
// `let %gIn : Int = global_param` at module scope — the
// `global_param` opcode carries the shader-uniform / push-constant
// kind of input named in the doc.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int gIn;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gIn;
}

// CHECK: let  %gIn	: Int	= global_param
