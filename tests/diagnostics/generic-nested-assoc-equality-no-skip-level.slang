//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// An equality constraint on a generic parameter (`where C == Evil`) carries
// over to a nested associated-type access only through the access's direct
// source, projecting the whole access path (`C.H.Data == Evil.H.Data`). It must
// NOT project the innermost requirement onto the far endpoint alone: `Evil.Data`
// is unrelated to `C.H.Data`, which can only ever be `GoodData`, so a member
// that exists only on `Evil.Data` stays unresolvable. Regression guard for the
// congruence projection in the lookup-derived inheritance scan (#13509).

interface IHolder
{
    associatedtype Data;
}

interface ICtx
{
    associatedtype H : IHolder;
}

struct GoodData
{
    uint good;
}

struct BadData
{
    uint good;
    uint evil;
}

struct Impl : IHolder
{
    typealias Data = GoodData;
}

struct Evil : IHolder, ICtx
{
    typealias Data = BadData;
    typealias H = Impl;
}

struct S<C : ICtx>
{
    C.H.Data f;
}

uint g<C>(S<C> s)
    where C : ICtx
    where C == Evil
{
    return s.f.good;
}

uint bad<C>(S<C> s)
    where C : ICtx
    where C == Evil
{
    return s.f.evil;
    /*CHECK: 'evil' is not a member of 'C.H.Data'.
    */
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
