//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debuglexicalblock
//META: doc_section_digest=4b17526a5ebdb3091f4f46201f2818b32e7da614403168f6e45b160e1862df35
//META: purpose=DebugLexicalBlock carries source, line, column and a parentScope that is the enclosing DebugFunction for a braced block directly in the function body, and is created at Standard debug level.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the operand shape of the lexical-block record. The shader has one
// braced block inside `main`; the CHECK captures the function record's id and
// requires the block record to end with exactly that id as its fourth operand.
// At full -g the DebugSource record echoes this file, so each pattern leads
// with a numeric id that the echoed comment text cannot reproduce.

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
// CHECK: %[[FN:[0-9]+]]{{.*}}= DebugFunction("main"
// CHECK: = DebugLexicalBlock(%{{[0-9]+}}, {{[0-9]+}} : UInt, {{[0-9]+}} : UInt, %[[FN]])
