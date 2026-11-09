//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly -O0

// Regression guard for shader-slang/slang#12581. A pointer to a mutable structured-buffer element,
// passed through a function parameter and merged inside a [noinline] helper with another element
// pointer of the same buffer, must compile. Both values are logical StorageBuffer pointers, so the
// merge into `p` is consistent -- there is no conflict.
//
// The address-space reconcile pre-pass runs before call-site specialization rewrites `ext`'s
// address space from its argument, so at pre-pass time `ext` still carries the provisional surface
// `int*` (physical Device) type. The pass must treat that unresolved parameter as unknown (like a
// phi) and let the slot's concrete write (&buf[0], StorageBuffer) drive reconciliation, rather than
// reading the provisional physical type and falsely reporting an inconsistent-address-space
// conflict on this valid code. `[noinline]` keeps `ext` a real parameter through the pass.
//
// Non-debug runs only: under `-g`, a pointer parameter specialized from Device to StorageBuffer
// leaves its debug backing variable with the old pointee -- a pre-existing cross-function debug-var
// gap (reachable via `&buf[i]` too), out of scope here and deferred to #13039.

// A front-end/legalization rejection would abort before emission, so forbid a diagnostic first,
// then confirm emission reached the entry point and no physical pointer was produced (both merged
// values are logical StorageBuffer). (`error:` -- not bare `error` -- avoids the harness's
// `standard error = {` wrapper line.)
//CHECK-NOT: error:
//CHECK-NOT: PhysicalStorageBuffer
//CHECK: OpEntryPoint
//CHECK-NOT: PhysicalStorageBuffer

RWStructuredBuffer<int> buf;
RWStructuredBuffer<int> outp;

[noinline]
int pick(int* ext, bool c)
{
    int* p = &buf[0];
    if (c)
        p = ext;
    return *p;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outp[0] = pick(&buf[1], tid.x == 0);
}
