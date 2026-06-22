// operator-name-on-variable.slang
//
// Regression test for https://github.com/shader-slang/slang/issues/11664
//
// An `operator <op>` name is only valid when declaring an operator function.
// Using it for any other kind of declaration (a variable, a parameter, a
// `typedef`, or a property) must be a syntax error (E20020), matching the
// modern `let`/`var` syntax which already rejects it. The rejection is enforced
// at a single chokepoint (`UnwrapDeclarator`), so every legal operator-function
// form must still compile — the positive cases below guard against that.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

struct Vec2
{
    float x;
    float y;
}

// Positive: a real operator-function declaration is routed to the function-decl
// path, which opts in to the operator name, so it stays valid (no diagnostic).
Vec2 operator+(Vec2 a, Vec2 b)
{
    return {a.x + b.x, a.y + b.y};
}

// Positive: a *generic* operator function must also still compile — it is
// disambiguated by the trailing `<` and likewise takes the function path.
T operator-<T>(T a, T b)
{
    return a;
}

// Positive: the call operator `operator()` (whose name token consumes `()`)
// must still compile as a member function.
struct Callable
{
    int operator()(int x)
    {
        return x;
    }
}

// Negative: an operator name used as a variable name. A local variable such as
// `int operator+ = 10;` inside a function body reaches the same chokepoint, so
// this also covers the reported repro.
int operator+ = 10;
//CHK: E20020
//CHK: 'operator+' can only be used to declare an operator function.

// Negative: an operator name used as a function parameter name.
void g(int operator*)
//CHK: E20020
//CHK: 'operator*' can only be used to declare an operator function.
{
}

// Negative: operator name as the *second* declarator in a multi-declarator list.
// The clean declarator (`x`) must stay valid; only `operator/` errors. A distinct
// operator is used for each negative case so the diagnostic is not collapsed with
// another by the sink's identical-message deduplication.
int x, operator/;
//CHK: E20020
//CHK: 'operator/' can only be used to declare an operator function.

// Negative: operator name as the *first* declarator in a list; the trailing
// declarator (`y`) must stay valid (guards against the operator-name provenance
// leaking from one declarator onto the next in the shared DeclaratorInfo).
int operator%, y;
//CHK: E20020
//CHK: 'operator%' can only be used to declare an operator function.

// Negative: an operator name used for a `typedef` (a loophole the previous,
// variable/parameter-only rejection left open).
typedef int operator-;
//CHK: E20020
//CHK: 'operator-' can only be used to declare an operator function.

// Negative: an operator name used for a (traditional-syntax) property.
struct H
{
    property int operator&
    {
        get { return 0; }
    }
    //CHK: E20020
    //CHK: 'operator&' can only be used to declare an operator function.
}
