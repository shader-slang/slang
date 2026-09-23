//TEST:SIMPLE(filecheck=CHECK):-target spirv -g2 -O0 -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir-after finalizeAutoDiffPass -o - -entry computeMain -stage compute -g2

// Regression test for shader-slang/slang#13238: each autodiff-generated derivative must carry its
// own DebugFunction record, named after the derivative, rather than sharing the original function's
// record. A shared record reports the derivative body under the original's name and, because a
// DebugFunctionDefinition binds one record to one body, lets only one sharing body be described.
// The bug is invisible to spirv-val (NonSemantic debug info is not validated), so this FileCheck
// guard is the only regression coverage. Each check is ID-anchored -- it binds an id (a name's
// OpString in the SPIR-V pass, a record in the IR pass) and then requires the record / definition /
// decoration that references it -- so it cannot self-match any embedded-source OpString.

[Differentiable]
float diffSquare(float x)
{
    return x * x;
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
}

// The derivatives that survive to SPIR-V (forward and backward-propagate) each own a distinct
// DebugFunction record, and that record -- not the original's -- is the one bound to the
// derivative's own body by its DebugFunctionDefinition.
//CHECK-DAG: [[FWD_NAME:%[0-9]+]] = OpString "s_fwd_diffSquare"
//CHECK-DAG: [[BWD_NAME:%[0-9]+]] = OpString "s_bwdProp_diffSquare"
//CHECK-DAG: [[FWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[FWD_NAME]]
//CHECK-DAG: [[BWD_REC:%[0-9]+]] = OpExtInst %void %{{[0-9]+}} DebugFunction [[BWD_NAME]]
//CHECK-DAG: DebugFunctionDefinition [[FWD_REC]] %s_fwd_diffSquare
//CHECK-DAG: DebugFunctionDefinition [[BWD_REC]] %s_bwdProp_diffSquare

// The apply and remat derivatives are folded away before SPIR-V, so their per-derivative records
// are checked at the IR level (after autodiff): each is a distinct record named after itself, and
// the derivative function carries it via a DebugFunction decoration that references that record.
//IR-DAG: [[APPLY_REC:%[0-9]+]]{{.*}}DebugFunction("s_apply_diffSquare"
//IR-DAG: [DebugFunction([[APPLY_REC]])]
//IR-DAG: [[REMAT_REC:%[0-9]+]]{{.*}}DebugFunction("s_remat_diffSquare"
//IR-DAG: [DebugFunction([[REMAT_REC]])]
