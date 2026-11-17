// Regression test for #13509: mutual equality constraints between two
// same-depth abstract associated-type accesses (`C.Primitive == D.Primitive`
// and the reverse) must not send the inheritance computation of the nested
// access into a cycle — the body must check, the compilation must terminate,
// and the concrete instantiation must produce the right value. The member
// call here resolves through the declared bounds (the directed path); the
// equalities add mutual bases at the `C.Primitive` level, so the frames of
// `C.Primitive.Attributes` and `D.Primitive.Attributes` see each other's
// facets while a projection is attempted.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=buf
RWStructuredBuffer<uint> buf;

interface IPayload
{
    uint payload();
}

interface IAttr
{
    associatedtype Data : IPayload;
}

interface IPrim
{
    associatedtype Attributes : IAttr;
}

interface ICtx
{
    associatedtype Primitive : IPrim;
}

struct PayloadImpl : IPayload
{
    uint payload() { return 7; }
}

struct AttrImpl : IAttr
{
    typealias Data = PayloadImpl;
}

struct PrimImpl : IPrim
{
    typealias Attributes = AttrImpl;
}

struct CtxA : ICtx
{
    typealias Primitive = PrimImpl;
}

struct CtxB : ICtx
{
    typealias Primitive = PrimImpl;
}

struct S<C : ICtx>
{
    C.Primitive.Attributes.Data d;
}

uint readMutual<C, D>(S<C> s)
    where C : ICtx
    where D : ICtx
    where C.Primitive == D.Primitive
    where D.Primitive == C.Primitive
{
    return s.d.payload();
}

[numthreads(1, 1, 1)]
void computeMain()
{
    S<CtxA> sa;
    sa.d = PayloadImpl();
    buf[0] = readMutual<CtxA, CtxB>(sa);
}

//CHECK: 7
