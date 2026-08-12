//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=4820552ad726706a9ebecdf715453f85eb93c80c7a2decca221f929866c46c78
//META: purpose=A __stage_switch lowers to the ordinary switch terminator whose scrutinee is the compiler-supplied current-stage value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the catalog's claim that StageSwitchStmt shares the `switch`
// opcode with an ordinary SwitchStmt rather than getting its own terminator.
// The surface program contains no `switch` statement at all, so any `switch`
// in the dump has to have come from the `__stage_switch`; the CHECK also
// pins the scrutinee to the synthesized current-stage query, which is what
// distinguishes this lowering from a runtime switch on a program value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

int pick(int v)
{
    __stage_switch
    {
    case compute:
        return v + 1;
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
// CHECK: [[STAGE:%[0-9]+]]{{.*}}= GetCurrentStage
// CHECK: switch([[STAGE]], %{{[0-9]+}}, %{{[0-9]+}}, {{[0-9]+}} : Int, %{{[0-9]+}})
