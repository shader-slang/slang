//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Test the `[nodiscard]` attribute: calling a function marked `[nodiscard]`
// and discarding its result produces a warning.

struct MyType
{
    int val;

    [nodiscard]
    MyType load() { MyType r; r.val = 42; return r; }

    [nodiscard]
    int compute() { return val * 2; }

    // `[nodiscard]` on a `void` function has no result to discard, so discarding
    // a call to it never warns.
    [nodiscard]
    void touch() {}

    int regular() { return val; }
}

[nodiscard]
int freeFunc() { return 1; }

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MyType t;
    t.val = 0;

    t.load(); // warn
/*diag:
          ^ result of '[nodiscard]' function is discarded
          ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/

    t.compute(); // warn
/*diag:
             ^ result of '[nodiscard]' function is discarded
             ^ the result of calling 'compute' is discarded; this function is marked '[nodiscard]'.
*/

    freeFunc(); // warn
/*diag:
            ^ result of '[nodiscard]' function is discarded
            ^ the result of calling 'freeFunc' is discarded; this function is marked '[nodiscard]'.
*/

    // The result is discarded in a `for` loop's side-effect expression too.
    for (int i = 0; i < 1; t.load())
/*diag:
                                 ^ result of '[nodiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/
    {
        i++;
    }

    // The result is also discarded in a comma operand of a `for` loop side effect.
    for (int j = 0; j < 1; t.load(), j++)
/*diag:
                                 ^ result of '[nodiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/
    {
    }

    // `[nodiscard]` on a `void` function does not warn even when discarded.
    t.touch();

    // Storing the result is fine, no warning.
    let loaded = t.load();
    int c = t.compute();

    // A function not marked `[nodiscard]` does not warn even when discarded.
    t.regular();

    // Silence unused-variable concerns.
    t.val = loaded.val + c;
}
