# Compiler Diagnostic: cleanUpVoidType strips intrinsic arguments from zero-sized groupshared arrays

**Status**: Needs better diagnostic  
**Severity**: Crash (internal assertion) with unhelpful error message  
**Discovered**: 2026-04-01 while debugging `sumReduceRows` benchmark  

## Problem

When a `groupshared` array has zero size (e.g. `groupshared uint4 data[0]`), the compiler
lowers its type to `Void`. The `cleanUpVoidType` IR pass then:

1. Replaces uses of `load(%data)` (type `Void`) with `voidValue`
2. Strips `Void`-typed arguments from `IRCall` instructions

This silently breaks calls to functions with `__intrinsic_asm` that reference arguments
by positional index (`$0`, `$1`, ...). The argument is removed but the asm string still
references `$0`, causing an assertion failure:

```
assert failure: (0 <= argIndex) && (argIndex < m_argCount)
  at slang-intrinsic-expand.cpp:290
```

## Root Cause Chain

```
SharedMemorySize0.Bytes = 0       (all terms are 0 for CUDA + Inference mode)
  → data[Bytes / sizeof(uint4)]  = data[0]   (zero-sized groupshared array)
  → load(%data) : Void           (zero-sized array type lowered to Void)
  → cleanUpVoidType strips arg   (Void-typed values removed from calls)
  → getSharedBaseAddr()           (was getSharedBaseAddr(%data), now 0 args)
  → intrinsic asm "$0" fails     (no argument at index 0)
```

## How It Was Triggered

The `sumReduceRows` benchmark used `ExecutionMode.Inference` which gives `Bytes = 0`.
Changing to `ExecutionMode.Training` allocates proper shared memory and avoids the issue.

## Diagnostic Improvement Needed

The crash message (`assert failure: (0 <= argIndex) && (argIndex < m_argCount)`) gives
no indication that the root cause is a zero-sized `groupshared` array. Better options:

### Option A: Diagnose in `cleanUpVoidType`

When stripping a `Void` argument from a call whose callee has `IRTargetIntrinsicDecoration`,
emit a diagnostic warning or error:

```cpp
// In slang-ir-cleanup-void.cpp, kIROp_Call case:
if (inst->getOp() == kIROp_Call)
{
    auto callee = inst->getOperand(0);
    if (auto specialize = as<IRSpecialize>(callee))
        callee = specialize->getBase();
    if (callee->findDecoration<IRTargetIntrinsicDecoration>())
    {
        // Emit diagnostic: "Void-typed argument passed to intrinsic function;
        // this usually means a zero-sized groupshared array is being used.
        // Check that SharedMemorySize.Bytes > 0 for this configuration."
    }
}
```

### Option B: Diagnose zero-sized groupshared arrays earlier

Add a validation pass that flags `groupshared T[0]` declarations with a clear error
message before they reach type lowering.

### Option C: Preserve Void args for intrinsic calls

Instead of stripping the argument, keep it so the intrinsic asm can still reference it.
This would prevent the crash but the generated code would still be incorrect (accessing
a zero-sized array). A diagnostic is better.

## Files Involved

| File | Role |
|------|------|
| `source/slang/slang-ir-cleanup-void.cpp` | The pass that strips Void arguments |
| `source/slang/slang-intrinsic-expand.cpp:290` | The assertion that crashes |
| `source/standard-modules/neural/shared-memory-pool.slang` | `getSharedBaseAddr` intrinsic and `SharedMemorySize0.Bytes` |
