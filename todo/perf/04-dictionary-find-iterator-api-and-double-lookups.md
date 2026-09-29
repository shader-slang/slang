# 04 — `Dictionary` has no `find()`-returning-position API, forcing double probes across the compiler

**Status:** not started
**Estimated size:** L (the API change is small; the call-site migration is the work)
**Impact:** halves the probe count on a large number of hot paths

---

## Summary

`Dictionary`'s public surface is `containsKey`, `tryGetValue`, `getValue`,
`remove`, `operator[]`, `add`, `addIfNotExists`, `tryGetValueOrAdd`,
`getOrAddValue`, `set`. None of them return a reusable _position_. The
consequence is that the very common "look, then act on the result" pattern is
structurally forced into two full hash-and-probe operations.

Because the underlying maps all have `find()` returning an iterator, and all have
`erase(iterator)` and `insert`-at-hint style APIs, this is entirely avoidable.

There are 112 `containsKey(` call sites, 499 `tryGetValue(` call sites and 262
`.contains(` call sites in `source/`. Not all of them are double lookups, but a
substantial fraction are.

---

## Background

### The current API

```cpp
// source/core/slang-dictionary.h:206-243
template<typename K> bool containsKey(const K& k) const;
template<typename K> const TValue* tryGetValue(const K& key) const;
template<typename K> TValue* tryGetValue(const K& key);
template<typename K> bool tryGetValue(const K& key, TValue& value) const;
const TValue& getValue(const TKey& key) const;
TValue& getValue(const TKey& key);
// ... and, separately:
void remove(const TKey& key) { map.erase(key); }        // :166
TValue& operator[](const TKey& key) { return map[key]; } // :301
```

`tryGetValue` returning `TValue*` is _almost_ a position — but it cannot be used
for erasure, and it cannot be used to insert when the lookup missed.

`Dictionary::Iterator` / `ConstIterator` are already exposed
(`slang-dictionary.h:129-130`) and `begin()`/`end()` are public
(`:142-145`), so there is no encapsulation reason for `find()` to be absent.

---

## Evidence — concrete double-lookup sites

### 1. IR global value numbering removal — two `IRInstKey` constructions

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

Each `IRInstKey{inst}` construction runs `IRInstKey::_getHashCode()`
(`source/slang/slang-ir.cpp:2236-2249`), which is **O(operand count)** — it
folds the opcode, the full type pointer, the operand count, and every operand
pointer. So this function does `2 × O(operandCount)` hashing plus two probes,
where one of each would do.

This matters because `removeHoistableInstFromGlobalNumberingMap`
(`source/slang/slang-ir-deduplicate.cpp:14-42`) calls it for every _transitive
user_ of the instruction being removed:

```cpp
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
```

**Desired shape:**

```cpp
auto it = m_globalValueNumberingMap.find(IRInstKey{inst});
if (it != m_globalValueNumberingMap.end() && it->second == inst)
    m_globalValueNumberingMap.erase(it);
```

Note `HashMapImpl::eraseAndAdvance` (`source/core/slang-hashmap-impl.h:130-143`)
already documents the backend differences around `erase(iterator)` — Abseil
returns `void`, ankerl moves the last element into the erased slot. An
`erase(iterator)` exposed on `Dictionary` can just forward; only the
_iterate-and-erase_ case needs the shim, and that already exists.

### 2. SPIR-V instruction memoisation — `tryGetValue` then `operator[]`

```cpp
// source/slang/slang-emit-spirv.cpp:1418-1440
SpvInstKey key;
key.instWords.add(opcode);
key.instWords.addRange(ourOperands);
key.extraKeyData = std::move(extraKeyData);

if (SpvInst** memoized = m_memoizedSpvInsts.tryGetValue(key))   // probe 1, hash 1
{
    ...
    return *memoized;
}

InstConstructScope scopeInst(this, opcode, irInst);
SpvInst* spvInst = scopeInst;
m_memoizedSpvInsts[key] = spvInst;                               // probe 2, hash 2, key copy
```

The same shape repeats at `slang-emit-spirv.cpp:1467-1485`. `SpvInstKey`'s hash
is uncached and rescans the whole encoded instruction word array
(`slang-emit-spirv.cpp:1940-1950`), and `operator[]` copies the key — which
means copying **two `List<SpvWord>`s**, i.e. two heap allocations. See issue
[09](09-spvinstkey-list-key-and-double-insert.md).

**Desired shape:** `try_emplace`-style — one probe that either returns the
existing entry or reserves a slot.

### 3. IR link symbol resolution — up to three probes

```cpp
// source/slang/slang-ir-link.cpp:119-134
IRSpecSymbol* findSymbols(UnownedStringSlice mangledName)
{
    ImmutableHashedString hashedName(mangledName);     // allocates + hashes
    RefPtr<IRSpecSymbol> symbol;
    if (shared->symbols.tryGetValue(hashedName, symbol))   // probe 1
        return symbol;
    for (auto m : irModules)
        for (auto inst : m->findSymbolByMangledName(hashedName))
            insertGlobalValueSymbol(shared, inst);
    if (shared->symbols.tryGetValue(hashedName, symbol))   // probe 2
        return symbol;
    shared->symbols[hashedName] = nullptr;                 // probe 3 + key copy
    return nullptr;
}
```

Probe 1 and probe 2 are genuinely distinct (the map is mutated in between), but
probe 2 and probe 3 are not — on the miss path they are the same key and the map
has not changed. See issue [13](13-ir-link-mangled-name-lookups.md).

### 4. The IR worklist idiom — `contains` then `add`

This is the single most repeated instance. Representative:

```cpp
// source/slang/slang-ir-cleanup-void.cpp:34-37
if (workListSet.contains(inst))
    return;
workListSet.add(inst);
```

