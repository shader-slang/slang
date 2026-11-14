// Companion to `operator-declared-as-member.slang`: the declarations that the member-operator error
// (E30073) must NOT flag. Exhaustive mode (no `non-exhaustive`) fails if any of these emits a
// diagnostic. Covered: `operator()` and `operator[]`/`__subscript` (both resolved by member lookup
// on the operand type, so legitimately declared as members, and neither is an actual
// prefix/postfix/infix operator), and free operator functions at module and namespace scope (the
// reachable spelling the error steers users toward).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `operator()` is resolved by member lookup on the callee type, so a member declaration is valid.
struct Callable
{
    int operator()(int a) { return a; }
}

// `__subscript` / `operator[]` is resolved by member lookup on the base type.
struct Indexable
{
    int data[4];
    __subscript(int i) -> int { get { return data[i]; } }
}

// Free operator functions at module scope are the reachable form.
struct Vec2 { float x; float y; }
Vec2 operator+(Vec2 a, Vec2 b) { return { a.x + b.x, a.y + b.y }; }
Vec2 operator-(Vec2 a) { return { -a.x, -a.y }; }
T operator*<T>(T a, T b) { return a; }

// A free operator declared at namespace scope is also fine: `getParentAggTypeDeclBase` is null for
// a namespace, so the module/namespace-vs-type boundary is exercised, not merely assumed.
namespace ns
{
struct W { int v; }
W operator+(W a, W b) { W r; r.v = a.v + b.v; return r; }
}

int main()
{
    Callable c = {};
    int r = c(3);

    Indexable ix = { { 10, 20, 30, 40 } };
    r += ix[0];

    Vec2 u = { 1.0, 2.0 };
    Vec2 v = { 3.0, 4.0 };
    Vec2 s = u + v;
    Vec2 n = -u;

    return r + int(s.x + n.y);
}
