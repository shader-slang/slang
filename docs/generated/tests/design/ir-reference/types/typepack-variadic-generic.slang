//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#tuples-packs-and-target-tuples
//META: doc_section_digest=5dd886c6a7091235bb66306bec8f28f82567d9db3e4761919dd9f9bc5d6b0b8c
//META: purpose=Calling a variadic generic produces a value of IR type TypePack(types...) holding one operand per pack element.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A variadic-generic type pack has its own IR type opcode whose operand
// list is the pack's element types, in order. A `sum<each T : IInteger>`
// helper is called with three int arguments; the IR dump is FileChecked
// for the argument pack value typed `TypePack(Int, Int, Int)` produced by
// makeValuePack, so the arity of the call site is what determines the
// operand count of the pack type.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage compute -entry main -dump-ir -o /dev/null

RWStructuredBuffer<int> outputBuffer;

int sum<each T : IInteger>(expand each T vals)
{
    int r = 0;
    expand r += int((each vals).toInt());
    return r;
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = sum(1, 2, 3);
}

// IR: TypePack(Int, Int, Int){{.*}}= makeValuePack(1 : Int, 2 : Int, 3 : Int)
