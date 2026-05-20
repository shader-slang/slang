//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/ir-reference/misc.md#string-hashing
//META: doc_section_digest=feb9c367ea292467351910e923078e1f844ea748daaed45e8b73eee871728629
//META: purpose=Two identical `getStringHash(...)` calls reference the same string-literal operand in the IR dump (stable hash, same operand).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `getStringHash` is a compile-time stable hash of
// a string literal. Two helpers calling it with the same literal
// must both emit the opcode with the same string operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint h1() { return getStringHash("shared-key"); }
uint h2() { return getStringHash("shared-key"); }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = h1();
    buf[1] = h2();
}

// IR-LABEL: func %h1
// IR: getStringHash("shared-key")
// IR-LABEL: func %h2
// IR: getStringHash("shared-key")
