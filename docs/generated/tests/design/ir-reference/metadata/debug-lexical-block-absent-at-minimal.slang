//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#debuglexicalblock
//META: doc_section_digest=4b17526a5ebdb3091f4f46201f2818b32e7da614403168f6e45b160e1862df35
//META: purpose=DebugLexicalBlock records exist only at Standard debug level and above, so a Minimal compile of a braced block emits none.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the level gate on lexical blocks. The MIN run (-g1) forbids the
// record from the lowering banner to end of input; the STD run (-g2) compiles
// the identical shader and finds one, so the absence is the level and not the
// shader. STD patterns carry a regex because Standard embeds this file's text.

//TEST:SIMPLE(filecheck=MIN):-target spirv-asm -dump-ir -o - -entry main -stage compute -g1
//TEST:SIMPLE(filecheck=STD):-target spirv-asm -dump-ir -o - -entry main -stage compute -g2

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

// MIN: {{^}}### LOWER-TO-IR:
// MIN-NOT: DebugLexicalBlock

// STD: {{^}}### LOWER-TO-IR:
// STD: = DebugLexicalBlock(%{{[0-9]+}}, {{[0-9]+}} : UInt
