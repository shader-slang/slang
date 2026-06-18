// operator-name-on-variable.slang
//
// Regression test for https://github.com/shader-slang/slang/issues/11664
//
// In traditional C-style declarations the `operator <op>` name form was silently
// accepted as an ordinary variable or parameter name. It is only valid when
// declaring an operator function, so it must now be a syntax error (E20020),
// matching the modern `let`/`var` syntax which already rejects it. A legitimate
// operator-function declaration must still compile.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

struct Vec2
{
    float x;
    float y;
}

// Positive: a real operator-function declaration is routed to the function-decl
// path before the variable branch, so it stays valid and produces no diagnostic.
Vec2 operator+(Vec2 a, Vec2 b)
{
    return {a.x + b.x, a.y + b.y};
}

// Negative: an operator name used as a variable name. A local variable such as
// `int operator+ = 10;` inside a function body reaches the same CompleteVarDecl
// path, so this also covers the reported repro.
int operator+ = 10;
//CHK: E20020
//CHK: 'operator+' can only be used to declare an operator function, not a variable or parameter name.

// Negative: an operator name used as a function parameter name.
void g(int operator*)
//CHK: E20020
//CHK: 'operator*' can only be used to declare an operator function, not a variable or parameter name.
{
}
