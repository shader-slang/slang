//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-defer-and-asm
//META: doc_section_digest=8197d918039016a312fb02c828516cb8f62b7d3234a740d1e588d091a11132ad
//META: purpose=An __intrinsic_asm statement lowers to the GenericAsm terminator carrying the inline target text as its operand 0 string literal.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the GenericAsm row: an inline-asm statement becomes a terminator
// in its own right, which is why the block that hosts it needs no branch
// afterwards. The asm is placed in a __target_switch arm for a target other
// than the one being compiled, so the text is still present in the lowered
// IR while the emitted HLSL never has to interpret it. The CHECK pins the
// arm block ending on GenericAsm with the verbatim asm text as its operand.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int bump(int v)
{
    __target_switch
    {
    case cpp:
        __intrinsic_asm "($0) + 1";
    default:
        return v + 1;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bump(n);
}

// CHECK-LABEL: func %bump
// CHECK: targetSwitch(%{{[0-9]+}}, {{[0-9]+}} : Int, %[[ASMBLOCK:[0-9]+]], {{[0-9]+}} : Int, %{{[0-9]+}})
// CHECK: block %[[ASMBLOCK]]:
// CHECK-NEXT: GenericAsm("($0) + 1")
