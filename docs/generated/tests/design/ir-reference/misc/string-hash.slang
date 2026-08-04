//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:44:47Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/ir-reference/misc.md#string-hashing
//META: doc_section_digest=feb9c367ea292467351910e923078e1f844ea748daaed45e8b73eee871728629
//META: purpose=`getStringHash("...")` lowers to the `getStringHash` IR opcode carrying the literal string operand for a stable compile-time hash.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names `GetStringHashExpr` as the AST origin and
// `IRStringLit` as the operand. We wrap in a helper so the call
// is visible in a named function in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

uint helperA() { return getStringHash("hello-world"); }
uint helperB() { return getStringHash("slang-misc"); }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helperA();
    buf[1] = helperB();
}

// IR-LABEL: func %helperA
// IR: getStringHash("hello-world")
// IR-LABEL: func %helperB
// IR: getStringHash("slang-misc")
