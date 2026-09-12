# 09 — `SpvInstKey` holds two heap `List`s with an uncached hash, and the call site inserts twice

**Status:** not started
**Estimated size:** M
**Impact:** two heap allocations and a full instruction-word rescan per emitted
SPIR-V instruction on the memoised paths; doubled again on a memo miss

---

## Summary

The SPIR-V emitter memoises instructions by their fully encoded word sequence.
The key type:

```cpp
// source/slang/slang-emit-spirv.cpp:1926-1952
struct SpvInstKey
{
    List<SpvWord> instWords;
    List<SpvWord> extraKeyData;
    bool operator==(const SpvInstKey& other) const
    {
        return instWords == other.instWords && extraKeyData == other.extraKeyData;
    }
    const static bool kHasUniformHash = true;
    // Spelled out rather than deduced with `auto`, because this is a nested
    // class: see the comment on SLANG_COMPONENTWISE_HASHABLE_1 in
    // slang-hash.h for why a deduced return type here would make
    // HasSlangHash<SpvInstKey> false at the point m_memoizedSpvInsts below
    // is declared.
    HashCode64 getHashCode() const
    {
        const auto instWordsHash = Slang::getHashCode(
            reinterpret_cast<const char*>(instWords.getBuffer()),
            instWords.getCount() * sizeof(SpvWord));
        const auto extraKeyDataHash = Slang::getHashCode(
            reinterpret_cast<const char*>(extraKeyData.getBuffer()),
            extraKeyData.getCount() * sizeof(SpvWord));
        return combineHash(instWordsHash, extraKeyDataHash);
    }
};

Dictionary<SpvInstKey, SpvInst*> m_memoizedSpvInsts;
```

Four distinct problems:

1. **Two heap-allocated `List`s per key.** Per probe key _and_ per stored entry.
2. **Uncached hash.** Rescans the whole encoded word array on every probe and on
   every stored key during every table growth.
3. **Double insert at the call site.** `tryGetValue(key)` then
   `m_memoizedSpvInsts[key] = spvInst` — two hashes, two probes, and a full key
   copy (two more allocations).
4. **`kHasUniformHash = true`** suppresses the backend's own remixing of a hash
   produced by `combineHash` — see issue [14](14-khasuniformhash-audit.md).

---

## Background

### The call sites

```cpp
// source/slang/slang-emit-spirv.cpp:1405-1447 (abridged)
List<SpvWord> ourOperands;
{
    auto scopePeek = OperandMemoizeScope(this);
    f();
    // Steal our operands back, so we don't have to calculate them again
    ourOperands = std::move(m_operandStack);
}

// Hash the opcode, encoded operands, and any caller-provided key data.
SpvInstKey key;
key.instWords.add(opcode);
key.instWords.addRange(ourOperands);      // allocation #1
key.extraKeyData = std::move(extraKeyData);

// If we have seen this before, return the memoized instruction
if (SpvInst** memoized = m_memoizedSpvInsts.tryGetValue(key))   // hash #1, probe #1
{
    if (irInst)
        m_mapIRInstToSpvInst.addIfNotExists(irInst, *memoized);
    return *memoized;
}

InstConstructScope scopeInst(this, opcode, irInst);
SpvInst* spvInst = scopeInst;
m_memoizedSpvInsts[key] = spvInst;        // hash #2, probe #2, key copy => allocations #2 and #3
```

The identical shape repeats in `emitInstMemoizedNoResultIDCustomOperandFunc` at
`slang-emit-spirv.cpp:1449-1491`.

Note `key.extraKeyData = std::move(extraKeyData)` is a move (good), but the
subsequent `operator[]` copies the whole key including both lists, so the move
is undone.

### How often this runs

These are `emitInstMemoized*`, the path for every type, constant, decoration and
capability-bearing instruction in the SPIR-V output — i.e. the deduplicated
portion of the module, which for a typical shader is most of the global section.
Memoisation is exactly why they are on this path, so the _hit_ rate is high, and
the hit path is the one that pays allocations #1 plus a full rescan.

---

## Evidence

The `List` allocations are **not measured** — the allocation probe used elsewhere
in this directory cannot observe `Slang::List`, which does not route through the
global `operator new`. Confirm with a profiler or a `List`-level counter before
investing.

The uncached-hash and double-probe claims are direct from the code and are not in
doubt.

---

## Why this is independent of the selected hash function and map

The allocations, the double probe, and the repeated `getHashCode()` invocation
all happen outside the hash function and outside the map's probing. `SLANG_HASH`
only changes the `hashBytes` call at the end of `getHashCode`.

---

## Proposed change

### Step 1 — collapse the double insert (smallest, highest confidence)

Replace `tryGetValue` + `operator[]` with a single find-or-insert. If issue
[04](04-dictionary-find-iterator-api-and-double-lookups.md) has landed a
`Dictionary::tryEmplace`, use it:

