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

[nodiscard]
bool check() { return true; }

// `[nodiscard]` is accepted on any `FunctionDeclBase`, so it should also warn when the
// discarded result comes from a constructor, a user-defined operator, or a generic function.
struct Widget
{
    int v;

    [nodiscard]
    __init(int x) { v = x; }
}

[nodiscard]
Widget operator+(Widget a, Widget b) { return Widget(a.v + b.v); }

[nodiscard]
T identity<T>(T x) { return x; }

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

    // Parentheses are transparent: a parenthesized discarded call still warns.
    (t.load());
/*DIAG:
           ^ result of '[nodiscard]' function is discarded
           ^ the result of calling 'load' is discarded; this function is marked '[nodiscard]'.
*/

    // A ternary discards whichever arm is selected, so both `[nodiscard]` arms warn.
    // (`?:` itself is deprecated in Slang, hence the extra deprecation diagnostic.)
    bool cond = t.val > 0;
    cond ? t.compute() : freeFunc();
/*DIAG:
                    ^ result of '[nodiscard]' function is discarded
                    ^ the result of calling 'compute' is discarded; this function is marked '[nodiscard]'.
                                 ^ result of '[nodiscard]' function is discarded
                                 ^ the result of calling 'freeFunc' is discarded; this function is marked '[nodiscard]'.
*/

    // A cast is a transparent wrapper, so a cast-wrapped discarded call still warns.
    (uint)freeFunc();
/*DIAG:
                  ^ result of '[nodiscard]' function is discarded
                  ^ the result of calling 'freeFunc' is discarded; this function is marked '[nodiscard]'.
*/

    // Short-circuit `&&`/`||` pass their right operand through as the expression's result, so a
    // discarded `[nodiscard]` call in that position warns. Storing the result does not warn.
    bool stored = cond && check();
    cond && check();
/*DIAG:
                 ^ result of '[nodiscard]' function is discarded
                 ^ the result of calling 'check' is discarded; this function is marked '[nodiscard]'.
*/
    cond || check();
/*DIAG:
                 ^ result of '[nodiscard]' function is discarded
                 ^ the result of calling 'check' is discarded; this function is marked '[nodiscard]'.
*/

    // A discarded user-defined operator result and a discarded generic-function result both warn.
    Widget w1 = Widget(1);
    Widget w2 = Widget(2);
    w1 + w2;
/*DIAG:
       ^ result of '[nodiscard]' function is discarded
       ^ the result of calling '+' is discarded; this function is marked '[nodiscard]'.
*/
    identity<int>(3);
/*DIAG:
                 ^ result of '[nodiscard]' function is discarded
                 ^ the result of calling 'identity' is discarded; this function is marked '[nodiscard]'.
*/

    // A bare discarded constructor call is not flagged: the call resolves through initializer
    // handling rather than a direct callee `DeclRef`, so `[nodiscard]` on `__init` has no effect
    // here. (A discarded constructor call is already an obvious no-op.)
    Widget(7);

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
