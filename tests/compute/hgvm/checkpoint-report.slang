//TEST:INTERPRET(hgvm,filecheck=REPORT): -api vulkan -report-checkpoint-intermediates
//TEST:INTERPRET(hgvm,filecheck=QUIET): -api vulkan

// Exercise shader-JIT diagnostics as well as the host compilation. The unflagged run must
// use a different cached compiler session and RHI environment, with no checkpoint notes.
// REPORT: note: no checkpoint contexts to report
// REPORT: note: checkpointing context of {{[0-9]+}} bytes associated with: 'checkpointProbe'
// QUIET-NOT: checkpoint
// QUIET: 42
// QUIET-NOT: checkpoint

[Differentiable]
[PreferCheckpoint]
float checkpointValue(float x)
{
    return log(x);
}

[Differentiable]
float checkpointProbe(float x, bool useValue)
{
    float y = useValue ? checkpointValue(x) : 1.0f;
    return y * y;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(RWStructuredBuffer<float> values, float value, bool useValue)
{
    var x = diffPair(value, 0.0f);
    bwd_diff(checkpointProbe)(x, useValue, 1.0f);
    values[0] = x.d;
}

[host]
void main()
{
    let values = RWStructuredBuffer<float>::alloc(1);
    dispatch(computeMain, uint3(1, 1, 1))(values, 2.0f, true);
    float error = values.readBack(0) - 0.69314718f;
    if (error * error > 1e-10f)
    {
        String message = "Checkpoint reporting changed the derivative";
        hgbc_asm { OpFail $message; };
    }
    print(42);
}
