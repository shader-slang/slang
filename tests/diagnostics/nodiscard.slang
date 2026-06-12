//DIAGNOSTIC_TEST:SIMPLE(diag=DIAG):

// Test the `[NoDiscard]` attribute: calling a function marked `[NoDiscard]`
// and discarding its result produces an error.

struct MyType
{
    int val;

    [NoDiscard]
    MyType load() { MyType r; r.val = 42; return r; }

    [NoDiscard]
    int compute() { return val * 2; }

    int regular() { return val; }
}

[NoDiscard]
int freeFunc() { return 1; }

void consume(int x) {}

// Returning the result is using it, not discarding it, so no error.
int useResult() { return freeFunc(); }

[NoDiscard]
bool check() { return true; }

// `[NoDiscard]` is accepted on any `FunctionDeclBase`. A discarded result from a user-defined
// operator or a generic function errors; a bare constructor call is a documented exception
// (see the `Widget(7);` case in `main` below).
struct Widget
{
    int v;

    [NoDiscard]
    __init(int x) { v = x; }
}

[NoDiscard]
Widget operator+(Widget a, Widget b) { return Widget(a.v + b.v); }

[NoDiscard]
T identity<T>(T x) { return x; }

// The motivating use case: a function that has side effects via an `out` parameter but also
// returns a result the caller is expected to consume (like `linalg::CoopMat::Load`).
struct OutWidget
{
    [NoDiscard]
    int loadWith(out int sideEffect) { sideEffect = 1; return 2; }
}

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    MyType t;
    t.val = 0;

    t.load(); // error
/*DIAG:
          ^ result of '[NoDiscard]' function is discarded
          ^ the result of calling 'load' is discarded; this function is marked '[NoDiscard]'.
*/

    t.compute(); // error
/*DIAG:
             ^ result of '[NoDiscard]' function is discarded
             ^ the result of calling 'compute' is discarded; this function is marked '[NoDiscard]'.
*/

    freeFunc(); // error
/*DIAG:
            ^ result of '[NoDiscard]' function is discarded
            ^ the result of calling 'freeFunc' is discarded; this function is marked '[NoDiscard]'.
*/

    // The result is discarded in a `for` loop's side-effect expression too.
    for (int i = 0; i < 1; t.load())
/*DIAG:
                                 ^ result of '[NoDiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[NoDiscard]'.
*/
    {
        i++;
    }

    // The result is also discarded in a comma operand of a `for` loop side effect.
    for (int j = 0; j < 1; t.load(), j++)
/*DIAG:
                                 ^ result of '[NoDiscard]' function is discarded
                                 ^ the result of calling 'load' is discarded; this function is marked '[NoDiscard]'.
*/
    {
    }

    // Parentheses are transparent: a parenthesized discarded call still errors.
    (t.load());
/*DIAG:
           ^ result of '[NoDiscard]' function is discarded
           ^ the result of calling 'load' is discarded; this function is marked '[NoDiscard]'.
*/

    // A ternary discards whichever arm is selected, so both `[NoDiscard]` arms error.
    // (`?:` itself is deprecated in Slang, hence the extra deprecation diagnostic.)
    bool cond = t.val > 0;
    cond ? t.compute() : freeFunc();
/*DIAG:
                    ^ result of '[NoDiscard]' function is discarded
                    ^ the result of calling 'compute' is discarded; this function is marked '[NoDiscard]'.
                                 ^ result of '[NoDiscard]' function is discarded
                                 ^ the result of calling 'freeFunc' is discarded; this function is marked '[NoDiscard]'.
*/

    // A cast is a use of the result, so a cast-wrapped call does not error.
    (uint)freeFunc();

    // Short-circuit `&&`/`||` pass their right operand through as the expression's result, so a
    // discarded `[NoDiscard]` call in that position errors. Storing the result does not.
    bool stored = cond && check();
    cond && check();
/*DIAG:
                 ^ result of '[NoDiscard]' function is discarded
                 ^ the result of calling 'check' is discarded; this function is marked '[NoDiscard]'.
*/
    cond || check();
/*DIAG:
                 ^ result of '[NoDiscard]' function is discarded
                 ^ the result of calling 'check' is discarded; this function is marked '[NoDiscard]'.
*/

    // A discarded user-defined operator result and a discarded generic-function result both error.
    Widget w1 = Widget(1);
    Widget w2 = Widget(2);
    w1 + w2;
/*DIAG:
       ^ result of '[NoDiscard]' function is discarded
       ^ the result of calling '+' is discarded; this function is marked '[NoDiscard]'.
*/
    identity<int>(3);
/*DIAG:
                 ^ result of '[NoDiscard]' function is discarded
                 ^ the result of calling 'identity' is discarded; this function is marked '[NoDiscard]'.
*/

    // A bare discarded constructor call is not flagged: the call resolves through initializer
    // handling rather than a direct callee `DeclRef`, so `[NoDiscard]` on `__init` has no effect
    // here. (A discarded constructor call is already an obvious no-op.)
    Widget(7);

    // Storing the result is fine, no error.
    let loaded = t.load();
    int c = t.compute();

    // A function not marked `[NoDiscard]` does not error even when discarded.
    t.regular();

    // Using the result in a sub-expression is not discarding it, so none of these error:
    // as a call argument, ...
    consume(t.compute());
    // ... and as an operand of a larger, non-`[NoDiscard]` expression whose own result is
    // then discarded (the `[NoDiscard]` call's result is consumed by `+`, not discarded).
    t.compute() + c;

    // The motivating case: a method with an `out` parameter that also returns a value errors when
    // the returned value is discarded, but not when it is used.
    OutWidget ow;
    int side;
    ow.loadWith(side); // error
/*DIAG:
               ^ result of '[NoDiscard]' function is discarded
               ^ the result of calling 'loadWith' is discarded; this function is marked '[NoDiscard]'.
*/
    int used = ow.loadWith(side);

    // Used as a control-flow predicate, the result is consumed by the condition, not discarded,
    // so none of these error.
    if (check()) {}
    while (check()) break;
    do {} while (check());
    for (int k = 0; check(); k++) break;

    // Silence unused-variable concerns.
    t.val = loaded.val + c + side + used;
}
