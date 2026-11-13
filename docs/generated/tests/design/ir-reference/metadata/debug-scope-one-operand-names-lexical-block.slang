//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debugscope-and-debugnoscope
//META: doc_section_digest=8849ab9e5ff9cbb66329b5cf029a66c4b8bf64c0c450f4968378a12e13634ef1
//META: purpose=Lowering emits a one-operand DebugScope naming a DebugLexicalBlock when it enters a braced block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lowering-side DebugScope form. The CHECK captures the lexical
// block's id and then requires a DebugScope whose only operand is that id,
// with no inlining-chain operand, since nothing is inlined here.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -g

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int a = int(tid.x);
    if (a > 1)
    {
        int b = a * 2;
        buf[1] = b;
    }
    buf[0] = a;
}

// CHECK: {{^}}### LOWER-TO-IR:
// CHECK: %[[LB:[0-9]+]]{{[ \t]*}}: Void{{[ \t]*}}= DebugLexicalBlock(
// CHECK: DebugScope(%[[LB]])
