# 11 — `_findOrEmitHoistableInst` does N+1 hash-map probes and an arena allocation per instruction

**Status:** not started
**Estimated size:** L
**Impact:** every type, witness table and other hoistable instruction created
anywhere in the compiler goes through this function

---

## Summary

`IRBuilder::_findOrEmitHoistableInst` is the hash-consing entry point for every
hoistable IR instruction. Per call, _before_ the deduplication probe even
happens, it:

1. Copies the fixed operands into a `ShortList`, then runs
   `canonicalizeInstOperands`, which for array types calls back into
   `IRBuilder::getIntValue` — **another hash-map lookup, nested inside this one**.
2. Allocates a full dummy `IRInst` plus all its `IRUse` slots on the module's
   memory arena.
3. Performs **one `m_instReplacementMap` probe per operand** — on a map that is
   empty in the overwhelming majority of compilations.
4. Constructs an `IRInstKey`, which folds all the operands into a hash
   (O(operand count)).

Then it probes the numbering map. On a hit — the common case, which is the whole
point of hash-consing — every one of the above is discarded, and additionally a
**linear scan of the sibling instruction list** may run.

So deduplicating an N-operand instruction costs N+1 hash-map probes and 2 hash
computations, where 1 probe and 1 hash would do.

---

## The code

```cpp
// source/slang/slang-ir.cpp:2798-2900 (abridged, comments preserved)
IRInst* IRBuilder::_findOrEmitHoistableInst(
    IRType* type, IROp op,
    Int fixedArgCount, IRInst* const* fixedArgs,
    Int varArgListCount, Int const* listArgCounts, IRInst* const* const* listArgs)
{
    UInt operandCount = fixedArgCount;
    for (Int ii = 0; ii < varArgListCount; ++ii)
        operandCount += listArgCounts[ii];

    ShortList<IRInst*, 8> canonicalizedOperands;
    canonicalizedOperands.setCount(fixedArgCount);
    for (Index i = 0; i < fixedArgCount; i++)
        canonicalizedOperands[i] = fixedArgs[i];

    canonicalizeInstOperands(*this, op, canonicalizedOperands.getArrayView().arrayView);

    auto& memoryArena = getModule()->getMemoryArena();
    void* cursor = memoryArena.getCursor();

    // We are going to create a 'dummy' instruction on the memoryArena
    // which can be used as a key for lookup, so see if we
    // already have an equivalent instruction available to use.
    size_t keySize = sizeof(IRInst) + operandCount * sizeof(IRUse);
    IRInst* inst = (IRInst*)memoryArena.allocateAndZero(keySize);
    ...
    // Don't link up as we may free (if we already have this key)
    {
        IRUse* operand = inst->getOperands();
        for (Int ii = 0; ii < fixedArgCount; ++ii)
        {
            auto arg = canonicalizedOperands[ii];
            m_dedupContext->getInstReplacementMap().tryGetValue(arg, arg);   // <-- probe per operand
            operand->usedValue = arg;
            operand++;
        }
        for (Int ii = 0; ii < varArgListCount; ++ii)
        {
            UInt listOperandCount = listArgCounts[ii];
            for (UInt jj = 0; jj < listOperandCount; ++jj)
            {
                auto arg = listArgs[ii][jj];
                m_dedupContext->getInstReplacementMap().tryGetValue(arg, arg); // <-- and here
                operand->usedValue = arg;
                operand++;
            }
        }
    }

    // Find or add the key/inst
    {
        IRInstKey key = {inst};    // <-- O(operandCount) hash

        IRInst** found = m_dedupContext->getGlobalValueNumberingMap().tryGetValueOrAdd(key, inst);
        SLANG_ASSERT(endCursor == memoryArena.getCursor());
        // If it's found, just return, and throw away the instruction
        if (found)
        {
            memoryArena.rewindToCursor(cursor);

            // If the found inst is defined in the same parent as current insert location but
            // is located after the insert location, we need to move it to the insert location,
            // except for insts at the module level, where order does not matter.
            //
            // This last condition helps to accelerate the common case of emitting global hoistable
            // insts (types, sets, etc.)
            //
            auto foundInst = *found;
            if (foundInst->getParent() && foundInst->getParent() == getInsertLoc().getParent() &&
                getInsertLoc().getMode() == IRInsertLoc::Mode::Before &&
                foundInst->getParent() != getModule()->getModuleInst())
            {
                auto insertLoc = getInsertLoc().getInst();
                bool isAfter = false;
                for (auto cur = insertLoc->next; cur; cur = cur->next)   // <-- O(siblings)
                {
                    if (cur == foundInst)
                    {
                        isAfter = true;
                        break;
                    }
                }
                if (isAfter)
                    foundInst->insertBefore(insertLoc);
            }
            return *found;
        }
    }
    ...
}
```

