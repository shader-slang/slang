//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -experimental-feature

// Test that the potential-aliasing warnings cover an argument that
// aliases the receiver of a `[mutating]` or `[__ref]` method, reported
// as W30134, and arguments whose access path is a prefix of another
// argument's access path, reported as W30051 (issue #13758). The
// receiver is `inout` storage for `[mutating]` and `__ref` storage for
// `[__ref]`, so the mode rules for explicit arguments apply to the
// receiver's *effective* mode: a receiver whose effective mode is `in`
// (a `[nonmutating]` method, or a plain method on a copyable type)
// never warns, a plain method on a non-copyable type is effectively
// borrowed and follows the borrow rules, and a `__ref` receiver paired
// with a `__ref` argument has opted into reference semantics and never
// warns. Extension steps that do not name storage inside their base —
// a property (mediated by accessors through a temporary) most notably —
// do not prove aliasing.

struct S
{
    int v;

    [mutating] void mutInout(inout S other) { v = 5; other.v = 7; }
    [mutating] int mutBorrow(__borrow S other) { v = 5; return other.v; }
    [__ref] int refBorrow(__borrow S other) { v = 5; return other.v; }
    [mutating] void mutInoutField(inout int other) { v = 5; other = 7; }
    [mutating] void setPtr(inout S* other) { v = 5; }
    [__ref] void refInOut(inout S other) { v = 5; other.v = 7; }
    [__ref] void refRef(__ref S other) { v = 5; other.v = 7; }
    [nonmutating] void nonmutInout(inout S other) { other.v = 7; }
    void defaultInout(inout S other) { other.v = 7; }
}

struct Box { S s; }

// A nested type's method can name its enclosing type, which lets the test
// put the receiver deeper in the access path than the argument.
struct Outer
{
    int v;

    struct Inner
    {
        int w;

        [mutating] void mutInout(inout Outer other) { w = 5; other.v = 7; }
    }

    Inner inner;
}

// A method without a receiver-mode specifier on a non-copyable type is
// effectively borrowed, like a `__borrow` parameter.
[__NonCopyableType] struct NC
{
    int v;

    void plainBorrow(inout int x) { x = v; }
}

// A property is mediated by accessors; an argument naming it binds
// through a temporary, not through storage inside the base object.
struct P
{
    int v;

    property int p { get { return v; } set { v = newValue; } }
    [mutating] void mutBorrowProp(__borrow int x) { v = x; }
    [mutating] void mutInoutProp(inout int x) { v = x; }
}

struct IntArr { int e[2]; }

static int gIdx = 1;

void twoInout(inout S a, inout S b) { a.v = 5; b.v = 7; }

void inoutAndInoutPrefix(inout S a, inout int b) { a.v = 5; b = 7; }

// Same parameter modes as `inoutAndInoutPrefix` with the longer access
// path passed first, so the alias check proves the pair through the
// other direction of the prefix test.
void inoutAndInoutPrefixRev(inout int a, inout S b) { a = 7; b.v = 5; }

void inoutBoxAndIntPrefix(inout Box a, inout int b) { a.s.v = 5; b = 7; }

void inoutPAndInoutPrefix(inout P a, inout int b) { a.v = 5; b = 7; }

void arrAndElem(inout IntArr a, inout int b) { a.e[0] = b; }

void inAndInoutPrefix(in S a, inout int b) { b = a.v; }

