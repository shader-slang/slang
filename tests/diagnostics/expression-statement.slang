//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

// Diagnostics for an expression used in a statement-like context (a position where its value is
// ignored and it is evaluated only for side effects):
//   - warning E30073: the expression's form cannot be used as a statement.
//   - warning E30074: the expression computes a non-`void` result that is then discarded.
//   - error   E30088: the expression names a type, generic, or namespace, not a value.

struct MyType { int x; };
typealias MyAlias = MyType;

int addOne(int x) { return x + 1; }
void doThing() {}
int compute() { return 1; }

typealias IntFn = functype(int) -> int;
IntFn getFunc() { return addOne; }

enum MyEnum { A, B };

[NoDiscard]
int mustUse() { return 1; }

[DiscardableResult]
int mayDiscard() { return 1; }

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

    // --- A bare variable is not an appropriate statement form (case 1). Its `int` result is also
    // discarded, so case (2) fires too; the syntactic case (1) is not suppressed by it. ---
    a;
/*diag:
    ^ expression cannot be used as a statement
    ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^ result of this expression is not used
    ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/

    // --- A bare unapplied function reference: an inappropriate form (case 1), a discarded result
    // (case 2), plus the function-type follow-up hint (keyed on the result type only). ---
    addOne;
/*diag:
    ^^^^^^ expression cannot be used as a statement
    ^^^^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^^^^^^ result of this expression is not used
    ^^^^^^ the result of this expression is not used; consider binding it to a variable with 'let'.
    ^^^^^^ expression is of function type
    ^^^^^^ expression is of function type; perhaps a call was intended?
*/

    // --- A call is an appropriate form (case 1 stays quiet), but its `int` result is discarded. ---
    compute();
/*diag:
           ^ result of this expression is not used
           ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/

    // --- A genuine call whose *result type* is a function (`functype`). This is an appropriate
    // form (a call), discards a real result (case 2), and the function-type follow-up hint fires
    // because the discarded value is of function type. ---
    getFunc();
/*diag:
           ^ result of this expression is not used
           ^ the result of this expression is not used; consider binding it to a variable with 'let'.
           ^ expression is of function type
           ^ expression is of function type; perhaps a call was intended?
*/

    // --- A cast to a function type also draws the function-type hint (type-only, no form gate).
    // The cast is an appropriate form (case 1 stays quiet), so only the discarded-result diagnostics
    // and the hint fire, keyed on the cast's operand. ---
    (IntFn)addOne;
/*diag:
           ^^^^^^ result of this expression is not used
           ^^^^^^ the result of this expression is not used; consider binding it to a variable with 'let'.
           ^^^^^^ expression is of function type
           ^^^^^^ expression is of function type; perhaps a call was intended?
*/

    // --- A non-`void` cast is an appropriate form (case 1 stays quiet), but discards its result
    // (case 2), matching `(void)compute();` above being silent. ---
    (int)compute();
/*diag:
    ^ result of this expression is not used
    ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/

    // --- A constructor-style enum conversion is a cast: an appropriate form, discards its result. ---
    MyEnum(a);
/*diag:
           ^ result of this expression is not used
           ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/

    // --- A discarded `==` (a likely mistyped `=`) draws its own dedicated diagnostic instead of the
    // generic discarded-result warning. A comparison is not an appropriate statement form, so the
    // case-1 warning also fires. ---
    a == b;
/*diag:
      ^^ expression cannot be used as a statement
      ^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
      ^^ result of '==' not used
      ^^ result of '==' not used, did you intend '='?
*/

    // --- A call to a `[NoDiscard]` function whose result is discarded is an error. ---
    mustUse();
/*diag:
           ^ result of '[NoDiscard]' function is discarded
           ^ the result of calling 'mustUse' is discarded; this function is marked '[NoDiscard]'.
*/

    // --- A call to a `[DiscardableResult]` function may discard its result silently. ---
    mayDiscard();

    // --- Case (3): a bare type name is a clean error, not an internal-compiler crash. The
    // inappropriate-form warning (case 1) also fires; the error's `ErrorType` result suppresses the
    // case-2 warning. ---
    MyType;
/*diag:
    ^^^^^^ expression cannot be used as a statement
    ^^^^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^^^^^^ expression does not have a proper type
    ^^^^^^ a proper expression was expected, but the expression names a type.
*/

    MyAlias;
/*diag:
    ^^^^^^^ expression cannot be used as a statement
    ^^^^^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^^^^^^^ expression does not have a proper type
    ^^^^^^^ a proper expression was expected, but the expression names a type.
*/

    // --- Case (3): an unapplied generic used as a value checks to a type-valued expression, so it
    // is reported as naming a type. ---
    RWStructuredBuffer;
/*diag:
    ^^^^^^^^^^^^^^^^^^ expression cannot be used as a statement
    ^^^^^^^^^^^^^^^^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^^^^^^^^^^^^^^^^^^ expression does not have a proper type
    ^^^^^^^^^^^^^^^^^^ a proper expression was expected, but the expression names a type.
*/
}

