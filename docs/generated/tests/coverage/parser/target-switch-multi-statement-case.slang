//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies a `__target_switch` case that holds more than one statement collects them into a single sequence body, so all of them run for the selected target.
//META: intent=characterization
//META: covers=source/slang/slang-parser.cpp
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `parseTargetSwitchStmtImpl` keeps a single `bodyStmt` for
// the labels it just collected. The first statement it parses becomes that
// body directly; the second one has to promote it, so the parser allocates a
// `SeqStmt`, moves the existing body in, and appends. Every statement after
// that appends to the existing `SeqStmt` instead of promoting again. Only a
// case with three or more statements exercises both the promotion and the
// append.
//
// The three statements below compute 1, then +10, then +100, so a result of
// 111 in the emitted HLSL is only reachable if all three ended up in the case
// body. `default:` is present so a non-HLSL target would take the other arm.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

int f()
{
    int r = 0;
    __target_switch
    {
    case hlsl:
        r = 1;
        r = r + 10;
        r = r + 100;
    default:
        r = 7;
    }
    return r;
}

//CHECK: int f_0()
//CHECK: return int(111);

[numthreads(1, 1, 1)]
void main(uniform RWStructuredBuffer<int> o)
{
    o[0] = f();
}
