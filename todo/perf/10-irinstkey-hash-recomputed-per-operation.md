# 10 — `IRInstKey` is rebuilt (and rehashed) per map operation; removal probes twice

**Status:** not started
**Estimated size:** M
**Impact:** O(operand count) hashing per global-value-numbering map operation,
doubled on every removal, multiplied by the transitive user set on invalidation

---

## Summary

`IRInstKey` gets the caching right for _rehash_ — it stores the hash in the key,
so table growth is free:

```cpp
// source/slang/slang-ir.h:1943-1961
struct IRInstKey
{
private:
    IRInst* inst = nullptr;
    HashCode hashCode = 0;
    HashCode _getHashCode();

public:
    IRInstKey() = default;
    IRInstKey(const IRInstKey& key) = default;
    IRInstKey(IRInst* i)
        : inst(i)
    {
        hashCode = _getHashCode();
    }
    IRInstKey& operator=(const IRInstKey&) = default;
    HashCode getHashCode() const { return hashCode; }
    IRInst* getInst() const { return inst; }
    ...
};
```

But the hash lives only in the transient key, never on the `IRInst`. Since a
fresh `IRInstKey` is constructed for every lookup, insert and removal, the
operand-folding loop runs once per map _operation_:

```cpp
// source/slang/slang-ir.cpp:2236-2249
HashCode IRInstKey::_getHashCode()
{
    auto code = Slang::getHashCode(inst->getOp());
    code = combineHash(code, Slang::getHashCode(inst->getFullType()));
    code = combineHash(code, Slang::getHashCode(inst->getOperandCount()));

    auto argCount = inst->getOperandCount();
    auto args = inst->getOperands();
    for (UInt aa = 0; aa < argCount; ++aa)
    {
        code = combineHash(code, Slang::getHashCode(args[aa].get()));
    }
    return code;
}
```

And the removal path constructs the key **twice**.

---

## Background

### The map

```cpp
// source/slang/slang-ir.h:2031-2032
typedef Dictionary<IRInstKey, IRInst*> GlobalValueNumberingMap;
typedef Dictionary<IRConstantKey, IRConstant*> ConstantMap;
```

This is the hash-consing table for all hoistable instructions — types, witness
tables, generic applications, and everything else `getIROpInfo(op).isHoistable()`
says can be deduplicated at module scope. It is one of the busiest maps in the
compiler.

### The double-probe removal

```cpp
// source/slang/slang-ir.h:2043-2053
void _removeGlobalNumberingEntry(IRInst* inst)
{
    IRInst* value = nullptr;
    if (m_globalValueNumberingMap.tryGetValue(IRInstKey{inst}, value))
    {
        if (value == inst)
        {
            m_globalValueNumberingMap.remove(IRInstKey{inst});
        }
    }
}
```

Two `IRInstKey{inst}` constructions, so `_getHashCode()` runs twice, so the
operand loop runs twice. Then two probes.

### The multiplier

```cpp
// source/slang/slang-ir-deduplicate.cpp:14-42 (abridged)
void IRDeduplicationContext::removeHoistableInstFromGlobalNumberingMap(IRInst* instToRemove)
{
    InstHashSet userWorkListSet(instToRemove->getModule());
    InstWorkList userWorkList(instToRemove->getModule());
    ...
    for (Index i = 0; i < userWorkList.getCount(); i++)
    {
        auto inst = userWorkList[i];
        if (getIROpInfo(inst->getOp()).isHoistable())
        {
            _removeGlobalNumberingEntry(inst);
            for (auto use = inst->firstUse; use; use = use->nextUse)
                addToWorkList(use->getUser());
        }
    }
}
```

Every transitive user of a removed hoistable instruction pays the doubled cost.
For a type that many things depend on, that user set is large.

### The insert side

```cpp
// source/slang/slang-ir.h:2037-2041
void _addGlobalNumberingEntry(IRInst* inst)
{
    m_globalValueNumberingMap.add(IRInstKey{inst}, inst);
    m_instReplacementMap.remove(inst);
    tryHoistInst(inst);
}
```

One key construction — fine — but note it also unconditionally probes
`m_instReplacementMap`, which is empty in most compilations. See issue
[11](11-findoremithoistableinst-per-operand-probes.md).

### The equality side

```cpp
// source/slang/slang-ir.h:1962-1983
bool operator==(IRInstKey const& right) const
{
    if (hashCode != right.getHashCode())
        return false;
    if (getInst()->getOp() != right.getInst()->getOp())
        return false;
    if (getInst()->getFullType() != right.getInst()->getFullType())
        return false;
    if (getInst()->operandCount != right.getInst()->operandCount)
        return false;

    auto argCount = getInst()->operandCount;
    auto leftArgs = getInst()->getOperands();
    auto rightArgs = right.getInst()->getOperands();
    for (UInt aa = 0; aa < argCount; ++aa)
    {
        if (leftArgs[aa].get() != rightArgs[aa].get())
            return false;
    }
    return true;
}
```

