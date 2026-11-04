//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// When a member is found on a pointer level, the checks that depend on the base use that
// level: a mutating member needs an l-value pointer, and a pointee member hides the pointer
// levels even when it is not accessible. Only `.` searches the pointer levels: `->` on an
// interface pointer and `::` do not.

interface IFoo
{
    int getVal();
}

extension Ptr<int>
{
    [mutating]
    void advance() { this = this + 1; }
    static int answer() { return 77; }
}

extension<T> Ptr<T>
{
    int tag() { return 10; }
}

struct S
{
    private int m() { return 0; }
}

extension Ptr<S>
{
    int m() { return 1; }
}

ConstantBuffer<Ptr<int>> cb;

void rvalueBase(Ptr<int> p)
{
    (p + 1).advance();
//CHECK:           ^ mutating method cannot be called on immutable value
//CHECK:           ^ mutating method 'advance' cannot be called on an immutable value
}

void immutableBufferBase()
{
    cb.advance();
//CHECK:      ^ mutating method cannot be called on immutable value
//CHECK:      ^ mutating method 'advance' cannot be called on an immutable value
}

int inaccessiblePointeeMember(Ptr<S> ps)
{
    return ps.m();
//CHECK:      ^ declaration not accessible
//CHECK:      ^ 'm' is not accessible from the current context.
}

int arrowOnInterfacePointer(IFoo* pf)
{
    return pf.tag() + pf->tag();
//CHECK:                  ^^^ member not found
//CHECK:                  ^^^ 'tag' is not a member of
}

int staticMemberWithColons(Ptr<int> p)
{
    return p.answer() + p::answer();
//CHECK:                   ^^^^^^ member not found
//CHECK:                   ^^^^^^ 'answer' is not a member of 'int'.
}
