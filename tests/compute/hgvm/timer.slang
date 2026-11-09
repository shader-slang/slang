//TEST:INTERPRET(hgvm): -api default

__target_intrinsic(hgbc, "OpTimerType")
struct Timer
{
}

Timer createTimer()
{
    return hgbc_asm {
        result : $$Timer = OpCreateTimer;
    };
}

float elapsedSeconds(Timer timer)
{
    return hgbc_asm {
        result : $$float = OpTimerElapsed $timer;
    };
}

void resetTimer(Timer timer)
{
    hgbc_asm {
        OpResetTimer $timer;
    };
}

void fail(String message)
{
    hgbc_asm {
        OpFail $message;
    };
}

[host]
void main()
{
    Timer timer = createTimer();
    Timer alias = timer;
    if (elapsedSeconds(alias) < 0.0)
        fail("timer returned a negative duration");
    resetTimer(alias);
    if (elapsedSeconds(timer) < 0.0)
        fail("timer alias did not preserve the shared reset point");
}
