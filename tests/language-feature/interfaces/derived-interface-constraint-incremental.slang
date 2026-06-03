//TEST:INTERPRET(filecheck=CHECK):

// Constraints that *incrementally introduce* the facets later constraints
// depend on:
//
//   __constraint A     : IFoo;   // introduces `A.B`
//   __constraint A.B   : IHasC;  // `A.B` now meaningful; introduces `A.B.C`
//   __constraint A.B.C : IBar;   // `A.B.C` now meaningful
//
// `A` is otherwise unconstrained, so `A.B` is meaningful only via `A : IFoo`,
// and `A.B.C` only via `A.B : IHasC`. Resolving the deeper constraint's subject
// thus requires the shallower constraints to have already been incorporated
// into the inheritance graph. This works because each constraint's endpoints are
// resolved during header checking before the inheritance scan that consults it,
// so when an associated type's inheritance is computed the only sibling whose
// subject is still unresolved is one that genuinely reaches back through the
// type currently being computed (which is correctly skipped, not force-checked).
//
// Constraints must be written in this dependency order: a constraint that
// reaches *through* a facet has to follow the constraint that introduces that
// facet. (A future improvement could make the checker order-independent here.)

interface IBar
{
    int bar();
}
interface IHasC
{
    associatedtype C : IBar;
    C getC();
}
interface IFoo
{
    associatedtype B;
    B getB();
}
interface IHas
{
    associatedtype A;
    A getA();

    __constraint A : IFoo;
    __constraint A.B : IHasC;
    __constraint A.B.C : IBar;
}

struct CV : IBar
{
    int n;
    int bar() { return n; }
}
struct BV : IHasC
{
    typealias C = CV;
    CV getC() { CV c; c.n = 42; return c; }
}
struct AV : IFoo
{
    typealias B = BV;
    BV getB() { BV b; return b; }
}
struct Root : IHas
{
    typealias A = AV;
    AV getA() { AV a; return a; }
}

// `t.getA().getB().getC().bar()` is `T.A.B.C.bar()`; calling `bar()` relies on
// the `A.B.C : IBar` constraint, which in turn relies on the other two.
int deep<T : IHas>(T t) { return t.getA().getB().getC().bar(); }

void main()
{
    Root r;
    // CHECK: 42
    printf("%d\n", deep(r));
}
