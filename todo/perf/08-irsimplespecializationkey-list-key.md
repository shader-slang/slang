# 08 — `IRSimpleSpecializationKey` is a heap-allocated `List` with an uncached hash

**Status:** not started
**Estimated size:** M
**Impact:** one heap allocation per probe and per stored entry, plus O(n) hashing
per probe and per rehash, on the generics specialisation path

---

## Summary

```cpp
// source/slang/slang-ir-clone.h:155-173
struct IRSimpleSpecializationKey
{
    // The structure of a specialization key will be a list
    // of instructions, typically starting with the function,
    // generic, or other object to be specialized, and then
    // having one or more entries to represent the specialization
    // arguments.
    //
    List<IRInst*> vals;

    // In order to use this type as a `Dictionary` key we
    // need it to support equality and hashing.
    //
    // TODO: honestly we might consider having `getHashCode`
    // and `operator==` defined for `List<T>`.

    bool operator==(IRSimpleSpecializationKey const& other) const;
    HashCode getHashCode() const;
};
```

```cpp
// source/slang/slang-ir-clone.cpp:442-451
HashCode IRSimpleSpecializationKey::getHashCode() const
{
    auto valCount = vals.getCount();
    HashCode hash = Slang::getHashCode(valCount);
    for (Index ii = 0; ii < valCount; ++ii)
    {
        hash = combineHash(hash, Slang::getHashCode(vals[ii]));
    }
    return hash;
}
```

Three problems, all independent of the hash function:

1. **Heap-allocated key.** `List<IRInst*>` owns its storage. Every probe key
   allocates; every stored entry allocates; every copy of the key allocates.
2. **Uncached hash.** Recomputed on every probe _and_ on every stored key during
   every table growth.
3. **O(n) equality.** `operator==` walks the list again after a hash match
   (`slang-ir-clone.cpp:429-440`) — necessary, but it compounds with (2).

---

## Background

### Where it is used

```cpp
// source/slang/slang-ir-specialize.cpp:61
Dictionary<IRSimpleSpecializationKey, IRSpecialize*> activeGenericSpecializations;
```

```cpp
// source/slang/slang-ir-specialize.cpp:402
typedef IRSimpleSpecializationKey Key;
```

```cpp
// source/slang/slang-ir-specialize-function-call.cpp:317
typedef IRSimpleSpecializationKey Key;
```

and a construction site at `source/slang/slang-ir-specialize.cpp:3050`.

These are the caches that make generic specialisation not blow up
combinatorially, so they are probed once per candidate specialisation — which on
a heavily generic shader is a lot.

### Typical key shape

The first element is the thing being specialised (a generic or a function), and
the rest are the specialisation arguments. So a key is usually 2–6 entries, but
can be larger for generics with many parameters plus witness arguments.

At 2–6 entries of 8 bytes each, the data is 16–48 bytes — comfortably small
enough to live inline, yet it is behind a heap pointer.

---

## Evidence

Code-reading only; **not measured**. The allocation probe used elsewhere in this
directory cannot see `List` allocations because `Slang::List` does not route
through the global `operator new`:

```
100 List<int> copies: allocs=0
```

Before investing effort here, confirm the magnitude with a profiler or a
`List`-level allocation counter on a generics-heavy shader. The structural
argument is solid; the size of the win is not established.

---

## Why this is independent of the selected hash function and map

The allocation happens in `List`'s storage management, and the repeated
invocation of `getHashCode()` is forced by the key type carrying no cached hash.
Neither depends on `SLANG_HASH` or `SLANG_HASHMAP`.

---

## Proposed change

### Step 1 — cache the hash

```cpp
struct IRSimpleSpecializationKey
{
    List<IRInst*> vals;
    HashCode hashCode = 0;     // computed by init(), after vals is populated

    void init();               // sets hashCode from vals
    bool operator==(IRSimpleSpecializationKey const& other) const;
    HashCode getHashCode() const { return hashCode; }
};
```