[numthreads(1,1,1)]
void computeMain()
{
    S s;
    s.v = 1;
    S s2;
    S arrS[2];
    Box box;
    Outer outer;
    P p;
    p.v = 2;
    NC nc;
    nc.v = 3;
    IntArr arrObj;

    // Positive control: the same storage to two inout parameters warns (#10699).
    twoInout(s, s);
//CHECK:     ^ potentially aliased argument to a parameter that may be written
//CHECK:     ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior

    // Positive: a `[mutating]` receiver is `inout` storage the call may write,
    // so an argument naming the receiver, or a subobject of it, warns. A
    // `[__ref]` receiver paired with an `inout` argument warns too: only a
    // ref/ref pair has opted into reference semantics. The receiver may be
    // deeper in the access path than the argument (`outer.inner` vs `outer`),
    // and an array element is a receiver like any other l-value.
    {
        s.mutInout(s);
//CHECK:^ argument may alias the receiver of a method that may write it
//CHECK:^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        s.mutBorrow(s);
//CHECK:^ argument may alias the receiver of a method that may write it
//CHECK:^ argument for 'borrow' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        s.mutInoutField(s.v);
//CHECK:^ argument may alias the receiver of a method that may write it
//CHECK:^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        s.refBorrow(s);
//CHECK:^ argument may alias the receiver of a method that may write it
//CHECK:^ argument for 'borrow' parameter 'other' may alias the receiver of the call, passed 'ref'; passing the same value to both may produce undefined behavior
        s.refInOut(s);
//CHECK:^ argument may alias the receiver of a method that may write it
//CHECK:^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'ref'; passing the same value to both may produce undefined behavior
        arrS[0].mutInout(arrS[0]);
//CHECK:    ^ argument may alias the receiver of a method that may write it
//CHECK:    ^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        arrS[0].mutInoutField(arrS[0].v);
//CHECK:    ^ argument may alias the receiver of a method that may write it
//CHECK:    ^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        outer.inner.mutInout(outer);
//CHECK:      ^^^^^ argument may alias the receiver of a method that may write it
//CHECK:      ^^^^^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
    }

    // Positive: an unannotated method on a non-copyable receiver is
    // effectively borrowed, so it warns like any other writable borrow.
    {
        nc.plainBorrow(nc.v);
//CHECK:^^ argument may alias the receiver of a method that may write it
//CHECK:^^ argument for 'inout' parameter 'x' may alias the receiver of the call, passed 'borrow'; passing the same value to both may produce undefined behavior
    }

    // Positive: an access path that is a prefix of another argument's access
    // path names storage contained in it, so the arguments alias even without a
    // receiver — in either argument order, over multiple steps, and when the
    // extension step carries a dynamic index (`arrObj.e[gIdx]` is inside
    // `arrObj`'s storage for any `gIdx`).
    inoutAndInoutPrefix(s, s.v);
//CHECK:                ^ potentially aliased argument to a parameter that may be written
//CHECK:                ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior
    inoutAndInoutPrefix(arrS[0], arrS[0].v);
//CHECK:                    ^ potentially aliased argument to a parameter that may be written
//CHECK:                    ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior
    inoutAndInoutPrefixRev(s.v, s);
//CHECK:                     ^ potentially aliased argument to a parameter that may be written
//CHECK:                     ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior
    inoutBoxAndIntPrefix(box, box.s.v);
//CHECK:                 ^^^ potentially aliased argument to a parameter that may be written
//CHECK:                 ^^^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior
    arrAndElem(arrObj, arrObj.e[gIdx]);
//CHECK:       ^^^^^^ potentially aliased argument to a parameter that may be written
//CHECK:       ^^^^^^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior

    // Negative: an `in` parameter is a copy taken before the call, so a
    // prefix-aliased argument never warns. Exhaustive mode fails the test if a
    // spurious diagnostic is emitted.
    inAndInoutPrefix(s, s.v);

    // Negative: a receiver whose effective mode is `in` (`[nonmutating]`, or a
    // plain method on a copyable type) never warns, and a `__ref` receiver
    // with a `__ref` argument has opted into reference semantics.
    s.nonmutInout(s);
    s.defaultInout(s);
    s.refRef(s);

    // Negative: distinct storage — a different object, a field of a different
    // object, or a different element of the same array — never warns.
    s2.mutInout(s);
    s.mutInout(s2);
    s.mutInoutField(s2.v);
    arrS[0].mutInout(arrS[1]);

    // Negative: a dynamic index inside the *shared* path is never provable
    // (the identical-path check compares constant indices only), so a dynamic
    // element paired with its own field does not warn.
    inoutAndInoutPrefix(arrS[gIdx], arrS[gIdx].v);

    // Negative: a deref callee (`p->method()`) carries a `DerefExpr`-wrapped
    // pointer as its receiver, which the structural proof cannot see through,
    // so the receiver pair never fires for it — including when the argument is
    // the pointer itself (`p->setPtr(p)`) or the pointee (`p->mutInout(*p)`).
    {
        S* p = &s;
        p->mutInout(s);
        p->mutInout(*p);
        p->setPtr(p);
    }

    // Negative: a property is mediated by accessors, so an argument naming it
    // does not name storage inside its base — neither against the receiver
    // nor in an argument pair.
    p.mutBorrowProp(p.p);
    p.mutInoutProp(p.p);
    inoutPAndInoutPrefix(p, p.p);
}
