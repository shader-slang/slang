# 05 — `Dictionary` insert helpers materialise a `value_type` before probing

**Status:** not started
**Estimated size:** S
**Impact:** one wasted deep copy of key+value on every insert that hits an
existing key; two copies instead of one on a genuine insert

---

## Summary

`Dictionary`'s convenience insert overloads build a `std::pair<TKey, TValue>`
temporary _before_ the map is probed, then hand that temporary to
`map.insert(...)`, which copies it again into the table. The result:

- **Hit path** (key already present): one deep copy of key and value is
  constructed and immediately destroyed, for nothing.
- **Miss path**: two deep copies instead of one.

For `TValue` types that own heap storage — `List`, `String`, `ShortList`,
`RefPtr` chains — this is a real allocation, not just a register shuffle.

---

## Background

```cpp
// source/core/slang-dictionary.h:270-289
TValue* tryGetValueOrAdd(const typename InnerMap::value_type& kvPair)
{
    const auto& [iterator, inserted] = map.insert(kvPair);
    return inserted ? nullptr : std::addressof(HashMapImpl::valueOf(iterator));
}
TValue* tryGetValueOrAdd(typename InnerMap::value_type&& kvPair)
{
    const auto& [iterator, inserted] = map.insert(std::move(kvPair));
    return inserted ? nullptr : std::addressof(HashMapImpl::valueOf(iterator));
}
TValue* tryGetValueOrAdd(const TKey& key, const TValue& value)
{
    return tryGetValueOrAdd({key, value});          // <-- pair built here, by copy
}
```

```cpp
// source/core/slang-dictionary.h:293-297
TValue& getOrAddValue(const TKey& key, const TValue& defaultValue)
{
    auto [iterator, inserted] = map.insert({key, defaultValue});   // <-- same
    return HashMapImpl::valueOf(iterator);
}
```

```cpp
// source/core/slang-dictionary.h:312-347
bool addIfNotExists(const TKey& k, const TValue& v) { return addIfNotExists({k, v}); }
void add(const TKey& key, const TValue& value) { add({key, value}); }
```

The `{key, value}` braced initialiser constructs an `InnerMap::value_type`
(`std::pair<TKey, TValue>` for every backend) by copying both members. The
subsequent `map.insert(const value_type&)` copies again into the table slot on
success; on failure the temporary is simply destroyed.

Note that `HashSet::add` routes through here too:

```cpp
// source/core/slang-dictionary.h:427-428
bool add(const T& obj) { return dict.addIfNotExists(obj, _DummyClass()); }
```

For `HashSet`, `TValue` is `_DummyClass` so the value copy is free, but the
_key_ copy still happens on the hit path. For a `HashSet<String>` that is an
extra refcount round-trip; for `HashSet<IRInst*>` it is free.

---

## Evidence

This one is **not measured**. The allocation-counting probe used elsewhere in
this directory could not observe it, because `Slang::List` does not route its
storage through the global `operator new` / `operator new[]`:

```
100 List<int> copies: allocs=0
```

and at `-O2` the compiler hoisted the loop-invariant pair construction out of
the probe loop entirely, yielding `addIfNotExists on EXISTING key x1000: allocs=1`.

So the claim here rests on reading the code, not on a measurement. **Before
investing effort, confirm with either a `List`-level allocation counter or a
profiler.** The code-reading argument is straightforward and hard to dispute,
but the magnitude is unknown.

### Where the values are non-trivial

```cpp
// source/slang/slang-ast-builder.h:234-237
Dictionary<GenericDecl*, List<Val*>> m_cachedGenericDefaultArgs;
Dictionary<Decl*, ShortList<Decl*, 4>> m_substituteMap;
```

```cpp
// source/slang/slang-ir.h:2308-2316
Dictionary<IRInst*, IRAnalysis> m_mapInstToAnalysis;
Dictionary<ImmutableHashedString, List<IRInst*>> m_mapMangledNameToGlobalInst;
```

```cpp
// source/slang/slang-ir-link.cpp:56-60
typedef Dictionary<ImmutableHashedString, RefPtr<IRSpecSymbol>> SymbolDictionary;
Dictionary<ImmutableHashedString, bool> isImportedSymbol;
```

