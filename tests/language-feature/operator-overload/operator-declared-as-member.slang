// A prefix/postfix/infix operator declared as a member of a type, interface, or extension (static
// or not) is an error (E30073): an operator call site (`a + b`) resolves the operator name by
// lexical scope lookup, not by member lookup on the operand type, so a member operator can never be
// found from a call. The member-looked-up `operator()` and `operator[]`/`__subscript`, and free
// operator functions, remain valid with no diagnostic; those are covered by the exhaustive
// companion test `operator-declared-as-member-allowed.slang`. (shader-slang/slang#12761)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Plain struct member operator. The caret pins that E30073 is attached to the operator
// declaration itself (a `decl:Decl` span), not to some later use.
struct S
{
    int x;
    S operator+(S o) { S r; r.x = x + o.x; return r; }
    //CHECK: error E30073
    //CHECK:  ^ operator '+' cannot be declared as a member
}

// Interface requirement declared with an operator name.
interface IArith
{
    This operator*(This o);
    //CHECK: E30073
}

// Non-generic extension member operator.
struct P { int v; }
extension P
{
    P operator-(P o) { P r; r.v = v - o.v; return r; }
    //CHECK: E30073
}

// Unconstrained generic extension member operator.
struct Box<T> { T value; }
extension<T> Box<T>
{
    Box<T> operator/(Box<T> o) { return this; }
    //CHECK: E30073
}

// Constrained generic extension member operator.
extension<T : IArithmetic> Box<T>
{
    Box<T> operator%(Box<T> o) { return this; }
    //CHECK: E30073
}

// Member prefix operator (unary, uses implicit `this`).
struct N
{
    int v;
    N operator~() { N r; r.v = ~v; return r; }
    //CHECK: E30073
}

// Member postfix operator (`__postfix` marks the fixity; without it the operator would not be
// treated as postfix at all).
struct Q
{
    int v;
    [mutating]
    __postfix Q operator++() { v += 1; return this; }
    //CHECK: E30073
}

// A `static` member operator is rejected just like a non-static one: the rule keys off the
// declaration being a member, not off static-ness.
struct HasStatic
{
    int v;
    static HasStatic operator+(HasStatic a, HasStatic b) { HasStatic r; r.v = a.v + b.v; return r; }
    //CHECK: E30073
}

// Compound-assignment, comma, and ternary operator names are actual operators too and must be
// flagged as members.
struct M
{
    int v;
    [mutating]
    M operator+=(M o) { v += o.v; return this; }
    //CHECK: E30073
    M operator,(M o) { return o; }
    //CHECK: E30073
    //CHECK: operator ',' cannot be declared as a member
    M operator?:(bool c, M a, M b) { return c ? a : b; }
    //CHECK: E30073
    //CHECK: operator '?:' cannot be declared as a member
}

// A generic operator method inside a type is wrapped in a `GenericDecl`; `getParentAggTypeDeclBase`
// walks past it, so the member is still detected.
struct G
{
    int v;
    G operator*<T>(T o) { return this; }
    //CHECK: E30073
}

// A type nested inside another type is still a non-module scope.
struct Outer
{
    struct Inner
    {
        int v;
        Inner operator-(Inner o) { Inner r; r.v = v - o.v; return r; }
        //CHECK: E30073
    }
}

void main()
{
    printf("done\n");
}
