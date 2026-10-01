# 03 — `HashSet<T>` is `Dictionary<T, _DummyClass>`, doubling the memory of every pointer set

**Status:** not started
**Estimated size:** M
**Impact:** 2x memory and 2x cache-line pressure on the IR worklist sets, which
are among the most frequently touched data structures in the compiler

---

## Summary

`Slang::HashSet<T>` is implemented as a map from `T` to a one-byte empty struct:

```cpp
// source/core/slang-dictionary.h:353-437
class _DummyClass
{
};

template<typename T, typename DictionaryType>
class HashSetBase
{
protected:
    DictionaryType dict;
    ...
    bool add(const T& obj) { return dict.addIfNotExists(obj, _DummyClass()); }
    bool contains(const T& obj) const { return dict.containsKey(obj); }
};

template<typename T>
class HashSet : public HashSetBase<T, Dictionary<T, _DummyClass>>
{ ... };
```

The backing store therefore holds `std::pair<T, _DummyClass>` per entry.
`_DummyClass` is 1 byte, but alignment padding rounds the pair up to 16 bytes for
an 8-byte `T`. Half of every entry — and half of every cache line the set
touches — is padding.

Every hash map backend selectable via `SLANG_HASHMAP` ships a real set type that
stores just the key. None of them is wired up in
`source/core/slang-hashmap-impl.h`.

---

## Evidence

Measured directly:

```
sizeof(std::pair<Foo*, _DummyClass>) = 16
sizeof(_DummyClass) = 1
```

### Where this matters

The IR passes build worklist-dedup sets constantly. The central one:

```cpp
// source/slang/slang-ir.h:2377-2412
struct InstHashSet
{
    HashSet<IRInst*>* set = nullptr;
    ContainerPool* pool = nullptr;

    InstHashSet(IRModule* module)
    {
        pool = &module->getContainerPool();
        set = module->getContainerPool().getHashSet<IRInst>();
    }
    ...
    bool add(IRInst* inst) { return set->add(inst); }
    bool contains(IRInst* inst) { return set->contains(inst); }
};
```

`InstHashSet` is a member of `InstPassBase` (`source/slang/slang-ir-inst-pass-base.h:17`),
so essentially every IR pass has one. Sites that allocate their own include
`slang-ir-cleanup-void.cpp`, `slang-ir-lower-binding-query.cpp`,
`slang-ir-lower-bit-cast.cpp`, `slang-ir-lower-coopvec.cpp`,
`slang-ir-defer-buffer-load.cpp`, `slang-ir-dominators.cpp`,
`slang-ir-eliminate-multilevel-break.cpp`, `slang-ir-deduplicate.cpp`, and many
more. There are 262 `.contains(` call sites in `source/`.

For a module with, say, 200 000 instructions, a full-module worklist set at a
0.8 load factor is 250 000 slots × 16 bytes = 4 MB, versus 2 MB if the value
were not there. On a machine with a 32 MB L3 shared between the build's parallel
jobs, that difference is real.

The `ContainerPool` (`IRModule::getContainerPool()`) recycles these sets, so the
allocation cost is already amortised — but the _footprint_ is not.

---

## Why this is independent of the selected hash function and map

It is a property of what is stored, not of how it is hashed or probed. All 32
matrix configurations pay 16 bytes per pointer-set entry.

---

## Proposed change

Add a `Set` alias next to the existing `Map` alias in
`source/core/slang-hashmap-impl.h`, and rebase `HashSet` on it.

```cpp
// source/core/slang-hashmap-impl.h — alongside the existing Map alias per backend
#if SLANG_HASHMAP_IMPL == SLANG_HASHMAP_UNORDERED_DENSE
template<typename TKey, typename Hash, typename KeyEqual>
using Set = ankerl::unordered_dense::set<TKey, Hash, KeyEqual>;
#elif SLANG_HASHMAP_IMPL == SLANG_HASHMAP_BOOST_FLAT
template<typename TKey, typename Hash, typename KeyEqual>
using Set = boost::unordered_flat_set<TKey, Hash, KeyEqual>;
...
#endif
```

Backend set types:

| `SLANG_HASHMAP` value | Set type                       | Header                                                                        |
| --------------------- | ------------------------------ | ----------------------------------------------------------------------------- |
| `UNORDERED_DENSE`     | `ankerl::unordered_dense::set` | `<ankerl/unordered_dense.h>` (already included)                               |
| `BOOST_FLAT`          | `boost::unordered_flat_set`    | `<boost/unordered/unordered_flat_set.hpp>`                                    |
| `BOOST_NODE`          | `boost::unordered_node_set`    | `<boost/unordered/unordered_node_set.hpp>`                                    |
| `BOOST_UNORDERED`     | `boost::unordered_set`         | `<boost/unordered/unordered_set.hpp>`                                         |
| `ABSL_FLAT`           | `absl::flat_hash_set`          | `<absl/container/flat_hash_set.h>`                                            |
| `ABSL_NODE`           | `absl::node_hash_set`          | `<absl/container/node_hash_set.h>`                                            |
| `TSL_ROBIN`           | `tsl::robin_set`               | `<tsl/robin_set.h>` (present at `external/robin-map/include/tsl/robin_set.h`) |
| `STD`                 | `std::unordered_set`           | `<unordered_set>`                                                             |

Then reimplement `HashSet<T>` directly over `HashMapImpl::Set` rather than over
`Dictionary`.

### API surface to preserve

`HashSetBase` (`slang-dictionary.h:358-431`) currently exposes:

- `begin()` / `end()` returning an `Iterator` that dereferences to `const T&`
  via `KeyValueDetail::getKey`. With a real set, `*it` is already the key, so the
  `Iterator` wrapper and the `KeyValueDetail::getKey` overloads
  (`slang-dictionary.h:76-85`) become unnecessary for `HashSet` — but they are
  still needed for `OrderedHashSet`, which is built on `OrderedDictionary` and
  is a genuinely different implementation. Do not delete them.
- `getCount()`, `getBucketCount()`, `clear()`, `clearAndDeallocate()`,
  `add()` returning `bool`, `remove()`, `contains()`.
- The variadic constructor `HashSetBase(Arg arg, Args... args)`.
- Copy and move assignment.

`clearAndDeallocate()` currently does

```cpp
// source/core/slang-dictionary.h:158-163
void clearAndDeallocate()
{
    InnerMap emptyMap(0, map.hash_function(), map.key_eq(), map.get_allocator());
    map.swap(emptyMap);
}
```

The equivalent set constructor signature must exist on all eight backends —
check this early, it is the most likely source of a backend-specific shim.

### Do not change `OrderedHashSet`

`OrderedHashSet` (`slang-dictionary.h:788-794`) is built on `OrderedDictionary`,
a bespoke implementation with a `LinkedList` and a `UIntSet` mark array. It is
out of scope here.

---

## Follow-on worth considering separately

For `HashSet<IRInst*>` specifically, a dense bitset indexed by a per-module
instruction index would beat any hash set — `UIntSet`
(`source/core/slang-uint-set.h`) already exists and is used exactly this way by
`OrderedDictionary`'s mark array. `IRModule` already has
`m_mapInstToUniqueId` (`source/slang/slang-ir.h:2316`) built on demand for
passes that need stable IDs. Turning `InstHashSet` into a bitset over that index
space would take the per-entry cost from 16 bytes to 1 bit.

That is a much larger and riskier change (it needs an index that is dense, stable
for the lifetime of the pass, and assigned to newly created insts), so it should
be its own issue if pursued. Record it here so it is not lost.

---

## Risks and things to watch

- `std::unordered_set` and the node-based sets have different iterator
  invalidation rules from their map counterparts — but they match their own map
  counterparts, so the existing `eraseAndAdvance` / `mutableIterator` reasoning
  in `slang-hashmap-impl.h:115-266` carries over. `HashSet` does not currently
  expose mutable iteration, so `mutableIterator` is not needed for it.
- `tsl::robin_set`'s iterator dereferences to `const Key&` already, so no
  `MutableValueIterator` shim is needed.
- Check for anywhere that reaches through `HashSetBase::dict` — it is `protected`,
  and `OrderedHashSet::getLast()` uses `this->dict.getLast().key`
  (`slang-dictionary.h:792`). That is on the `OrderedDictionary` path and is
  unaffected, but grep for other users before changing the base class shape.

---

## Validation

1. `static_assert` or a runtime print confirming the per-entry size dropped
   (compare `getBucketCount()` × entry size, or just check RSS on a large
   compile).
2. Build and run `sti` under every `SLANG_HASHMAP` value — this change touches
   the backend-shim layer, so it is exactly the kind of change that compiles for
   one backend and not another.
3. Measure peak RSS for `slangc` on a large shader (the RTX Remix corpus via
   `extras/repro-remix.md` is a good stress case) before and after.