This mirrors what `ValNodeDesc` already does (`source/slang/slang-ast-base.h:241-274`
and `source/slang/slang-ast-val.cpp:20-34`): populate operands, call `init()`,
then use it as a key. Follow that precedent for consistency.

`operator==` should then short-circuit on `hashCode` first, as `ValNodeDesc`'s
does:

```cpp
inline bool operator==(ValNodeDesc const& that) const
{
    if (hashCode != that.hashCode) return false;
    if (type != that.type) return false;
    if (operands.getCount() != that.operands.getCount()) return false;
    ...
}
```

**Hazard:** an `init()`-style API means a key used before `init()` silently
hashes to 0 and compares unequal to everything. Guard it — e.g. make `vals`
private with an `addVal()` that invalidates the hash, and `getHashCode()`
`SLANG_ASSERT` that `init()` has run. `ValNodeDesc` does not do this and relies
on discipline; do better here.

### Step 2 — inline storage

Replace `List<IRInst*>` with `ShortList<IRInst*, 8>`. `ShortList` is already the
choice for exactly this role in `ValNodeDesc`:

```cpp
// source/slang/slang-ast-base.h:246-247
SyntaxClass<NodeBase> type;
ShortList<ValNodeOperand, 8> operands;
```

With an inline capacity of 8, the overwhelming majority of specialisation keys
become allocation-free — both the transient probe key and the stored entry.

Check `ShortList`'s API surface covers what the call sites need (`add`,
`getCount`, `operator[]`, `addRange`, copy/move) and that copying a `ShortList`
into a map slot is well-behaved.

The trade-off: the stored entry becomes 8 × 8 = 64 bytes plus bookkeeping
inline, rather than a 24-byte `List` header plus an out-of-line block. For a map
with many entries that is more contiguous memory but fewer indirections. Measure
which wins for the actual key-count distribution — instrument
`vals.getCount()` on a representative corpus first, and pick the inline capacity
from the data rather than copying `ValNodeDesc`'s 8 by default.

### Step 3 (consider) — avoid rebuilding the key at all

The deeper observation: a specialisation key is `(thingBeingSpecialized, args...)`,
and both the generic and its argument list usually already exist as an
`IRSpecialize` instruction in the IR. If the lookup could be keyed off the
existing instruction rather than a freshly built list, the key construction
disappears. That is a design change to the specialisation caches, not a container
change — record it, do not bundle it here.

---

## Risks and things to watch

- If the key is mutated after being stored (it should never be), the cached hash
  goes stale and the entry becomes unfindable. Making `vals` private is the
  cheapest defence.
- `ShortList`'s inline buffer means the key is no longer cheap to move; check
  whether any call site relies on move-construction being O(1).
- `activeGenericSpecializations` and friends may be reasoned about for
  determinism somewhere; changing the container does not change iteration order
  here, but changing the _hash_ (which step 1 does not, and step 2 does not
  either) would. Neither step changes the hash value, so iteration order is
  preserved. Keep it that way, so this issue does not entangle with issue
  [01](01-combine-hash-no-finalisation.md).

---

## Validation

1. Instrument `vals.getCount()` over a generics-heavy compile to pick the inline
   capacity, and record the distribution in the PR.
2. Profile `specializeModule` / the function-call specialiser before and after.
3. Full `sti`, with particular attention to
   `tests/language-feature/generics/` and the autodiff tests (autodiff leans
   heavily on specialisation).
4. Confirm no change in emitted output — this is a pure cache-representation
   change and should be output-identical.

---

## Related

- [01](01-combine-hash-no-finalisation.md) — the `combineHash` fold used here,
  and why its low bits are degenerate for pointer lists exactly like this one.
- [05](05-dictionary-insert-eager-value-type-construction.md) — the insert path
  copies this key an extra time.
- [09](09-spvinstkey-list-key-and-double-insert.md) — the same anti-pattern in
  the SPIR-V emitter, with two lists instead of one.