`Dictionary<ImmutableHashedString, …>` is doubly affected: the _key_ owns a
`String`, so even a "cheap" value type still means a refcount increment and
decrement per wasted pair construction, and `ImmutableHashedString`'s
constructor from a slice allocates outright (see issue
[13](13-ir-link-mangled-name-lookups.md)).

---

## Why this is independent of the selected hash function and map

The pair is constructed in `Dictionary`'s own inline code, before any backend
call. All 32 matrix configurations pay it.

---

## Proposed change

Route the key+value overloads through `try_emplace` instead of building a pair
and calling `insert`.

```cpp
TValue* tryGetValueOrAdd(const TKey& key, const TValue& value)
{
    auto [iterator, inserted] = map.try_emplace(key, value);
    return inserted ? nullptr : std::addressof(HashMapImpl::valueOf(iterator));
}

TValue& getOrAddValue(const TKey& key, const TValue& defaultValue)
{
    auto [iterator, inserted] = map.try_emplace(key, defaultValue);
    return HashMapImpl::valueOf(iterator);
}

bool addIfNotExists(const TKey& k, const TValue& v)
{
    return map.try_emplace(k, v).second;
}
bool addIfNotExists(TKey&& k, TValue&& v)
{
    return map.try_emplace(std::move(k), std::move(v)).second;
}
```

`try_emplace` is defined to **not** construct the mapped value if the key is
already present, which is exactly the semantics wanted. The key still has to be
copied to hash it, but only once, and only if the backend needs it — most
backends hash the key argument in place.

Keep the existing `value_type`-taking overloads as they are: those callers
already have a pair in hand.

### Backend availability

`try_emplace` is present on all eight backends. Verify the exact overload sets,
particularly:

- ankerl: `unordered_dense.h:1729-1735` (the `Key const&` / `Key&&` forms) and
  `:1761-1803` (the `is_transparent`-gated `K&&` forms).
- `std::unordered_map`: `try_emplace` is C++17, available.
- `tsl::robin_map`: has `try_emplace`; confirm the return type is
  `std::pair<iterator, bool>` and that `HashMapImpl::valueOf` works on that
  iterator (it does — `valueOf` already handles tsl's `value()` accessor at
  `slang-hashmap-impl.h:162-173`).

### Interaction with issue 04

If issue [04](04-dictionary-find-iterator-api-and-double-lookups.md) adds a
public `tryEmplace` to `Dictionary`, these internal helpers should be written in
terms of it rather than duplicating the `map.try_emplace` call. Doing 04 first
and then this one is the cleaner order, but either order works.

---

## Risks and things to watch

- `try_emplace` requires the key to be convertible/constructible in a way
  `insert` did not; for a key type with an odd constructor set this could change
  which overload is selected. The key types in use (`String`,
  `ImmutableHashedString`, raw pointers, `IRInstKey`, `ValKey`) are all
  straightforward.
- `TValue` must be constructible from the argument; with `try_emplace(key, value)`
  it is copy-constructed in place, which is what happens today anyway.
- `Dictionary::add(const TKey&, const TValue&)` asserts on a duplicate key
  (`slang-dictionary.h:339-345`). Preserve that: `try_emplace(...).second`
  returning false must still trigger `SLANG_ASSERT_FAILURE("The key already
exists in Dictionary.")`.

---

## Validation

1. Instrument `List`/`String` allocation directly (a counter in
   `StringRepresentation::createWithCapacityAndLength` and in `List`'s growth
   path) and confirm the hit-path copy is gone.
2. Unit test: a `Dictionary<int, MoveOnlyCounter>` style type that counts its own
   copies, asserting 0 value-copies on a hit and 1 on a miss.
3. Build under every `SLANG_HASHMAP` value.
4. Full `sti` — `add`'s duplicate-key assertion is behaviour that tests depend on.

---

## Related

- [04](04-dictionary-find-iterator-api-and-double-lookups.md) — the public
  `tryEmplace` this should be built on.
- [09](09-spvinstkey-list-key-and-double-insert.md) — a concrete call site where
  the key itself owns two `List`s and is copied on the miss path.
