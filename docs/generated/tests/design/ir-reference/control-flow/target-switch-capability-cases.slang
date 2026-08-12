//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#switch-and-targetswitch
//META: doc_section_digest=bb310095020a0added87bde60cb729ab81078e469d274a935ab7dd9ece947e47
//META: purpose=A __target_switch lowers to targetSwitch(breakBlock, caseValue/caseBlock pairs...) whose case values are integer CapabilityName keys rather than runtime values.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the targetSwitch operand layout: unlike `switch`, operand 0 is
// the break block and there is no scrutinee at all, because the selection is
// made at compile time from the code-generation target. The CHECK binds
// operand 0 and pins three trailing (integer key, block) pairs -- the keys
// are left as wildcards because they are CapabilityName enum values, not
// values the surface program named. Binding operand 0 also lets the CHECK
// show the break block carries only `unreachable`, since every arm returns.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int pick(int v)
{
    __target_switch
    {
    case hlsl:
        return v + 1;
    case spirv:
        return v + 2;
    default:
        return v;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pick(n);
}

// CHECK-LABEL: func %pick
// CHECK: targetSwitch(%[[BRK:[0-9]+]], {{[0-9]+}} : Int, %{{[0-9]+}}, {{[0-9]+}} : Int, %{{[0-9]+}}, {{[0-9]+}} : Int, %{{[0-9]+}})
// CHECK: block %[[BRK]]:
// CHECK-NEXT: unreachable
