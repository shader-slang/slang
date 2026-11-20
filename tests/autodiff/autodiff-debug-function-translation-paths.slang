// Regression test for shader-slang/slang#14129: every autodiff translation op that synthesizes
// a function must give that function debug info from its source function. Five paths did not:
// the legacy [BackwardDerivative] adapters (s_apply_/s_remat_/s_bwdProp_ from
// maybeTranslateLegacyToNewBackwardDerivative), the trivial forward derivative of a
// [TreatAsDifferentiable] callee (maybeTranslateTrivialForwardDerivative), the bwd_diff(f)
// wrapper built from a backward derivative (maybeTranslateLegacyBackwardDerivative), the
// identity remat of a custom __apply conformance (maybeTranslateIdentityRemat), and the DiffPair
// add/zero witnesses (buildDifferentiablePairWitness; named only — no source function exists to
// copy a source location from). The first four now build a distinct DebugFunction record named
// after the function itself; their SPIR-V functions get a DebugFunctionDefinition and a
// DebugScope, so a debugger can attribute their code. The bug is invisible to spirv-val
// (NonSemantic debug info is not validated), so an explicit FileCheck guard is required. The
// checks anchor on SPIR-V ids or the IR DebugFunction(...) record syntax rather than on bare
// name text, so they cannot self-match the embedded-source OpString: the SPIR-V pass binds each
// name's OpString id, and the concrete IR checks bind each record id and its decoration.
//
// mySqr has a user-written backward derivative, so `bwd_diff(f)` (f calls mySqr) exercises the
// legacy paths: the apply/remat/propagate adapters for mySqr, and the bwd_diff(f) wrapper itself.
// bumpCounter is [TreatAsDifferentiable], so `fwd_diff(useCounter)` synthesizes a trivial forward
// derivative for it. applyFunc has a custom __apply func_extension: its conformance synthesizes
// an identity remat for its context type, and its bwd_diff wrapper adapts the synthesized
// extension function, which carries no name hint — the wrapper is then named after the context
// type its remat passes through (s_bwd_SquareContext), keeping its record distinguishable.
//
// The DiffPair add/zero witnesses are named for mid-pass IR readability, but they are inlined
// within the translation pass and appear at neither checkpoint (probed with a nested
// DifferentialPair<DifferentialPair<float>> differential forcing the pair's own conformance);
// they are the one name-only path and carry no DebugFunction record by design.
//
// The last directive compiles without -g: no source function carries debug info, so no
// synthesized function gets a record — pinning the helper's documented no-op.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -g3 -O0 -emit-spirv-directly -entry computeMain -stage compute -experimental-feature

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir-after finalizeAutoDiffPass -o - -entry computeMain -stage compute -g3 -experimental-feature

//TEST:SIMPLE(filecheck=NOG):-target spirv-asm -dump-ir-after finalizeAutoDiffPass -o - -entry computeMain -stage compute -experimental-feature
// NOG-NOT: DebugFunction(

RWStructuredBuffer<float> outputBuffer;

void mySqr_bwd(inout DifferentialPair<float> dpx, float dOut)
{
    dpx = diffPair(dpx.p, 2.0 * dpx.p * dOut);
}

[BackwardDerivative(mySqr_bwd)]
float mySqr(float x)
{
    return x * x;
}

[Differentiable]
float f(float x)
{
    return mySqr(x) + x;
}

[TreatAsDifferentiable]
void bumpCounter(no_diff inout int counter, float x)
{
    counter += 1;
}

[Differentiable]
float useCounter(float x)
{
    int n = 2;
    bumpCounter(n, x);
    return float(n) * x;
}

struct SquareContext
{
    float x;
    void operator()(out float dx, float dOut)
    {
        dx = dOut * 2.0 * x;
    }
};

float applyFunc(float x)
{
    return x * x;
}

__func_extension __apply(applyFunc)(float x) -> Tuple<float, SquareContext>
{
    return makeTuple(applyFunc(x), SquareContext(x));
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    var dpx = diffPair(3.0, 0.0);
    bwd_diff(f)(dpx, 1.0);
    outputBuffer[0] = dpx.d;
    outputBuffer[1] = fwd_diff(useCounter)(diffPair(2.0, 1.0)).d;
    var dpa = diffPair(3.0, 0.0);
    bwd_diff(applyFunc)(dpa, 1.0);
    outputBuffer[2] = dpa.d;
}

// The functions that survive to SPIR-V each own a distinct DebugFunction record, named after
// themselves, and that record is bound to their own body by a DebugFunctionDefinition: the
// legacy backward-propagate adapter for mySqr, the bwd_diff(f) wrapper, and bumpCounter's trivial
// forward derivative.
//CHECK-DAG: [[BWDPROP_STR:%[0-9]+]] = OpString "s_bwdProp_mySqr"
//CHECK-DAG: [[BWDPROP_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWDPROP_STR]]
//CHECK-DAG: DebugFunctionDefinition [[BWDPROP_REC]] %s_bwdProp_mySqr
//CHECK-DAG: [[BWD_STR:%[0-9]+]] = OpString "s_bwd_f"
//CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_STR]]
//CHECK-DAG: DebugFunctionDefinition [[BWD_REC]] %s_bwd_f
//CHECK-DAG: [[FWD_STR:%[0-9]+]] = OpString "s_fwd_bumpCounter"
//CHECK-DAG: [[FWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[FWD_STR]]
//CHECK-DAG: DebugFunctionDefinition [[FWD_REC]] %s_fwd_bumpCounter

// The apply and remat adapters, the identity remat, and the __apply wrapper are folded away
// before SPIR-V, so their debug records are checked at the IR level (after autodiff): each is a
// distinct record named after the function it describes, tied to it by the DebugFunction
// decoration. The __apply wrapper covers the adapter shape with no name hint (the synthesized
// extension function), where the name comes from the remat's context type.
//IR-DAG: [[APPLY_REC:%[0-9]+]]{{.*}}DebugFunction("s_apply_mySqr"
//IR-DAG: [DebugFunction([[APPLY_REC]])]
//IR-DAG: [[REMAT_REC:%[0-9]+]]{{.*}}DebugFunction("s_remat_mySqr"
//IR-DAG: [DebugFunction([[REMAT_REC]])]
//IR-DAG: [[CTXREMAT_REC:%[0-9]+]]{{.*}}DebugFunction("s_rematCtx_SquareContext"
//IR-DAG: [DebugFunction([[CTXREMAT_REC]])]
//IR-DAG: [[ABWD_REC:%[0-9]+]]{{.*}}DebugFunction("s_bwd_SquareContext"
//IR-DAG: [DebugFunction([[ABWD_REC]])]
