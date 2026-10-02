//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir-after finalizeAutoDiffPass -o - -entry computeMain -stage compute -g2

// Regression test for shader-slang/slang#13238: each autodiff-generated derivative must carry its
// own DebugFunction record, named after the derivative, rather than sharing the original function's
// record. A shared record reports the derivative body under the original's name and, because a
// DebugFunctionDefinition binds one record to one body, lets only one sharing body be described.
// The bug is invisible to spirv-val (NonSemantic debug info is not validated), so an explicit
// FileCheck guard is required. The checks anchor on SPIR-V ids or the IR
// `DebugFunction(...)` record syntax rather than on bare name text, so they cannot self-match any
// embedded-source OpString: the SPIR-V pass binds each name's OpString id, the concrete IR checks
// bind each record id and its decoration, and the generic IR checks match the record line itself.
//
// A concrete function (diffSquare) and a generic one (genPoly) are both differentiated: the generic
// case additionally guards the derivative's debug record being built in the derivative's own scope,
// since copyDebugInfo can run while a derivative is still nested in an IRGeneric (compiling at -g2
// would hit a dominance/scope violation if the record were emitted at module scope instead).
//
// Companion to tests/spirv/debug-function-definition-autodiff-distinct.slang, which covers the
// reverse-mode loop / [ForceInline] scenario; this file covers concrete + generic in both directions.

[Differentiable]
float diffSquare(float x)
{
    return x * x;
}

[Differentiable]
T genPoly<T : IDifferentiable & IFloat>(T x)
{
    return x * x * x;
}

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    float x = 3.0;

    var dpFwd = diffPair(x, 1.0);
    outputBuffer[0] = fwd_diff(diffSquare)(dpFwd).d;
    var dpRev = diffPair(x, 0.0);
    bwd_diff(diffSquare)(dpRev, 1.0);
    outputBuffer[1] = dpRev.d;

    var dpGenFwd = diffPair(x, 1.0);
    outputBuffer[2] = fwd_diff(genPoly<float>)(dpGenFwd).d;
    var dpGenRev = diffPair(x, 0.0);
    bwd_diff(genPoly<float>)(dpGenRev, 1.0);
    outputBuffer[3] = dpGenRev.d;
}

// The derivatives that survive to SPIR-V (forward and backward-propagate) each own a distinct
// DebugFunction record, and that record -- not the original's -- is the one bound to the
// derivative's own body by its DebugFunctionDefinition. Both the concrete and the generic function
// are covered.
//CHECK-DAG: [[FWD_NAME:%[0-9]+]] = OpString "s_fwd_diffSquare"
//CHECK-DAG: [[BWD_NAME:%[0-9]+]] = OpString "s_bwdProp_diffSquare"
//CHECK-DAG: [[FWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[FWD_NAME]]
//CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_NAME]]
//CHECK-DAG: DebugFunctionDefinition [[FWD_REC]] %s_fwd_diffSquare
//CHECK-DAG: DebugFunctionDefinition [[BWD_REC]] %s_bwdProp_diffSquare
//CHECK-DAG: [[GFWD_NAME:%[0-9]+]] = OpString "s_fwd_genPoly"
//CHECK-DAG: [[GBWD_NAME:%[0-9]+]] = OpString "s_bwdProp_genPoly"
//CHECK-DAG: [[GFWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[GFWD_NAME]]
//CHECK-DAG: [[GBWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[GBWD_NAME]]
//CHECK-DAG: DebugFunctionDefinition [[GFWD_REC]] %s_fwd_genPoly
//CHECK-DAG: DebugFunctionDefinition [[GBWD_REC]] %s_bwdProp_genPoly

// The apply and remat derivatives are folded away before SPIR-V, so their per-derivative records
// are checked at the IR level (after autodiff): each is a distinct record named after itself. For
// the concrete function the check also ties the record to its function via the DebugFunction
// decoration; the generic function's apply/remat records still live inside the IRGeneric at this
// point, so those are anchored on the record name alone.
//IR-DAG: [[APPLY_REC:%[0-9]+]]{{.*}}DebugFunction("s_apply_diffSquare"
//IR-DAG: [DebugFunction([[APPLY_REC]])]
//IR-DAG: [[REMAT_REC:%[0-9]+]]{{.*}}DebugFunction("s_remat_diffSquare"
//IR-DAG: [DebugFunction([[REMAT_REC]])]
//IR-DAG: DebugFunction("s_apply_genPoly"
//IR-DAG: DebugFunction("s_remat_genPoly"
