//TEST:SIMPLE: -no-codegen -Wno-38109
//TEST:SIMPLE(filecheck=CHECK): -no-codegen -dump-ir -Wno-38109

// Resolve both derivative function types for methods whose effective `this` parameter uses
// BorrowIn or Ref mode. The receiver is not differentiable, so NoDiffThis keeps its value type
// unchanged while the ordinary parameter is transformed. The established derivative ABI maps a
// BorrowIn receiver to In and a Ref receiver to BorrowInOut. The primal method itself retains Ref.

struct Receiver
{
    float scale;

    [constref]
    [NoDiffThis]
    [Differentiable]
    float borrowEval(float x)
    {
        return scale * x * x;
    }

    [__ref]
    [NoDiffThis]
    [Differentiable]
    float refEval(float x)
    {
        return scale * x * x;
    }
}

void exerciseDerivativeTypes()
{
    Receiver receiver = {3.0};

    let borrowForward = fwd_diff(Receiver::borrowEval)(receiver, diffPair(2.0, 1.0));
    var borrowArgument = diffPair(2.0, 0.0);
    bwd_diff(Receiver::borrowEval)(receiver, borrowArgument, 1.0);

    let refForward = fwd_diff(Receiver::refEval)(receiver, diffPair(2.0, 1.0));
    var refArgument = diffPair(2.0, 0.0);
    bwd_diff(Receiver::refEval)(receiver, refArgument, 1.0);
}

// A non-copyable differentiable receiver adjusts the ordinary by-value default to BorrowIn. The
// receiver is still a semantic input, so its differential is an output of the backward callable.
[__NonCopyableType]
struct NonCopyableReceiver : IDifferentiable
{
    typealias Differential = float;
    no_diff float value;

    // Spell these requirements out so this test does not depend on automatic synthesis for a
    // non-copyable self-differential type, which is a separately unsupported conformance shape.
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

    [ForwardDifferentiable]
    [BackwardDifferentiable]
    float evaluate(float scale)
    {
        return value * scale;
    }
}

void exerciseAdjustedBorrowIn()
{
    DifferentialPair<NonCopyableReceiver> receiver;
    var argument = diffPair(2.0, 0.0);
    bwd_diff(NonCopyableReceiver::evaluate)(receiver, argument, 1.0);
}

// CHECK-LABEL: [nameHint("Receiver.borrowEval")]
// CHECK: func {{.*}} : Func(Float, BorrowInParam(
// CHECK-LABEL: [nameHint("Receiver.refEval")]
// CHECK: func {{.*}} : Func(Float, RefParam(

// CHECK-LABEL: [nameHint("Receiver.borrowEval.fwd_diff")]
// CHECK-NOT: BorrowInParam(
// CHECK: Attributed(%Receiver
// CHECK-LABEL: [nameHint("Receiver.borrowEval.apply_bwd")]
// CHECK: BorrowInParam(
// CHECK-LABEL: [nameHint("Receiver.borrowEval.remat")]
// CHECK-NOT: BorrowInParam(
// CHECK: Attributed(%Receiver
// CHECK-LABEL: [nameHint("Receiver.borrowEval.bwd_diff")]
// CHECK-NOT: BorrowInParam(
// CHECK: Attributed(%Receiver

// The generated derivative entry points and rematerialization use the established derivative ABI.
// CHECK-LABEL: [nameHint("Receiver.refEval.fwd_diff")]
// CHECK-NOT: RefParam(
// CHECK: BorrowInOutParam(

// apply_bwd replays the primal ABI because it invokes the primal method.
// CHECK-LABEL: [nameHint("Receiver.refEval.apply_bwd")]
// CHECK: let {{.*}} : Func(
// CHECK: RefParam(

// CHECK-LABEL: [nameHint("Receiver.refEval.remat")]
// CHECK-NOT: RefParam(
// CHECK: BorrowInOutParam(
// CHECK-LABEL: [nameHint("Receiver.refEval.bwd_diff")]
// CHECK-NOT: RefParam(
// CHECK: BorrowInOutParam(

// CHECK-LABEL: [nameHint("NonCopyableReceiver.evaluate")]
// CHECK: func {{.*}} : Func(Float, BorrowInParam(%NonCopyableReceiver
// CHECK-LABEL: [nameHint("NonCopyableReceiver.evaluate.apply_bwd")]
// CHECK: BorrowInParam(%NonCopyableReceiver
// CHECK-LABEL: [nameHint("NonCopyableReceiver.evaluate.remat")]
// CHECK: BorrowInParam(%NonCopyableReceiver
// CHECK-LABEL: [nameHint("NonCopyableReceiver.evaluate.bwd_diff")]
// CHECK: BorrowInOutParam(DiffPair(%NonCopyableReceiver
// CHECK-LABEL: [nameHint("NonCopyableReceiver.evaluate.fwd_diff")]
// CHECK-NOT: BorrowInParam(
// CHECK: DiffPair(%NonCopyableReceiver
