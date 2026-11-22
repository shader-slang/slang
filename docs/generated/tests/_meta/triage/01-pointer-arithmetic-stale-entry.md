# 01 — `pointer-arithmetic-spirv-emission.slang`: a real codegen bug

**Verdict: compiler bug.** Keep the `expected-failures.txt` entry. Filed as
[`ptraccesschain-function-storage-missing-capability`](../findings/ptraccesschain-function-storage-missing-capability.yaml).

> **This file previously reached the opposite conclusion** — "stale entry,
> delete the line" — on the strength of a local pass. That was wrong, and the
> way it was wrong is the useful part; see _How the first pass went wrong_
> below.

## Evidence

Without SPIR-V validation the test passes:

```
$ build/Release/bin/slang-test -test-dir docs/generated/tests \
    docs/generated/tests/conformance/types-pointer/pointer-arithmetic-spirv-emission.slang
100% of tests passed (3/3)
```

With validation enabled — which is what the CI container does — it fails:

```
$ SLANG_RUN_SPIRV_VALIDATION=1 build/Release/bin/slang-test ... same file
error: line 61: Opcode PtrAccessChain requires one of these capabilities:
  Addresses VariablePointersStorageBuffer VariablePointers PhysicalStorageBufferAddresses
66% of tests passed (2/3)
```

## Root cause

`emitGetOffsetPtr` (`slang-emit-spirv.cpp:8753`) calls
`requireVariableBufferCapabilityIfNeeded` and then emits `OpPtrAccessChain`
unconditionally. That helper (`:12038`) picks the capability by address space
and handles only `StorageBuffer` and `GroupShared`. A pointer into a
function-local array is `AddressSpace::Function`, matches neither case, and so
no capability is declared while the opcode is emitted anyway.

The capability requirement belongs to the _opcode_, not to a set of address
spaces, so the guard is attached at the wrong place.

## How the first pass went wrong

Two compounding mistakes, both worth naming:

1. **The entry's own comment was not taken at its word.** It said the test
   "fail[s] in the slang-linux-clang-ci container" and passes locally. A local
   pass was therefore not evidence of anything; it was the documented
   behaviour. The comment even flagged the cause as "still to be determined",
   which is an invitation to investigate, not to delete.

2. **The environment difference was never reproduced.** The distinguishing
   variable is `SLANG_RUN_SPIRV_VALIDATION=1`, documented in `CLAUDE.md`.
   Setting it takes one command and settles the question immediately.

The general rule this suggests: an expected-failure entry that attributes a
failure to a _specific environment_ cannot be retired by a run in a different
environment. Reproduce the stated condition, or leave the entry alone.

## Still worth doing

`verify` reports `expected-fail: N` against a longer list without saying which
listed entries did not fail. A warning for entries that passed would turn this
class of staleness into a signal — but note that it would have flagged this
entry as stale too, since it passes locally. The warning should say "did not
fail here", not "is stale".
