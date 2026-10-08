//TEST:INTERPRET(filecheck=CHECK):

// Locks in this PR's central claim: an associated type's bound written as an
// inheritance clause (`associatedtype A : IFoo`), as a `where` clause
// (`associatedtype A where A : IFoo`), or as an interface-level `__constraint`
// (`__constraint A : IFoo` on a derived interface) all produce the same
// representation and therefore the same observable behavior. A future refactor
// that desugared one of the three forms differently would break this test.

interface IFoo
{
    int val();
}

// Form 1: inheritance-clause bound.
interface IViaColon
{
    associatedtype A : IFoo;
    A make();
}

// Form 2: where-clause bound.
interface IViaWhere
{
    associatedtype A where A : IFoo;
    A make();
}

// Form 3: interface-level `__constraint` on a derived interface, constraining
// an associated type inherited from a base.
interface IViaConstraintBase
{
    associatedtype A;
    A make();
}
interface IViaConstraint : IViaConstraintBase
{
    __constraint A : IFoo;
}

// One generic helper per form; each relies on `A : IFoo` to call `val()`.
int useColon<T : IViaColon>(T t)
{
    return t.make().val();
}
int useWhere<T : IViaWhere>(T t)
{
    return t.make().val();
}
int useConstraint<T : IViaConstraint>(T t)
{
    return t.make().val();
}

struct Five : IFoo
{
    int val() { return 5; }
}

struct CColon : IViaColon
{
    typealias A = Five;
    Five make() { Five v; return v; }
}
struct CWhere : IViaWhere
{
    typealias A = Five;
    Five make() { Five v; return v; }
}
struct CConstraint : IViaConstraint
{
    typealias A = Five;
    Five make() { Five v; return v; }
}

void main()
{
    CColon a;
    CWhere b;
    CConstraint c;

    // All three forms resolve `A : IFoo` identically and call `val() == 5`.
    // CHECK: 5 5 5
    printf("%d %d %d\n", useColon(a), useWhere(b), useConstraint(c));
}