And the nested builder call:

```cpp
// source/slang/slang-ir.cpp:2711-2731
static void canonicalizeInstOperands(IRBuilder& builder, IROp op, ArrayView<IRInst*> operands)
{
    if (op == kIROp_ArrayType)
    {
        if (operands.getCount() < 2)
            return;
        IRInst* elementCount = operands[1];
        if (auto intLit = as<IRIntLit>(elementCount))
        {
            if (intLit->getDataType()->getOp() != kIROp_IntType)
            {
                IRInst* newElementCount =
                    builder.getIntValue(builder.getIntType(), intLit->getValue());
                operands[1] = newElementCount;
            }
        }
    }
}
```

`builder.getIntValue` goes to `_findOrEmitConstant`, which probes the constant
map — see issue [07](07-irconstantkey-hash-not-cached.md) for what that costs.
`builder.getIntType()` is itself a hoistable-inst lookup, i.e. a recursive call
into this same function.

---

## Analysis of each cost

### The `m_instReplacementMap` probes (the clearest win)

```cpp
// source/slang/slang-ir.h:2065-2068
// Duplicate insts that are still alive and needs to be replaced in m_globalValueNumberMap
// when used as an operand to create another inst.
Dictionary<IRInst*, IRInst*> m_instReplacementMap;
```

This map only has entries while a deduplication-invalidating edit is in flight.
For the overwhelming majority of instruction creations it is empty, and the
lookup is pure overhead — one hash of a pointer plus one probe, per operand.

`ankerl::unordered_dense` does early-out on an empty map:

```cpp
// external/unordered_dense/include/ankerl/unordered_dense.h:1157-1161
template <typename K>
auto do_find(K const& key) -> iterator {
    if (ANKERL_UNORDERED_DENSE_UNLIKELY(empty())) {
        return end();
    }
    ...
```

but `absl`, `boost` and `std` do not all do so, and even ankerl's early-out
requires the call, the `empty()` load, and the branch. Since `Dictionary` is
inline, a caller-side `if (!map.empty())` hoisted _out of the operand loop_ is
strictly better than an early-out _inside_ it.

The same pattern appears on the non-hoistable creation path:

```cpp
// source/slang/slang-ir.cpp:2065 (inside createInstImpl-style code)
auto arg = fixedArgs[aa];
m_dedupContext->getInstReplacementMap().tryGetValue(arg, arg);
operand->init(inst, arg);
```

so a fix should cover both.

### The arena dummy instruction

Allocating and then rewinding is already cheap (`MemoryArena` bump-allocates and
`rewindToCursor` just resets the pointer), and the `SLANG_ASSERT(endCursor ==
memoryArena.getCursor())` guards against anything allocating in between. The
real cost is not the allocation but the _zeroing_ (`allocateAndZero`) and the
operand writes, which touch `sizeof(IRInst) + N * sizeof(IRUse)` bytes of fresh
memory for a lookup that usually hits.

Avoiding it entirely would mean a key type that can hash and compare against
`(op, type, operands[])` without materialising an `IRInst`. That means making
`IRInstKey` (or a transparent probe type) able to represent an
"instruction description" as well as an instruction — the same shape
`ValNodeDesc` vs `ValKey` already has on the AST side
(`source/slang/slang-ast-base.h:241-274` and `source/slang/slang-ast-builder.h:139-210`).
That is the principled fix and it is a genuine design change.

### The sibling linear scan

The scan only runs when the found instruction shares a parent with the insert
location, the insert mode is `Before`, and the parent is not the module inst.
The comment says the module-level exclusion "helps to accelerate the common
case", which suggests this was already identified as a hot spot once. There is a
related comment at `slang-ir.cpp:1545-1557` acknowledging an O(n)-in-blocks cost
elsewhere.

For a large basic block this is O(block size) per dedup hit. Fixing it properly
needs an ordering index on instructions within a parent — which the IR does not
currently maintain. Note it; it is probably the lowest-value item here unless
profiling says otherwise.

---

## Why this is independent of the selected hash function and map

Every cost above is either outside the map (arena, sibling scan,
canonicalisation) or is a _count of probes_ rather than a property of one probe.
All 32 matrix configurations pay them.

---

## Proposed change

Land these separately, easiest first.

### Part A — skip the replacement map when it is empty

