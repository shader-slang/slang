//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#vectors-matrices-and-composite
//META: doc_section_digest=54f732694185a0ac0e3a99ea776a2df81f46ad04014e1f6987caf714e9e03dcc
//META: purpose=A function declared with a throws clause has an IR result type of Result(valueType, errorType).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Result opcode is the sum of a success value type and an error
// type, and the surface construct that produces it is a throws clause
// rather than any spelling of Result in source. A helper declared
// `int risky(int) throws MyErr` is called under try/catch; the IR dump
// is FileChecked for `func %risky : Func(Result(Int, Enum(...)), Int)`,
// which shows the declared return type became the first Result operand
// and the thrown enum became the second.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

enum MyErr { Bad }

RWStructuredBuffer<int> outputBuffer;

int risky(int v) throws MyErr
{
    if (v < 0)
        throw MyErr.Bad;
    return v;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    do { outputBuffer[0] = try risky(int(tid.x)); }
    catch (MyErr e) { outputBuffer[0] = -1; }
}

// IR: func %risky{{.*}}: Func(Result(Int, Enum(
