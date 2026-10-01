//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#basic-scalar-types
//META: doc_section_digest=68c1541af3d471fceb3efd100d4811264b41226d1023362e44bf1166be1fb329
//META: purpose=The floating-point scalar opcodes Half, Float, Double appear as struct-field types in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The floating-point scalar opcodes Half, Float, Double appear as
// struct-field types in the IR dump. A struct whose fields cover all
// three floating-point widths surfaces each opcode in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

struct Floats { half a; float b; double c; }

uniform Floats s;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = float(s.a) + s.b + float(s.c);
}

// IR-LABEL: struct %Floats
// IR: field(%{{.*}}, Half)
// IR: field(%{{.*}}, Float)
// IR: field(%{{.*}}, Double)
