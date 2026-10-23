// Regression test for https://github.com/shader-slang/slang/issues/13888.
//
// Name lookup stops at the first scope level that has hits, unless one of those hits is
// overloadable. Two `using namespace` directives in one scope make the named namespaces one
// lookup level, so two same-named variables or types found through them are ambiguous with each
// other, and a same-named declaration in an outer scope is not a candidate. The test checks every
// diagnostic, so a candidate note for an outer declaration makes it fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

namespace A
{
    static const int xyzzyx = 123;
//CHECK:             ^^^^^^ candidate: static const int A::xyzzyx
}

namespace B
{
    static const int xyzzyx = 456;
//CHECK:             ^^^^^^ candidate: static const int B::xyzzyx
}

static const int xyzzyx = 789;

namespace Outer
{
    namespace Inner
    {
        using namespace A;
        using namespace B;

        int getXyzzyx()
        {
            return xyzzyx;
//CHECK:           ^^^^^^ ambiguous reference to 'xyzzyx'
        }
    }
}

// The same lookup with `C` and `D` nested in a namespace that does not contain the outer
// variable `plugh`. Unlike the first case, its expected result does not depend on how members
// named by `using namespace` rank against the global namespace (shader-slang/slang#13750).
static const int plugh = 789;

namespace Nested
{
    namespace C
    {
        static const int plugh = 123;
//CHECK:                 ^^^^^ candidate: static const int Nested::C::plugh
    }

    namespace D
    {
        static const int plugh = 456;
//CHECK:                 ^^^^^ candidate: static const int Nested::D::plugh
    }

    namespace Inner
    {
        using namespace C;
        using namespace D;

        int getPlugh()
        {
            return plugh;
//CHECK:           ^^^^^ ambiguous reference to 'plugh'
        }
    }
}

// Two same-named struct types at one level hide the outer struct type.
struct S
{
    int c;
}

namespace Types
{
    namespace TA
    {
        struct S
//CHECK:       ^ candidate: struct Types::TA::S
        {
            int a;
        }
    }

    namespace TB
    {
        struct S
//CHECK:       ^ candidate: struct Types::TB::S
        {
            int b;
        }
    }

    namespace Inner
    {
        using namespace TA;
        using namespace TB;

        int get()
        {
            S s;
//CHECK:    ^ ambiguous reference to 'S'
            return 0;
        }
    }
}

// Two same-named variables at one level hide the outer function `h`, so `h(7)` is an ambiguous
// reference to the two variables. In the same way, a single variable `k` hides the outer function
// `k`, and `k(7)` is an error.
int h(int x)
{
    return x;
}

int k(int x)
{
    return x;
}

namespace Calls
{
    namespace HA
    {
        static const int h = 1;
//CHECK:                 ^ candidate: static const int Calls::HA::h
    }

    namespace HB
    {
        static const int h = 2;
//CHECK:                 ^ candidate: static const int Calls::HB::h
    }

    namespace HC
    {
        static const int k = 3;
    }

    namespace Inner
    {
        using namespace HA;
        using namespace HB;

        int getH()
        {
            return h(7);
//CHECK:           ^ ambiguous reference to 'h'
        }
    }

    namespace SingleInner
    {
        using namespace HC;

        int getK()
        {
            return k(7);
//CHECK:            ^ E30016
//CHECK:            ^ no call operation found for type 'int'
        }
    }
}

// Two same-named variables at one level also hide an outer struct type, so the name in a type
// position does not refer to the type. A single variable already hides the outer type this way.
struct Shape
{
    int c;
}

namespace TypePosition
{
    namespace SA
    {
        static const int Shape = 1;
    }

    namespace SB
    {
        static const int Shape = 2;
    }

    namespace Inner
    {
        using namespace SA;
        using namespace SB;

        int get()
        {
            Shape s;
//CHECK:    ^^^^^ E30060
//CHECK:    ^^^^^ expected a type, got a 'overload group' for 'Shape'
            return 0;
        }
    }
}
