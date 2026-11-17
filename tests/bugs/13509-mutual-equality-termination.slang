// White-box termination guard for the equality-congruence projection
// (shader-slang/slang#13509): mutual equality constraints between two
// same-depth abstract associated-type accesses (`C.Primitive == D.Primitive`
// and the reverse) must not send the inheritance computation of the nested
// access into a cycle. This shape also compiles on master — the member call
// resolves through the declared bounds (the directed path), and the level-1
// cycle is already broken there — so its value is forward-looking: it is the
// one test where the projection runs inside mutually-cycling, in-progress
// frames with an abstract far endpoint, and it fails (a hang or a spurious
// "cyclic inheritance" error) if the projection ever regresses there.

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
