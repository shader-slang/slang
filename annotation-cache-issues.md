# Annotation Cache Issues

## Context

The side-effect optimization changed `doesCalleeHaveSideEffect` to use
`IRBuilder::tryLookupAnnotation` for a small set of auto-diff-related annotation kinds instead of
walking all `IRAnnotation` users of the callee.

That exposed a cache-coherency problem in the module annotation lookup cache.

## Repro

The issue reproduced with:

```powershell
.\build\windows-vs2022-dev\Release\bin\slang-test.exe `
    -bindir .\build\windows-vs2022-dev\Release\bin `
    -api vk `
    -emit-spirv-via-glsl `
    tests\autodiff\reverse-generic-arithmetic-1.slang
```

The expected value for `dpY.d` is `1.5`, but the failing path produced `0.0`.

Generated GLSL for the failing case had `s_bwdProp_simple` transpose the `add` call, but it did
not call the constructor backward-propagate function for `T(y)`. As a result, the derivative from
the temporary `T(y)` was not propagated back into the scalar `y`.

## What Went Wrong

The relevant constructor annotation did exist in the IR:

```text
Annotation(%FooImpl.$init, BackwardDerivativePropagate, ...)
```

However, `tryLookupAnnotation(%FooImpl.$init, BackwardDerivativePropagate)` could return a stale
cached miss.

The failure mode is:

1. A lookup for `(target, kind)` occurs before the annotation is visible on that exact target.
2. The cache records a null result for that key.
3. Later specialization/cloning or operand replacement retargets or creates an `IRAnnotation`.
4. The cache entry is not invalidated.
5. Later side-effect analysis trusts the cached miss and treats the callee as having no associated
   derivative-side-effect callees.
6. DCE/simplification can remove the rematerialization/propagation path needed by reverse AD.

The old use-list traversal did not suffer from this because it always observed the current use
list.

## Workaround Tested

A local workaround was tested that made the annotation cache more conservative:

- invalidate the annotation lookup cache when a raw `IRAnnotation` is emitted;
- invalidate entries when operand 0 of an `IRAnnotation` is changed through `IRUse::set`,
  `IRBuilder::replaceOperand`, or bulk `replaceUsesWith`;
- verify cached positive annotations still match the requested target and kind.

With that workaround, the repro passed and generated GLSL included:

```text
s_bwdProp_FooImpl_$init(out float, FooImpl)
```

inside `s_bwdProp_simple`, restoring `dpY.d == 1.5`.

## Why This Needs A Cleaner Fix

The workaround is broad and touches low-level operand mutation paths. It is probably correct in
spirit, but it is easy to miss another mutation path or to add overhead to very hot IR code.

A better fix should make annotation-cache invalidation explicit and centralized. Possible
directions:

- disallow direct mutation/creation of `IRAnnotation` outside annotation-aware builder helpers;
- add a module-level annotation-cache generation counter and bump it on any annotation mutation;
- store enough validation data in cache entries to cheaply detect stale null and stale positive
  results;
- rebuild annotation lookup state at pass boundaries where annotations are cloned, specialized, or
  retargeted.

Until that is designed, `doesCalleeHaveSideEffect` should keep using the conservative annotation
use-list traversal rather than `tryLookupAnnotation`.
