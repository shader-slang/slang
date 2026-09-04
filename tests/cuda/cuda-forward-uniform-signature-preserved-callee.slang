//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Emit-only, deliberately: this test needs a runtime-selected function value to keep the
// signature-preserving use alive, and NVRTC cannot compile one. `Slang_FuncType` is declared only in
// `slang-cpp-host-prelude.h`, so a `COMPARE_COMPUTE` companion here fails to build on a GPU runner
// rather than testing anything. The sibling `unprocessed-callee` test covers the same decline with a
// `[CudaDeviceExport]` callee, which does run.
//
// The negative path of the address-forward: a callee that keeps its by-value signature must keep
// being handed the aggregate by value, even though the same aggregate is forwarded by address into
// a callee that was rewritten.
//
// `readsumA` is used two ways here - called directly from the entry point with `big`, and taken as
// a function *value* for the runtime-selected argument of `call`. That second use makes
// `hasSignaturePreservingUse(readsumA)` true, so `processFunc` leaves its parameters as by-value
// `Big`. Forwarding `&big` into it anyway would hand a `Big*` to a `Big` parameter, which NVRTC
// rejects; the callee check in `findForwardableParamStorage` is what declines that.
//
// `call` itself has no such use, so its aggregate parameter *is* rewritten to `borrow in` - which
// is what makes this a discriminating test rather than a target-wide no-op: both decisions are
// taken for the same `big`, in the same kernel, and the checks below pin each one, including the
// temp-copy fallback that carries `big` into `call`.

struct Big
{
    float v[4];
}

[noinline]
float readsumA(Big b, uint i)
{
    return b.v[i];
}

[noinline]
float readsumB(Big b, uint i)
{
    return b.v[i] * 2.0f;
}

// Selecting the callee at runtime keeps both function values alive as values: specialization cannot
// fold either one away, so `readsumA`'s signature-preserving use survives to
// `transformParamsToConstRef`.
func call(f: functype(Big, uint) -> float, b: Big, i: uint) -> float
{
    return f(b, i);
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(
    uint tid: SV_DispatchThreadID,
    uniform Big big,
    uniform uint sel,
    uniform RWStructuredBuffer<float> output)
{
    // `sel` makes the callee a runtime choice, which is what stops specialization from folding the
    // function value away before the pass runs.
    output[tid] = readsumA(big, tid) + call(sel != 0u ? readsumA : readsumB, big, tid);
}

// `readsumA` keeps a by-value `Big_0` parameter, because it is also used as a function value.
// CHECK: {{.*}}float readsumA_0(Big_0 [[BV:[A-Za-z0-9_]+]], uint
// CHECK-NOT: readsumA_0(Big_0 *

// `call` has no signature-preserving use, so its aggregate parameter is rewritten to a pointer.
// This is the contrast that makes the by-value check above meaningful.
// CHECK: {{.*}}float call_0({{.*}}Big_0 * {{[A-Za-z0-9_]+}}, uint

// CHECK: __global__ void computeMain(Big_0 [[BIG:[A-Za-z0-9_]+]], uint
// The direct call must pass the aggregate by value - `&[[BIG]]` here would be the miscompile:
// a `Big_0 *` argument reaching `readsumA_0`'s by-value `Big_0` parameter.
// CHECK: readsumA_0([[BIG]],
// Because the forward declined, `big` reaches the rewritten `call_0` slot through the ordinary
// temp-copy fallback (`updateCallSites`'s `else` branch) - a per-thread spill and `&temp`, not
// `&big` directly. Pinning this keeps a regression that broke the fallback from passing on the
// strength of the by-value check alone.
// CHECK: Big_0 [[TMP:[A-Za-z0-9_]+]] = [[BIG]];
// CHECK: call_0({{.*}}&[[TMP]],