```cpp
auto& replacementMap = m_dedupContext->getInstReplacementMap();
const bool hasReplacements = replacementMap.getCount() != 0;
...
auto arg = canonicalizedOperands[ii];
if (hasReplacements)
    replacementMap.tryGetValue(arg, arg);
```

One `getCount()` load hoisted out of the loop, replacing N probes with N
predictable branches in the common case. Apply to both loops here and to the
non-hoistable path at `slang-ir.cpp:2065`.

Behaviour-preserving and trivially reviewable.

### Part B — avoid materialising the dummy instruction on the probe

**Measured, and deferred.** `sizeof(IRInst)` is 112 and `sizeof(IRUse)` is 32,
so the dummy costs `112 + 32N` bytes; at the expected median of one operand that
is 144 bytes zeroed and partly written per probe, against 8 bytes for a borrowed
pointer array. That ratio looks compelling until you notice that the arena is
rewound to the same cursor on every deduplication hit, so the dummy lands at the
_same address_ every time and its three cache lines stay resident in L1. The
memset is then single-digit cycles, against a probe already dominated by the
hash map's own cache misses.

So the payoff is small, while the risk is not: this is the hash-consing key, and
a subtle disagreement between the transparent comparator and
`IRInstKey::operator==` produces a missed or incorrect deduplication rather than
a slowdown. Do this only if a profile shows the probe path hot for a reason
other than the map itself. The design below is kept for whoever revisits it.

Introduce a description type and a transparent hash/equality pair, exactly as
the AST side already does:

```cpp
struct IRInstDesc
{
    IROp op;
    IRType* type;
    ArrayView<IRInst*> operands;   // borrowed, no ownership
    HashCode hashCode;
    void init();
};
```

with `Hash<IRInstKey>` / `IRInstKeyEqual` gaining `is_transparent` and overloads
for `IRInstDesc`, following the pattern and the argument-order caveat documented
on `ValKeyEqual` (`source/slang/slang-ast-builder.h:193-201`):

> Both argument orders are provided because a hash map performing a
> heterogeneous lookup may pass the stored key and the probe key to the
> comparator in either order [...] `ankerl::unordered_dense::map` compares
> `equal(probe, stored)` while `absl::flat_hash_map` and `tsl::robin_map`
> compare `equal(stored, probe)`.

Then the arena allocation only happens on a miss.

Note this depends on the general transparent-lookup work being viable — see
issue [02](02-transparent-heterogeneous-lookup.md) — though this particular case
can supply its own functors and does not need the `Dictionary` defaults changed.

### Part C — the sibling scan

Only if profiling justifies it. Options: maintain a monotonically increasing
order index per instruction within its parent (invalidated on reordering), or
restrict the scan with a bound. Do not attempt without data.

### Part D — the nested `canonicalizeInstOperands` lookup

`canonicalizeInstOperands` exists to normalise an array type's element-count
literal to `IntType`. The correct fix is probably upstream — whoever creates an
array type with a non-`IntType` element count should be creating it with the
canonical type in the first place, so the canonicalisation is a no-op. Trace the
producers before treating this as a container problem. If the canonicalisation
must stay, at least hoist `builder.getIntType()` out (it is a hoistable-inst
lookup on every call).

---

## Risks and things to watch

- Part A: the `hasReplacements` flag is computed once and the map cannot be
  mutated during the operand loop — verify that, since `tryGetValue` is
  non-mutating but the surrounding code is long.
- Part B changes the key type's shape; the transparent comparator must agree
  exactly with `IRInstKey::operator==` (`slang-ir.h:1962-1983`), including the
  order of the cheap checks, or deduplication silently changes.
- Deduplication changes are structurally load-bearing: a missed dedup produces
  duplicate types in the output, an incorrect dedup produces wrong code. Neither
  is subtle in the test suite, but both are subtle to debug.

---

## Validation

1. Instrument: count `m_instReplacementMap` probes and their hit rate on a large
   compile. If the hit rate is near zero, Part A is confirmed.
2. Instrument the operand-count distribution entering `_findOrEmitHoistableInst`
   — this sizes Parts A and B.
3. Instrument how often the sibling-scan branch is entered and its average
   length — this decides whether Part C is worth anything.
4. Full `sti` plus byte-identical output comparison; all parts should be
   output-identical.
5. `python3 ./extras/insttrace.py` is available if a dedup regression needs
   tracing to a creation site.

---

## Related

- [07](07-irconstantkey-hash-not-cached.md) — the constant map reached via
  `canonicalizeInstOperands`.
- [10](10-irinstkey-hash-recomputed-per-operation.md) — the key used here.
- [02](02-transparent-heterogeneous-lookup.md) — the transparent-lookup
  machinery Part B needs.
