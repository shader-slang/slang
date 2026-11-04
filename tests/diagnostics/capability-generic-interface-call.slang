// Calling Bad.invoke through a generic interface constraint must retain its anyhit requirement.
// Re-enable both generic-call runs after https://github.com/shader-slang/slang/issues/13995.
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=GENERIC): -entry main -target hlsl -no-codegen -DCALL_MODE=0
//DISABLE_DIAGNOSTIC_TEST:SIMPLE(filecheck=GENERIC): -entry main -target hlsl -no-codegen -DCALL_MODE=0 -std 2026
// A fix may reject the implementation's conformance or the selected entry point.
//GENERIC: error[E361

// Direct calls and calls through an ordinary helper already reject the same stage mismatch.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONTROL): -entry main -target hlsl -no-codegen -DCALL_MODE=1
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONTROL): -entry main -target hlsl -no-codegen -DCALL_MODE=1 -std 2026
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONTROL): -entry main -target hlsl -no-codegen -DCALL_MODE=2
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CONTROL): -entry main -target hlsl -no-codegen -DCALL_MODE=2 -std 2026
//CONTROL: error[E36107]: unavailable features in entry point

interface IFoo
{
    static void invoke();
}

struct Bad : IFoo
{
    static void invoke()
    {
        IgnoreHit();
    }
}

void forward<T : IFoo>()
{
    T::invoke();
}

void forwardDirectly()
{
    Bad::invoke();
}

[shader("miss")]
void main()
{
#if CALL_MODE == 0
    forward<Bad>();
#elif CALL_MODE == 1
    Bad::invoke();
#else
    forwardDirectly();
#endif
}
