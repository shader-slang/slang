//DIAGNOSTIC_TEST:SIMPLE:
// This test just needs to compile without crashing the compiler.

void f()
{
    // The problem here is that the deferred block occurs after the for-loop,
    // and is therefore unreachable. We must generate correct IR even in that
    // case.
    defer printf("B");
    for (;;) {}
}
