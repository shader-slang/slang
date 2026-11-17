//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0): -experimental-feature

// Test that the potential-aliasing warning (W30051) covers an argument that
// aliases the receiver of a `[mutating]` or `[__ref]` method, and arguments
// whose access path is a prefix of another argument's access path (issue
// #13758). The receiver is `inout` storage for `[mutating]` and `__ref`
// storage for `[__ref]`, so the same mode rules as for explicit arguments
// apply: an `in` receiver (`[nonmutating]` or the plain default) never warns,
// and a `__ref` receiver paired with a `__ref` argument has opted into
// reference semantics and never warns.

struct S
{
    int v;

    [mutating] void mutInout(inout S other) { v = 5; other.v = 7; }
    [mutating] int mutBorrow(__borrow S other) { v = 5; return other.v; }
    [__ref] int refBorrow(__borrow S other) { v = 5; return other.v; }
    [mutating] void mutInoutField(inout int other) { v = 5; other = 7; }
    [__ref] void refRef(__ref S other) { v = 5; other.v = 7; }
    [nonmutating] void nonmutInout(inout S other) { other.v = 7; }
    void defaultInout(inout S other) { other.v = 7; }
}

void twoInout(inout S a, inout S b) { a.v = 5; b.v = 7; }

void inoutAndInoutPrefix(inout S a, inout int b) { a.v = 5; b = 7; }

void inAndInoutPrefix(in S a, inout int b) { b = a.v; }

[numthreads(1,1,1)]
void computeMain()
{
    S s;
    s.v = 1;
    S s2;
    S arrS[2];

    // Positive control: the same storage to two inout parameters warns (#10699).
    twoInout(s, s);
//CHECK:     ^ potentially aliased argument to a parameter that may be written
//CHECK:     ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior

    // Positive: a `[mutating]` receiver is `inout` storage the call may write,
    // so an argument naming the receiver, or a subobject of it, warns.
    {
        s.mutInout(s);
//CHECK:^ potentially aliased argument to a method receiver that may be written
//CHECK:^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        s.mutBorrow(s);
//CHECK:^ potentially aliased argument to a method receiver that may be written
//CHECK:^ argument for 'borrow' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
        s.mutInoutField(s.v);
//CHECK:^ potentially aliased argument to a method receiver that may be written
//CHECK:^ argument for 'inout' parameter 'other' may alias the receiver of the call, passed 'inout'; passing the same value to both may produce undefined behavior
    }

    // Positive: a `[__ref]` receiver is reference storage the call may write.
    {
        s.refBorrow(s);
//CHECK:^ potentially aliased argument to a method receiver that may be written
//CHECK:^ argument for 'borrow' parameter 'other' may alias the receiver of the call, passed 'ref'; passing the same value to both may produce undefined behavior
    }

    // Positive: an access path that is a prefix of another argument's access
    // path names storage contained in it, so the arguments alias even without a
    // receiver (`s` and `s.v`, `arrS[0]` and `arrS[0].v`).
    inoutAndInoutPrefix(s, s.v);
//CHECK:                ^ potentially aliased argument to a parameter that may be written
//CHECK:                ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior
    inoutAndInoutPrefix(arrS[0], arrS[0].v);
//CHECK:                    ^ potentially aliased argument to a parameter that may be written
//CHECK:                    ^ argument for 'inout' parameter 'a' may alias argument for 'inout' parameter 'b'; passing the same value to both may produce undefined behavior

    // Negative: an `in` parameter is a copy taken before the call, so a
    // prefix-aliased argument never warns. Exhaustive mode fails the test if a
    // spurious diagnostic is emitted.
    inAndInoutPrefix(s, s.v);

    // Negative: an `in` receiver (`[nonmutating]`, or a plain method's default)
    // never warns, and a `__ref` receiver with a `__ref` argument has opted
    // into reference semantics, like any ref pair.
    s.nonmutInout(s);
    s.defaultInout(s);
    s.refRef(s);

    // Negative: distinct storage — a different object, or a field of a
    // different object — never warns.
    s2.mutInout(s);
    s.mutInout(s2);
    s.mutInoutField(s2.v);
}