```cpp
auto [it, inserted] = m_memoizedSpvInsts.tryEmplace(std::move(key), nullptr);
if (!inserted)
{
    if (irInst)
        m_mapIRInstToSpvInst.addIfNotExists(irInst, it->second);
    return it->second;
}
InstConstructScope scopeInst(this, opcode, irInst);
SpvInst* spvInst = scopeInst;
HashMapImpl::valueOf(it) = spvInst;
...
```

Care: `InstConstructScope` mutates emitter state, so the entry has to be
reserved with a placeholder and filled in afterwards, or the scope has to run
before the insert. Read `InstConstructScope`'s constructor/destructor before
choosing — it pushes onto `m_operandStack` and the ordering matters.

Alternatively use the existing `tryGetValueOrAdd`:

```cpp
if (SpvInst** found = m_memoizedSpvInsts.tryGetValueOrAdd(std::move(key), nullptr))
    { /* hit */ }
```

which is one probe today, at the cost of inserting a placeholder on the miss
path that must then be overwritten.

### Step 2 — cache the hash in the key

```cpp
struct SpvInstKey
{
    List<SpvWord> instWords;
    List<SpvWord> extraKeyData;
    HashCode64 hashCode = 0;

    void init();      // call after both lists are populated
    HashCode64 getHashCode() const { return hashCode; }

    bool operator==(const SpvInstKey& other) const
    {
        return hashCode == other.hashCode
            && instWords == other.instWords
            && extraKeyData == other.extraKeyData;
    }
};
```

Same `init()`-discipline hazard as issue
[08](08-irsimplespecializationkey-list-key.md): a key used before `init()` hashes
to 0. Prefer making the lists private with mutators that invalidate, or assert in
`getHashCode()`.

**Important:** this interacts with `kHasUniformHash`. See step 4.

### Step 3 — inline storage

Most SPIR-V instructions are short — an opcode plus a handful of operand words.
Replace `List<SpvWord>` with `ShortList<SpvWord, N>`, choosing `N` from a measured
distribution of `instWords.getCount()` on a representative corpus rather than
guessing. `extraKeyData` is empty for the majority of call sites (the
`NoResultID` variant does not set it at all), so it is a strong candidate for a
small inline capacity.

Consider instead flattening to a **single** list: `extraKeyData` exists to
disambiguate instructions whose encoded words coincide but which should not be
deduplicated. If the two can be concatenated with a length prefix, the key
becomes one buffer, one hash, one comparison. Check the call sites that pass
`extraKeyData` to see whether concatenation would introduce ambiguity — a length
prefix normally prevents it.

### Step 4 — resolve the `kHasUniformHash` question

`kHasUniformHash = true` makes `Slang::Hash<SpvInstKey>` advertise
`is_avalanching`, so ankerl and boost skip their remix
(`ankerl/unordered_dense.h:896-910`, `boost/unordered/detail/foa/core.hpp:1429-1434`).

The claim is questionable today: the value is `combineHash(a, b)` of two
avalanching hashes, and `combineHash` is `(a * 16777619) ^ b`
(`slang-hash.h:196-209`), which does not avalanche in general — see issue
[01](01-combine-hash-no-finalisation.md). Here it happens to be _nearly_ fine,
because `b` is itself an avalanching 64-bit hash and dominates the low bits. But
it is an assertion the code makes without justification.

If issue 01 lands a proper fold, this marker becomes correct by construction.
Until then, either remove the marker (costing a remix, gaining safety) or
document why it holds. Do not leave it unexamined. Tracked as issue
[14](14-khasuniformhash-audit.md).

---

## Risks and things to watch

- The SPIR-V memo table is load-bearing for output correctness: two IR
  instructions that must map to the same SPIR-V instruction (the code comments
  give `Ptr<T>` and `Ref<T>` as an example) rely on it. Any change to key
  equality or key content changes deduplication behaviour and therefore the
  emitted module.
- If step 3 changes the hash _value_, the memo table's iteration order changes.
  SPIR-V instructions are emitted through `parent->addInst(spvInst)`, which is
  order-driven by the emitter rather than by the map, so this should be safe —
  but confirm with SPIR-V output diffs, not by reasoning.
- `SLANG_RUN_SPIRV_VALIDATION=1` must be set when validating; do not use the
  system `spirv-val`.

---

## Validation

1. Instrument `instWords.getCount()` and `extraKeyData.getCount()` distributions
   on a representative corpus; record in the PR and use to pick inline capacities.
2. Byte-compare emitted SPIR-V before and after for a broad set of shaders —
   this change must be output-identical.
3. `SLANG_RUN_SPIRV_VALIDATION=1` on the full SPIR-V test set.
4. The RTX Remix corpus (`extras/repro-remix.md` / the `/repro-remix` skill) is a
   good large-scale SPIR-V stress case for both correctness and timing.

---

## Related

- [01](01-combine-hash-no-finalisation.md) — `combineHash` quality.
- [04](04-dictionary-find-iterator-api-and-double-lookups.md) — the
  find-or-insert API this needs.
- [08](08-irsimplespecializationkey-list-key.md) — the same anti-pattern in the
  specialiser.
- [14](14-khasuniformhash-audit.md) — the `kHasUniformHash` marker.
