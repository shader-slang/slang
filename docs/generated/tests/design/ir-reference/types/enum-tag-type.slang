//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:21:07Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/ir-reference/types.md#enum
//META: doc_section_digest=fc40d41df860311a9d23cd35b8f69698290ae1d742cf3080aef665ce2994545b
//META: purpose=An enum declaration lowers to the Enum parent opcode whose single operand is its integer tag type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies EnumDecl lowering produces the Enum parent opcode carrying the
// underlying integer tag type as its operand. A two-case enum is selected
// at runtime and its value stored through an int cast so the type is used;
// IR asserts the dump contains the named `Enum %Color` inst with the
// default Int tag type on its inst line.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

enum Color
{
    Red = 3,
    Green = 7,
}

uniform uint pick;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Color c = (pick != 0) ? Color.Red : Color.Green;
    buf[0] = int(c);
}

// IR: Enum %Color
// IR-SAME: Int
