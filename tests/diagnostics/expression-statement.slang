//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Diagnostics for the form and result of an expression used as a statement
// (see shader-slang/slang#12428, #12523, #12524, #12433):
//   - case (1), warning E30073: a statement whose form is not assignment / call / `++` / `--`.
//   - case (2), warning E30074: a statement that discards a non-`void` result.
//   - case (3), error   E30088: a statement that names a type rather than a value.

struct MyType { int x; };
typealias MyAlias = MyType;

int addOne(int x) { return x + 1; }
void doThing() {}
int compute() { return 1; }

typealias IntFn = functype(int) -> int;
IntFn getFunc() { return addOne; }

enum MyError { Failure };
void throwingVoid() throws MyError {}
int throwingInt() throws MyError { return 1; }

[numthreads(1, 1, 1)]
void main()
{
    int a = 1;
    int b = 2;

    // --- Allowed forms: no diagnostic. ---
    doThing();       // a `void` call
    a = b;           // assignment
    a += b;          // compound assignment
    a++;             // postfix increment
    ++a;             // prefix increment
    (void)compute(); // explicit cast to `void` discards the result on purpose
    b = compute();   // assignment whose result (non-`void`) is not itself discarded

    // --- Case (1): disallowed statement form (a bare variable). The `int` result is also
    // discarded, so case (2) fires too; the syntactic case (1) is not suppressed by it. ---
    a;
/*diag:
    ^ this form of expression should not be used as a statement
    ^ the result of this expression is not used; did you mean to call it with '()'?
    ^ result of this expression is not used
    ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // --- Case (1) only: a bare unapplied function reference. Its `FuncType` result is case (1)'s
    // domain, so case (2) is intentionally suppressed to keep the "did you mean to call it?" hint. ---
    addOne;
/*diag:
    ^^^^^^ this form of expression should not be used as a statement
    ^^^^^^ the result of this expression is not used; did you mean to call it with '()'?
*/

    // --- Case (2) only: a call is an allowed form (case 1 stays quiet), but its `int` result is
    // discarded. ---
    compute();
/*diag:
           ^ result of this expression is not used
           ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // --- Case (2) only: a genuine call whose *result type* is a function (`functype`). Unlike the
    // bare `addOne;` reference above (case 1's domain, no case 2), this is a call that discards a
    // real result, so it warns — the `FuncType` case-2 carve-out applies only to a bare reference,
    // not to a call that returns a function value. ---
    getFunc();
/*diag:
           ^ result of this expression is not used
           ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // --- Case (2): a non-`void` cast is diagnosed; the cast form itself is allowed (case 1 stays
    // quiet), matching `(void)compute();` above being silent. ---
    (int)compute();
/*diag:
    ^ result of this expression is not used
    ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // --- Case (3): a bare type name is a clean error, not a crash (was `unexpected: TypeType`,
    // shader-slang/slang#12433). The disallowed-form warning (case 1) also fires; the error's
    // `ErrorType` result suppresses the case-2 warning. ---
    MyType;
/*diag:
    ^^^^^^ this form of expression should not be used as a statement
    ^^^^^^ the result of this expression is not used; did you mean to call it with '()'?
    ^^^^^^ this expression does not name a value
    ^^^^^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
*/

    MyAlias;
/*diag:
    ^^^^^^^ this form of expression should not be used as a statement
    ^^^^^^^ the result of this expression is not used; did you mean to call it with '()'?
    ^^^^^^^ this expression does not name a value
    ^^^^^^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
*/

    RWStructuredBuffer;
/*diag:
    ^^^^^^^^^^^^^^^^^^ this form of expression should not be used as a statement
    ^^^^^^^^^^^^^^^^^^ the result of this expression is not used; did you mean to call it with '()'?
    ^^^^^^^^^^^^^^^^^^ this expression does not name a value
    ^^^^^^^^^^^^^^^^^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
*/
}

// A `try` clause is transparent to the statement-form checks: `try f();` is classified by the
// call it wraps, not flagged as a disallowed bare form (case 1). A `try` on a `void` call is a
// clean statement, while a `try` on a non-`void` call still discards a result (case 2).
void tryStatements() throws MyError
{
    // Allowed: a `try` on a `void` call is an ordinary effectful statement.
    try throwingVoid();

    // Case (2): a `try` on a non-`void` call still discards the call's result.
    try throwingInt();
/*diag:
                   ^ result of this expression is not used
                   ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/
}

// Case (3) applies to a discarded expression wherever it appears, not only to a top-level
// expression statement: a `for` loop's side-effect expression naming a type is a clean error, not
// a crash (it too reached IR lowering as `unexpected: TypeType` before this fix).
void forSideEffectType()
{
    for (int i = 0; i < 1; MyType)
/*diag:
                           ^^^^^^ this expression does not name a value
                           ^^^^^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
*/
    {
    }
}

namespace ns { int f() { return 0; } }

// Case (3) is not limited to type names: a bare namespace name is also not a value, and likewise
// crashed IR lowering (`unexpected: NamespaceType`) before this fix.
void namespaceAsStatement()
{
    ns;
/*diag:
    ^^ this form of expression should not be used as a statement
    ^^ the result of this expression is not used; did you mean to call it with '()'?
    ^^ this expression does not name a value
    ^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
*/
}

// The comma operator sequences several expressions; each operand is in a discarded, statement-like
// position, so the checks recurse into every operand rather than looking only at the whole comma.
void commaOperands()
{
    int a = 0;

    // A disallowed form (bare variable) in a non-final comma operand is still diagnosed (case 1),
    // and its discarded `int` result draws case 2, exactly as a bare `a;` statement would. The
    // trailing `void` call is an allowed form with no result, so it draws neither. (The comma
    // operator itself is separately flagged by the pre-existing E41024.)
    a, doThing();
/*diag:
    ^ this form of expression should not be used as a statement
    ^ the result of this expression is not used; did you mean to call it with '()'?
     ^ comma operator used in expression
     ^ comma operator used in expression (may be unintended)
    ^ result of this expression is not used
    ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/

    // A type name in a NON-final comma operand is reached only by the per-leaf case-3 check (the
    // whole comma's type comes from its final operand `a`), so this is the case that would ICE at
    // lowering (`unexpected: TypeType`) without that per-leaf handling. The `MyType` operand draws
    // case 1 + case 3; the trailing bare `a` draws case 1 + case 2, as a bare `a;` would.
    MyType, a;
/*diag:
    ^^^^^^ this form of expression should not be used as a statement
    ^^^^^^ the result of this expression is not used; did you mean to call it with '()'?
    ^^^^^^ this expression does not name a value
    ^^^^^^ this expression names a type or namespace, not a value, and cannot be used where a value is expected.
          ^ comma operator used in expression
          ^ comma operator used in expression (may be unintended)
            ^ this form of expression should not be used as a statement
            ^ the result of this expression is not used; did you mean to call it with '()'?
            ^ result of this expression is not used
            ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/
}

// The result of a call whose value flows out only through an `out` parameter (a `frexp`-style
// signature) is still a discarded non-`void` result and is diagnosed (case 2), per the design note
// that such calls are not exempted.
void frexpStyleDiscard()
{
    float m = 2.0;
    int e;
    frexp(m, e);
/*diag:
         ^ result of this expression is not used
         ^ the result of this expression is not used; assign it to a variable or discard it explicitly with '(void)'.
*/
}
