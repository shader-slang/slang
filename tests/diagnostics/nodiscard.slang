//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG):

// Test the `[nodiscard]` attribute: calling a function marked `[nodiscard]`
// and discarding its result produces a warning.

struct MyType
{
    int val;

    [nodiscard]
    MyType load() { MyType r; r.val = 42; return r; }

    [nodiscard]
    int compute() { return val * 2; }

    int regular() { return val; }
}

[nodiscard]
int freeFunc() { return 1; }

void consume(int x) {}

// Returning the result is using it, not discarding it, so no warning.
int useResult() { return freeFunc(); }

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MyType t;
    t.val = 0;

    t.load(); // warn
/*DIAG:
          ^ result of '[nodiscard]' function is discarded
          ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/

    t.compute(); // warn
/*DIAG:
             ^ result of '[nodiscard]' function is discarded
             ^ the result of calling 'compute' is discarded; this function is marked '[nodiscard]'.
*/

    freeFunc(); // warn
/*DIAG:
            ^ result of '[nodiscard]' function is discarded
            ^ the result of calling 'freeFunc' is discarded; this function is marked '[nodiscard]'.
*/

    // The result is discarded in a `for` loop's side-effect expression too.
    for (int i = 0; i < 1; t.load())
/*DIAG:
                                 ^ result of '[nodiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/
    {
        i++;
    }

    // The result is also discarded in a comma operand of a `for` loop side effect.
    for (int j = 0; j < 1; t.load(), j++)
/*DIAG:
                                 ^ result of '[nodiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/
    {
    }

    // Storing the result is fine, no warning.
    let loaded = t.load();
    int c = t.compute();

    // A function not marked `[nodiscard]` does not warn even when discarded.
    t.regular();

    // Using the result in a sub-expression is not discarding it, so none of these warn:
    // as a call argument, ...
    consume(t.compute());
    // ... and as an operand of a larger, non-`[nodiscard]` expression whose own result is
    // then discarded (the `[nodiscard]` call's result is consumed by `+`, not discarded).
    t.compute() + c;

    // Silence unused-variable concerns.
    t.val = loaded.val + c;
}