This is well written — the hash early-out is first, then the cheap scalar
checks, then the operand walk. No change needed here. It is listed only so the
reader knows the whole cost model: a probe is `O(operandCount)` hash plus, on a
fingerprint match, `O(operandCount)` comparison.

---

## Why this is independent of the selected hash function and map

The operand-folding loop is in Slang's own `_getHashCode`, and the double key
construction is in Slang's own `_removeGlobalNumberingEntry`. All 32 matrix
configurations pay both.

---

## Proposed change

Two independent pieces; do them in this order.

### Part A — fix the double probe in `_removeGlobalNumberingEntry` (small, safe)

Requires the `find`/`erase` API from issue
[04](04-dictionary-find-iterator-api-and-double-lookups.md):

```cpp
void _removeGlobalNumberingEntry(IRInst* inst)
{
    auto key = IRInstKey{inst};                       // one hash
    auto it = m_globalValueNumberingMap.find(key);    // one probe
    if (it != m_globalValueNumberingMap.end() && it->second == inst)
        m_globalValueNumberingMap.erase(it);
}
```

If issue 04 has not landed, an interim improvement is to hoist the key into a
local so at least `_getHashCode()` runs once:

```cpp
const IRInstKey key{inst};
IRInst* value = nullptr;
if (m_globalValueNumberingMap.tryGetValue(key, value) && value == inst)
    m_globalValueNumberingMap.remove(key);
```

That halves the hashing immediately at the cost of leaving the second probe.
This is a two-line change with no API dependency and should be done regardless.

### Part B — cache the hash on `IRInst`

The bigger win, and the riskier one. Store the hash on the instruction so
`IRInstKey(IRInst*)` becomes a load rather than a fold.

The obstacle is **invalidation**. `IRInst`'s operands and type are mutable —
`IRUse::init`, `set`, `replaceUsesWith`, operand rewriting in countless IR passes.
A stale cached hash silently breaks hash-consing: the instruction becomes
unfindable, deduplication stops working, and the symptom appears far away as
duplicate types in the output.

The existing code has a related mechanism worth studying first:
`removeHoistableInstFromGlobalNumberingMap` exists precisely because mutating a
hoistable instruction requires removing it from the numbering map first. If
every mutation of a hoistable instruction already goes through that path, then
that path is also the natural place to invalidate a cached hash.

**Investigate before implementing:**

1. Is every mutation of an operand of a _hoistable_ instruction already funnelled
   through `removeHoistableInstFromGlobalNumberingMap` / the dedup context? If
   yes, invalidation has an obvious home. If no, caching is unsafe without first
   establishing that invariant.
2. Where would the hash live? `IRInst` is size-sensitive — it is allocated from
   the module's `MemoryArena` in enormous numbers, and
   `_findOrEmitHoistableInst` sizes allocations as
   `sizeof(IRInst) + operandCount * sizeof(IRUse)` (`slang-ir.cpp:2825`). Adding
   8 bytes to every instruction to speed up the hoistable subset may be a net
   loss on memory. Consider a side table, or storing the hash only on
   hoistable instructions via a subclass/decoration, or packing into existing
   padding — check `IRInst`'s current layout for holes.
3. Is the win worth it? Instrument the operand-count distribution of instructions
   entering the numbering map. If the median is 2–3 operands, the fold is ~4
   `combineHash` steps and Part B may not pay for its risk. **Measure first.**

**Recommendation:** do Part A now. Treat Part B as conditional on the
measurement in (3) and on the invariant in (1) being provable.

---

## Risks and things to watch

- Part A is behaviour-preserving and low risk.
- Part B, done wrong, produces silent deduplication failures that manifest as
  bloated or subtly wrong output far from the cause. If pursued, add a debug-only
  validation mode that recomputes the hash and asserts it matches the cached
  one, enabled under `SLANG_ENABLE_IR_BREAK_ALLOC` or a similar debug flag.
- Adding a field to `IRInst` changes the arena allocation size for every
  instruction; check `_allocateInst` and `_createInst` (`slang-ir.cpp`) and the
  hand-computed `keySize` at `slang-ir.cpp:2825`.

---

## Validation

1. Instrument the operand-count distribution for instructions entering
   `m_globalValueNumberingMap`; put the histogram in the PR. This decides whether
   Part B is worth doing at all.
2. For Part A: count `_getHashCode` invocations on a large compile before and
   after; expect roughly a halving on removal-heavy workloads.
3. Full `sti`. Deduplication correctness is structural — if it breaks, many tests
   break.
4. Compare emitted output byte-for-byte; this should be output-identical.

---

## Related

- [01](01-combine-hash-no-finalisation.md) — `_getHashCode` uses `combineHash`
  over pointer operands, the exact shape with degenerate low bits.
- [04](04-dictionary-find-iterator-api-and-double-lookups.md) — provides the
  `find`/`erase` needed for Part A.
- [07](07-irconstantkey-hash-not-cached.md) — the sibling key, which does _not_
  cache and has the worse problem.
- [11](11-findoremithoistableinst-per-operand-probes.md) — the other half of the
  hoistable-inst cost model.
