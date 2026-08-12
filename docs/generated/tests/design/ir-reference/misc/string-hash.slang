//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#string-hashing
//META: doc_section_digest=aa2afd6a45af4a5bede0e4e6bb51d5bf4d6150e7a3b1fd9599961c4908973d52
//META: purpose=A call to the core-module `getStringHash(String)` lowers to a `getStringHash` inst whose single operand is the string literal itself.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the hash inst keeps the literal as its operand rather than a
// pre-computed number, which is what lets a later stage compute a stable hash
// of the literal's bytes. Two helpers hash two different literals; each CHECK
// is anchored at its own helper and quotes the literal text inside the inst,
// so an operand that had already been folded to an integer would not match.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<uint> outputBuffer;

uint helperA() { return getStringHash("hello-world"); }
uint helperB() { return getStringHash("slang-misc"); }

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = helperA();
    outputBuffer[1] = helperB();
}

// IR-LABEL: func %helperA
// IR: getStringHash("hello-world")
// IR-LABEL: func %helperB
// IR: getStringHash("slang-misc")
