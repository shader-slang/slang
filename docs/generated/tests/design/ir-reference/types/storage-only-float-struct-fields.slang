//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#storage-only-floating-point
//META: doc_section_digest=8b89a00e00724be691b976f3a2200b15b08ac425201af79a575547239ae672d8
//META: purpose=The three storage-only float opcodes BFloat16Type, FloatE4M3Type and FloatE5M2Type appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The storage-only float widths are distinct IR type opcodes, not
// aliases of Half or Float. A struct declares one field of each of the
// core-module types BFloat16, FloatE4M3 and FloatE5M2, and the entry
// point widens all three to float and sums them into a buffer so the
// fields survive to the dump; the three field rows are FileChecked
// under the struct parent, which shows each surface name reaches its own
// opcode rather than collapsing to a shared floating-point type.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

struct Storage
{
    BFloat16 bf;
    FloatE4M3 e4;
    FloatE5M2 e5;
}

uniform Storage s;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = float(s.bf) + float(s.e4) + float(s.e5);
}

// IR-LABEL: struct %Storage
// IR: field(%bf, BFloat16Type)
// IR: field(%e4{{[A-Za-z0-9_]*}}, FloatE4M3Type)
// IR: field(%e5{{[A-Za-z0-9_]*}}, FloatE5M2Type)
