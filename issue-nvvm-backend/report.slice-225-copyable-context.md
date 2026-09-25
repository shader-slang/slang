# Admit copyable values in generated thread-local contexts

## Motivation

Consider per-invocation state used by a helper:

```slang
static bool flag = false;
[noinline] bool flip()
{
    flag = !flag;
    return flag;
}
```

The established CUDA context producer puts flag in a local KernelContext and passes its address to
flip. Before this change, NVVM rejects that canonical pointer because the context's Bool field is
outside an older integer/Float32-only restriction. Research224 shows the same Boolean struct passed
as an ordinary local inout parameter executes correctly. The two frozen masked-prefix min/max tests
reach this same context boundary.

## Proposed solution

Use the existing recursive copyable-struct classifier in the explicit ThreadLocal pointer branch.
The context now admits value fields already supported in ordinary locals: Bool, numeric leaves,
vectors, fixed arrays and nested structs. Keep exact read-write, ThreadLocal, four-operand and default-
layout checks. Resource-bearing explicit contexts remain outside this copyable-only scope.

## Change summary

- `slang-emit-nvvm-type-lowering.cpp` reuses `asNVVMSupportedCopyableStructType` instead of the older
  scalar-only classifier and explains the producer's Boolean/nested-value example.
- A new structural unit checks a Boolean context is entry-local storage passed to a helper, with
  zero provider global declarations. The fake builder recognizes its existing Boolean leaf, typed
  Boolean loads and scalar Boolean unary results.
- `nvvm-copyable-kernel-context.slang` tests initialization and dynamic mutation of Boolean, nested,
  double, float2 and fixed-array state across helpers; one discovery identity is added.
- Plan/report, full acceptance evidence, design and STATUS retain the producer/consumer audit and
  exact preservation obligations.

## Concepts and vocabulary

A _copyable-value struct_ belongs to the existing finite recursive scalar/vector/array/struct value
algebra. _ThreadLocal_ is Slang's source address-space classification for per-invocation context;
its enum value1 is not LLVM global address space1. The _compact local pointer_ and explicit helper
pointer intentionally have different source spellings but the same exact pointee/provider address.

## Process report

No new production helper, fallback, alternate representation or name-based KernelContext rule is
introduced. `introduceExplicitGlobalContext` and `findOrCreateContextPtrForFunc` already create the
correct entry-local initialized storage and explicit helper parameters. The compact call argument
is intentionally compatible with the ThreadLocal parameter under `_isSupportedNVVMHelperArgument`.
The producer shape is valid; the support classifier was retaining an earlier capability restriction.

`asNVVMSupportedLocalResourceStructPointerType` already recognizes the finite struct pointee and the
exact pointer spelling. Only its context-specific scalar-struct test changes to the existing
copyable-struct test. No pointer qualifiers are erased. Type lowering still calls `_lowerPointerType`
with the same selected value representation and `SLANG_NVVM_ADDRESS_SPACE_GENERIC`; local allocation,
field addressing, loads/stores and helper argument matching are unchanged. Resource-bearing context
pointees remain rejected by the copyable-value classifier, even though ordinary local resource
structs have their own established support.

The new fixture checks zero initialization before writing lane-specific runtime input into state.
Helpers then read that state, mutate it, and validate the new state. Boolean flags change twice,
nested double values preserve low-word precision and flip sign, float2 components swap, fixed uint
array elements swap, and a visit counter changes independently per invocation. Expected values are
computed directly from the original input and known transitions, without another context or wave
operation serving as an oracle. NVRTC passes before the change; both direct modes reject exactly
at the context pointer boundary. The finalized source/inputs remain unchanged for after validation.

The structural test exposed three gaps in the fake provider's existing Boolean bookkeeping. Its
struct builder omitted the Boolean leaf, its value checker omitted recorded Boolean loads, and
that checker omitted typed Boolean unary results such as NOT. Each correction consumes existing
type or operation records; no new semantic representation is introduced. The load check retains
bounds checking, and unary recognition requires a scalar Boolean result descriptor. The real
provider and all three runtime modes already passed while these apparatus issues were isolated.
The three failed focused runs remain under `initial-fake-{struct,load,store}-check`, with their tested
provenance. Final focused and full unit gates run after the last correction and rebuild.

The full checkpoint preserves all 1,611 previous correct cells and adds three correct context cells:
1,665 fresh cells total, 1,614 correct and 51 known failures. The 306 previous discovery cells are
exact. Four frozen prefix cells retain their failed classification, return code and execution counts
while diagnostic/canonical shape advances from the context parameter to GenericAsm
`_wavePrefixExclusiveMin/Max(($1).x, $0)`, signature `double(double, vector<uint,4>)`. All other old
cells preserve all five fields exactly. First-known records, prior diagnostic observations and six
resolved histories remain intact. Diagnostic advancement is not counted as working prefix support.
All 550 previous runtime inputs and 102 old discovery rows are unchanged; the new fixture is the
551st runtime input. No cell is missing, duplicated or inherited.

All six material cells compile and assemble. That is support evidence only; material execution
still requires its application bindings, texture/LUT/input contract and independent output oracle.

Fresh-context delegation remains unavailable at the app's agent-thread limit. Parent execution and
review follow WORKFLOW's fallback and do not imply an independent worker review.

Final focused coverage passes 6/6, units 479/479 with one existing Windows-only skip, runtime smoke 4/4,
toolkit 18/18 and discovery contracts 6/6. Exact research224 replay passes 9/9 executions and 864 words,
including the two former Boolean-global preflight failures. Compiler hash is
`14e80c03ff1571a2248f935cc795a9465ec963f9d3ac5cf4a7f80ba076a48ae1`; provider/ABI 36 are unchanged.
Parent accepted after reviewing the complete diff and auditing 203 evidence references, 26 tested
sources, 12 artifacts and 551 runtime input hashes. See `runtime-validation.slice-225.json` and both
slice225 census files. Latest full checkpoint is 225; implementation cadence resets to zero.
