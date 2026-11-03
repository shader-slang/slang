//TEST:SIMPLE: -no-codegen -Wno-38109
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -Wno-38109

// A custom derivative must have the same effective `this` parameter-passing mode as its primal.
// Checking these declarations exercises association. The IR checks below also pin how the
// associated functions are projected onto the AD2 derivative ABI; the derivative bodies do not
// need to be invoked here.

struct BorrowReceiver
{
    float scale;

    [constref]
    [NoDiffThis]
    float eval(float x)
    {
        return scale * x * x;
    }
}

extension BorrowReceiver
{
    [constref]
    [NoDiffThis]
    [ForwardDerivativeOf(eval)]
    DifferentialPair<float> evalForward(DifferentialPair<float> x)
    {
        return diffPair(scale * x.p * x.p, scale * 2.0 * x.p * x.d);
    }

    [constref]
    [NoDiffThis]
    [BackwardDerivativeOf(eval)]
    void evalBackward(inout DifferentialPair<float> x, float dOut)
    {
        x = diffPair(x.p, x.d + scale * 2.0 * x.p * dOut);
    }
}

struct RefReceiver
{
    float scale;

    [__ref]
    [NoDiffThis]
    float eval(float x)
    {
        return scale * x * x;
    }
}

extension RefReceiver
{
    [__ref]
    [NoDiffThis]
    [ForwardDerivativeOf(eval)]
    DifferentialPair<float> evalForward(DifferentialPair<float> x)
    {
        return diffPair(scale * x.p * x.p, scale * 2.0 * x.p * x.d);
    }

    [__ref]
    [NoDiffThis]
    [BackwardDerivativeOf(eval)]
    void evalBackward(inout DifferentialPair<float> x, float dOut)
    {
        x = diffPair(x.p, x.d + scale * 2.0 * x.p * dOut);
    }
}

// A non-copyable receiver adjusts its declared In default to BorrowIn for primal calls. The current
// derivative-ABI compatibility projection processes that mode with the In rules: the forward
// derivative receives a value pair, and the backward derivative receives an inout value pair.
[__NonCopyableType]
struct AdjustedReceiver : IDifferentiable
{
    typealias Differential = float;
    no_diff float scale;

    [Differentiable]
    static float dzero()
    {
        return 0.0;
    }

    [Differentiable]
    static float dadd(float a, float b)
    {
        return a + b;
    }

    [ForwardDerivative(evalForward)]
    [BackwardDerivative(evalBackward)]
    float eval(float x)
    {
        return scale * x;
    }

    static DifferentialPair<float> evalForward(
        DifferentialPair<AdjustedReceiver> receiver,
        DifferentialPair<float> x)
    {
        return diffPair(receiver.p.scale * x.p, receiver.p.scale * x.d);
    }

    static void evalBackward(
        inout DifferentialPair<AdjustedReceiver> receiver,
        inout DifferentialPair<float> x,
        float dOut)
    {}
}

// The declared custom derivatives retain the effective `this` parameter mode of their primal.
// CHECK-LABEL: [nameHint("BorrowReceiver.evalForward")]
// CHECK-NEXT: func {{.*}} : Func(DiffPair(Float,
// CHECK-SAME: BorrowInParam(Attributed(%BorrowReceiver,
// CHECK-SAME: DiffPair(Float,
// CHECK-LABEL: [nameHint("BorrowReceiver.evalBackward")]
// CHECK-NEXT: func {{.*}} : Func(Void,
// CHECK-SAME: BorrowInParam(Attributed(%BorrowReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,
// CHECK-LABEL: [nameHint("RefReceiver.evalForward")]
// CHECK-NEXT: func {{.*}} : Func(DiffPair(Float,
// CHECK-SAME: RefParam(Attributed(%RefReceiver,
// CHECK-SAME: DiffPair(Float,
// CHECK-LABEL: [nameHint("RefReceiver.evalBackward")]
// CHECK-NEXT: func {{.*}} : Func(Void,
// CHECK-SAME: RefParam(Attributed(%RefReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,

// AdjustedReceiver expresses the differentiated receiver as an ordinary explicit parameter.
// Its declared In mode becomes a value DifferentialPair, while inout remains BorrowInOut.
// CHECK-LABEL: [nameHint("AdjustedReceiver.evalForward")]
// CHECK-NEXT: func [[ADJUSTED_FWD:%[A-Za-z0-9_]+]] : Func(DiffPair(Float,
// CHECK-SAME: DiffPair(%AdjustedReceiver,
// CHECK-SAME: DiffPair(Float,
// CHECK-LABEL: [nameHint("AdjustedReceiver.evalBackward")]
// CHECK-NEXT: func {{.*}} : Func(Void,
// CHECK-SAME: BorrowInOutParam(DiffPair(%AdjustedReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,

// The generated AD2 aliases use the established derivative-ABI projection. BorrowIn follows the
// In rules and becomes a value receiver; Ref becomes BorrowInOut.
// CHECK-LABEL: [nameHint("BorrowReceiver.eval.$__syn_fwd_diff")]
// CHECK-NOT: BorrowInParam(
// CHECK: Attributed(%BorrowReceiver,
// CHECK-SAME: DiffPair(Float,
// CHECK-LABEL: [nameHint("BorrowReceiver.eval.bwd_diff")]
// CHECK-NOT: BorrowInParam(
// CHECK: Attributed(%BorrowReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,
// CHECK-LABEL: [nameHint("RefReceiver.eval.$__syn_fwd_diff")]
// CHECK-NOT: RefParam(
// CHECK: BorrowInOutParam(Attributed(%RefReceiver,
// CHECK-SAME: DiffPair(Float,
// CHECK-LABEL: [nameHint("RefReceiver.eval.bwd_diff")]
// CHECK-NOT: RefParam(
// CHECK: BorrowInOutParam(Attributed(%RefReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,

// The explicit non-copyable receiver already has the projected shape, so the forward association
// directly references evalForward and the backward alias preserves BorrowInOut.
// CHECK: witness_table_entry({{.*}},[[ADJUSTED_FWD]])
// CHECK-LABEL: [nameHint("AdjustedReceiver.eval.bwd_diff")]
// CHECK: BorrowInOutParam(DiffPair(%AdjustedReceiver,
// CHECK-SAME: BorrowInOutParam(DiffPair(Float,
