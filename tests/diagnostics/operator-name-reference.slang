// Diagnostic coverage for shader-slang/slang#12971. An `operator <op>` name is allowed after `::`
// in expression position, and two diagnostics must fire on that path:
//   * `Vec2::operator+(...)` -- a `::`-qualified reference to a non-static member operator -- must
//     report a static-reference-to-non-static-member error (E30100), not silently mis-parse. This
//     is analogous to the non-static-member coverage the `__subscript::` path has in
//     `subscript-accessor-reference.slang`.
//   * `ns::operator@(...)` -- a malformed operator spelling after `::` -- must report the same
//     invalid-operator error (E20008) it does in declaration position. This case is
//     operator-name-specific; the `__subscript::` path has no analog.
// The well-formed positive forms are covered in
// `tests/language-feature/operator-overload/operator-name-as-expr.slang`.
//
// Diagnostics are emitted parse-phase-first, so E20008 precedes E30100 regardless of source order;
// the CHECK lines follow that order.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Vec2
{
    float x;
    Vec2 operator+(Vec2 b)
    {
        Vec2 r = {x + b.x};
        return r;
    }
}

namespace ns
{
    int add(int a, int b) { return a + b; }
}

void f(Vec2 a, Vec2 b)
{
    // Malformed operator spelling after `::` reaches `ParseDeclName`'s operator-switch `default`
    // branch -- the same invalid-operator diagnostic emitted in declaration position.
    int d = ns::operator@(1, 2);
    //CHECK: E20008

    // `+` is an instance member of Vec2, so naming it through the type is a static reference to a
    // non-static member -- it must diagnose, not mis-parse as `(Vec2::operator) + (a, b)`.
    Vec2 c = Vec2::operator+(a, b);
    //CHECK: E30100
}