```cpp
// source/slang/slang-ir-lower-binding-query.cpp:93-96, and again at :128-131
if (workListSet.contains(inst))
    return;
workList.add(inst);
```

```cpp
// source/slang/slang-ir-lower-bit-cast.cpp:29-32
if (workList.contains(inst))
    return;
```

```cpp
// source/slang/slang-ir-lower-coopvec.cpp:67-70
if (workListSet.contains(inst))
    return;
```

```cpp
// source/slang/slang-ir-defer-buffer-load.cpp:136-139
if (!dom->dominates(rootBlock, block) || searchBlocks.contains(block))
    return;
searchBlocks.add(block);
```

```cpp
// source/slang/slang-ir-dominators.cpp:285-288
if (!visited.contains(succ))
{
    nodeStack.add(succ);
    ...
}
```

**This one needs no API change at all.** `HashSet::add` already returns whether
the insert happened:

```cpp
// source/core/slang-dictionary.h:427
bool add(const T& obj) { return dict.addIfNotExists(obj, _DummyClass()); }
```

So each of these collapses to a single probe:

```cpp
if (!workListSet.add(inst))
    return;
```

Some sites already do this — e.g. `slang-ir-eliminate-multilevel-break.cpp:158`
writes `if (info.blockSet.add(successor))`. It is purely inconsistent usage.

---

## Why this is independent of the selected hash function and map

A redundant probe is a redundant hash computation plus a redundant cache-missing
memory access, whichever hash and whichever map you pick.

---

## Proposed change

Split into two independently landable pieces.

### Part A — the mechanical `contains`-then-`add` cleanup (no API change)

Audit the 262 `.contains(` call sites for the "`contains` guard immediately
followed by `add` of the same element into the same set" pattern and rewrite to
`if (!set.add(x))`. This is behaviour-preserving and reviewable in bulk.

A grep that finds most of them:

```bash
grep -rn -A3 "\.contains(" source/ --include=*.cpp | grep -B1 "\.add("
```

Do **not** blindly rewrite: cases where the `contains` check guards something
other than the `add`, or where the `add` is conditional on more than the
membership test, must stay.

### Part B — add position-returning APIs to `Dictionary`

```cpp
// Lookup returning a position usable with erase()
template<typename K> Iterator find(const K& key);
template<typename K> ConstIterator find(const K& key) const;

// Erase at a known position
void erase(Iterator it);

// Single-probe find-or-insert, constructing the value only on insert
template<typename... Args>
std::pair<Iterator, bool> tryEmplace(const TKey& key, Args&&... args);
```

`tryEmplace` is the important one: it subsumes `tryGetValueOrAdd`,
`getOrAddValue` and the `tryGetValue`-then-`operator[]` pattern, and it avoids
constructing the value on the hit path (see issue
[05](05-dictionary-insert-eager-value-type-construction.md)).

**Backend compatibility to check before committing to the signatures:**

- `try_emplace` exists on all eight backends (ankerl `:1729-1735` and
  `:1761-1803`, boost, absl, tsl, std) — confirm the exact overload set,
  particularly the `is_transparent`-gated ones.
- `erase(iterator)` returns `void` on Abseil and an iterator elsewhere. For a
  bare `erase(it)` with no return, that difference does not matter. The existing
  `HashMapImpl::eraseAndAdvance` shim already handles the iterate-and-erase case.
- `Dictionary::Iterator` is `typename InnerMap::iterator`
  (`slang-dictionary.h:129`), but `begin()`/`end()` return
  `HashMapImpl::mutableIterator(map.begin())` (`:142`), which under
  `tsl::robin_map` is a _different_ wrapper type. A `find()` that returns
  something comparable against `end()` must be consistent about which of the two
  it returns. Decide this explicitly, and make `erase()` accept whichever it is.
  This is the single most likely place for this change to break the
  `SLANG_HASHMAP=tsl_robin` build.

### Part C — migrate the identified call sites

At minimum: `_removeGlobalNumberingEntry` (`slang-ir.h:2043`),
`emitInstMemoized*` (`slang-emit-spirv.cpp:1418, 1467`),
`findSymbols` (`slang-ir-link.cpp:119`).

---

## Risks and things to watch

- Exposing iterators widens `Dictionary`'s contract. The existing comment on
  `mutableIterator` (`slang-hashmap-impl.h:227-254`) explains that a mutable
  entry reference lets a caller assign to the _key_, leaving the entry in the
  wrong bucket. That hazard already exists for `begin()`/`end()`; `find()` does
  not make it worse, but the doc comment should repeat the warning.
- Iterator invalidation differs across backends (flat maps invalidate on rehash;
  node maps do not; ankerl's erase moves the last element). Any call site that
  holds a `find()` result across a mutation is a bug. Document that the returned
  position is only valid until the next mutation.

---

## Validation

1. Unit tests for `find`/`erase`/`tryEmplace` under every `SLANG_HASHMAP` value —
   this is a shim-layer change, so per-backend build coverage is essential.
2. For Part A, a full `sti` run: the rewrite is behaviour-preserving, so any
   test movement indicates a mis-rewrite.
3. Profile the IR dedup removal path before/after on a module with heavy
   specialisation.

---

## Related

- [05](05-dictionary-insert-eager-value-type-construction.md) — `tryEmplace`
  also fixes the eager value construction.
- [09](09-spvinstkey-list-key-and-double-insert.md) — the SPIR-V memo table.
- [10](10-irinstkey-hash-recomputed-per-operation.md) — why the double probe in
  `_removeGlobalNumberingEntry` is especially expensive.
- [13](13-ir-link-mangled-name-lookups.md) — `findSymbols`.