// A ternary `?:` is diagnosed transparently, per result-flowing arm, rather than as a whole. Each
// arm is checked in the enclosing discarded position: the `(a = 1)` arm is an assignment (an
// appropriate form, so neither case 1 nor case 2 fires on it), while the `compute()` arm is a call
// whose non-`void` result is discarded (case 2). The `?:` itself is not flagged as case 1.
void ternaryStatement(bool cond)
{
    int a = 0;
    a > 0 ? (a = 1) : compute();
/*diag:
                             ^ result of this expression is not used
                             ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}

// A short-circuit `&&`/`||` is likewise diagnosed transparently, per its result-flowing right
// operand, rather than as a whole. For `&&`, the right operand `(a = 1)` is an assignment (an
// appropriate form), so it draws no case-1 warning; only its discarded `bool` result draws case 2
// (alongside the pre-existing, unrelated `int`-to-`bool` conversion warning). For `||`, the right
// operand `compute() > 0` is a comparison (an inappropriate form), so it draws case 1 (reported at
// the `||`) and its discarded `bool` result draws case 2 (reported at the `>`).
void shortCircuitStatement(bool cond)
{
    int a = 0;
    cond && (a = 1);
/*diag:
            ^ implicit conversion not recommended
            ^ implicit conversion from 'int' to 'bool' is not recommended
            ^ result of this expression is not used
            ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
    cond || (compute() > 0);
/*diag:
         ^^ expression cannot be used as a statement
         ^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
                       ^ result of this expression is not used
                       ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}

// An assignment written as the operand of an explicit cast is, before checking, an
// `ExplicitCastExpr` (a cast, not an assignment), so its discarded non-`void` cast result warns.
void castOfAssignment()
{
    int a = 0;
    (int)(a = 1);
/*diag:
            ^ result of this expression is not used
            ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}

// Parenthesized and nested assignments still carve out (parentheses are transparent to the form).
void nestedAssignment()
{
    int a = 0;
    (( ((a = 1)) ));
}

// A `try` clause is transparent to the statement-form checks: `try f();` is classified by the call
// it wraps, not flagged as an inappropriate form (case 1). A `try` on a `void` call is a clean
// statement, while a `try` on a non-`void` call still discards a result (case 2). An assignment
// under `try` still carves out.
void tryStatements() throws MyError
{
    // Allowed: a `try` on a `void` call is an ordinary effectful statement.
    try throwingVoid();

    // Case (2): a `try` on a non-`void` call still discards the call's result.
    try throwingInt();
/*diag:
                   ^ result of this expression is not used
                   ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}

// Case (3) applies to a discarded expression wherever it appears, not only a top-level expression
// statement: a `for` loop's side-effect expression naming a type is a clean error.
void forSideEffectType()
{
    for (int i = 0; i < 1; MyType)
/*diag:
                           ^^^^^^ expression cannot be used as a statement
                           ^^^^^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
                           ^^^^^^ expression does not have a proper type
                           ^^^^^^ a proper expression was expected, but the expression names a type.
*/
    {
    }
}

// A `for` loop's side-effect expression is a statement-like context subject to the same rules, so a
// side-effect expression that discards a non-`void` result now warns (case 2), while a comma of
// effectful updates is permitted without the comma-usage warning.
void forSideEffectDiscard()
{
    for (int i = 0; i < 1; compute())
/*diag:
                                  ^ result of this expression is not used
                                  ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
    {
    }

    for (int i = 0, j = 0; i < 1; i++, j++)
    {
    }
}

namespace ns { int f() { return 0; } }

// Case (3) is not limited to type names: a bare namespace name is also not a value.
void namespaceAsStatement()
{
    ns;
/*diag:
    ^^ expression cannot be used as a statement
    ^^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^^ expression does not have a proper type
    ^^ a proper expression was expected, but the expression names a namespace.
*/
}

// The comma operator sequences several expressions; every operand but the last is in a discarded,
// statement-like position, and the last operand carries the comma's result, so it too is discarded
// in a statement context.
void commaOperands()
{
    int a = 0;

    // A bare variable as the last comma operand is an inappropriate form (case 1) whose discarded
    // `int` result draws case 2, exactly as a bare `a;` statement would. (The comma operator itself
    // is separately flagged by the pre-existing E41024.)
    a, doThing();
/*diag:
    ^ expression cannot be used as a statement
    ^ an expression used as a statement must be a function call (including a cast), an assignment, a compound assignment, an increment or decrement, an inline 'spirv_asm' block, or an 'expand' of one of these.
    ^ result of this expression is not used
    ^ the result of this expression is not used; consider binding it to a variable with 'let'.
     ^ comma operator used in expression
     ^ comma operator used in expression (may be unintended)
*/
}

// The result of a call whose value flows out only through an `out` parameter (a `frexp`-style
// signature) is still a discarded non-`void` result and is diagnosed (case 2).
void frexpStyleDiscard()
{
    float m = 2.0;
    int e;
    frexp(m, e);
/*diag:
         ^ result of this expression is not used
         ^ the result of this expression is not used; consider binding it to a variable with 'let'.
*/
}
