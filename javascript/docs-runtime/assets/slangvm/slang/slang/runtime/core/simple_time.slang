// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// Opaque high-resolution CPU wall-clock timer.
__target_intrinsic(hgbc, "OpTimerType") struct Timer
{
};

Timer createTimer()
{
    return hgbc_asm
    {
    result:
        $$Timer = OpCreateTimer;
    };
}

float elapsedSeconds(Timer timer)
{
    return hgbc_asm
    {
    result:
        $$float = OpTimerElapsed $timer;
    };
}

void resetTimer(Timer timer)
{
    hgbc_asm
    {
        OpResetTimer $timer;
    };
}
