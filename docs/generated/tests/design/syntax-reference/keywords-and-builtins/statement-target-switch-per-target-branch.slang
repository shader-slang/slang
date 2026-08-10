//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
//META: purpose=`__target_switch` is a statement keyword dispatched at statement entry (`parseTargetSwitchStmt`); each `case <target>` arm reuses the documented `case`/`default` keywords and only the arm matching the active target survives into the emitted code.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parser recognizes `__target_switch` at statement head
// and reuses `case` / `default` labels inside its body. A helper
// returns a distinct constant per target arm; the HLSL emit must
// contain the hlsl-arm constant and never the glsl-arm constant, and
// vice versa — only target-selected code reaches emission, which is
// only possible if the statement parsed as a target switch rather
// than an ordinary switch.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

int pickPerTarget()
{
    __target_switch
    {
    case hlsl:
        return 111333;
    case glsl:
        return 222333;
    default:
        return 999333;
    }
}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // HLSL-NOT: 222333
    // HLSL: 111333
    // HLSL-NOT: 222333
    // GLSL-NOT: 111333
    // GLSL: 222333
    // GLSL-NOT: 111333
    outBuf[0] = pickPerTarget();
}
