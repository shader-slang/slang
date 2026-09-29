# 06 — `String` never caches its hash; every probe rescans the bytes

**Status:** not started
**Estimated size:** M
**Impact:** O(length) work on every probe into any of the 95 string-keyed
dictionaries, and on every stored key during every table growth

---

## Summary

```cpp
// source/core/slang-string.h:797-801
static constexpr bool kHasUniformHash = true;
HashCode64 getHashCode() const
{
    return Slang::getHashCode(StringRepresentation::asSlice(m_buffer));
}
```

`String`'s hash is computed from scratch on every call. `StringRepresentation`
— the refcounted heap block that backs every non-empty `String` — has room for a
cached hash and does not have one:

```cpp
// source/core/slang-string.h:285-292
class SLANG_RT_API StringRepresentation : public RefObject
{
public:
    Index length;
    Index capacity;
    ...
};
```

Two consequences, and the second is the one usually overlooked:

1. Every lookup re-scans the whole string.
2. **Every table growth re-scans every stored string.** A `Dictionary<String, V>`
   growing from 2^k to 2^(k+1) entries rehashes all 2^k stored keys, reading
   every byte of every one of them. Over the life of a map that grows to N
   entries, that is ~2N full string scans of pure rehash cost, on top of the
   per-lookup cost.

The codebase already knows this is a problem — `ImmutableHashedString` exists
precisely to cache the hash — but it is used in only a handful of places.

---

## Background

### The hash path for a `String`

```
String::getHashCode()                                   slang-string.h:798
  -> Slang::getHashCode(UnownedStringSlice)             slang-hash.h:130-134 (the Hash<T> wrapper)
     -> UnownedStringSlice::getHashCode()               slang-string.h:208
        -> Slang::getHashCode(m_begin, len)             slang-hash.h:136-139
           -> HashImpl::hashBytes(buffer, len)          slang-hash-impl.h:84/99/112/126
```

`hashBytes` is the only part that varies with `SLANG_HASH`. The byte scan
happens regardless.

### The existing cached variant

```cpp
// source/core/slang-string.h:806-855
class ImmutableHashedString
{
public:
    String slice;
    HashCode64 hashCode;

    ImmutableHashedString(const UnownedStringSlice& slice)
        : slice(slice), hashCode(slice.getHashCode())
    {
    }
    ...
    bool operator==(const ImmutableHashedString& other) const
    {
        return hashCode == other.hashCode && slice == other.slice;
    }
    HashCode64 getHashCode() const { return hashCode; }
};
```

Used by:

- `IRSpecSharedContext::symbols` and `isImportedSymbol` — `source/slang/slang-ir-link.cpp:56-60`
- `IRModule::m_mapMangledNameToGlobalInst` — `source/slang/slang-ir.h:2310`

Everything else keyed by text uses plain `String`.

### Scale

95 declarations in `source/` match `Dictionary<String` / `HashSet<String` /
`Dictionary<Slang::String`. The `NamePool` one
(`source/compiler-core/slang-name.h:61`) is on the path of every identifier in
every source file.

---

## Why this is independent of the selected hash function

`SLANG_HASH` changes _which_ function walks the bytes, not _whether_ they are
walked, nor how many times. All four hash implementations (`wyhash`, `boost`,
`absl`, `std`) are linear in the string length. Caching removes the walk
entirely for repeat hashes, which no choice of hash function can do.

---

## Proposed change

Add a lazily-computed cached hash to `StringRepresentation`.

```cpp
class SLANG_RT_API StringRepresentation : public RefObject
{
public:
    Index length;
    Index capacity;
    mutable HashCode64 cachedHash;   // 0 == not yet computed
    ...
};
```

```cpp
HashCode64 String::getHashCode() const
{
    if (!m_buffer)
        return Slang::getHashCode(UnownedStringSlice());
    if (!m_buffer->cachedHash)
        m_buffer->cachedHash = Slang::getHashCode(StringRepresentation::asSlice(m_buffer));
    return m_buffer->cachedHash;
}
```

### The invalidation problem — this is the hard part

`StringRepresentation` is **mutable**. `String` and `StringBuilder` write through
it. Every mutation point must invalidate the cache. Enumerate them before
writing any code:

- `StringRepresentation::setContents(const UnownedStringSlice&)` — declared at
  `slang-string.h:298`, defined in `slang-string.cpp`.
- `StringRepresentation::ensureCapacity` / `cloneWithCapacity` / `clone`
  (`slang-string.h:344-360`) — these produce a _new_ representation, so the new
  one must start with `cachedHash = 0`. Note the existing `cloneWithCapacity`
  has what looks like a `memcpy` argument-order bug
  (`memcpy(getData(), newObj->getData(), length + 1)` copies _from_ the new
  object _into_ the old one) — investigate that separately, it is not part of
  this issue but should not be perpetuated.
- Everything in `StringBuilder`, which derives from `String`
  (`slang-string.h:857`) and appends in place.
- Any code that calls `getData()` (non-const) and writes through the returned
  `char*`. Grep for this; `getData()` returning a mutable `char*`
  (`slang-string.h:294`) is the escape hatch that makes a cached hash unsound if
  any caller uses it to mutate.

**`StringBuilder` is the reason to be careful.** It is used to build strings
incrementally and is the base of most string production in the compiler; if it
inherits a stale-hash hazard the bug will be silent and data-dependent.

### Safer alternatives if invalidation proves messy

**Alternative A — cache only in `StringBuilder`-free contexts.** Make the cache
live on a new immutable string type rather than on `StringRepresentation`, and
migrate the hot dictionaries to it. This is what `ImmutableHashedString` already
is. The work then becomes "use `ImmutableHashedString` for the hot
`Dictionary<String, …>`s" rather than "make `String` cache".

Candidates in rough priority order:

- `NamePool::names` (`slang-name.h:61`)
- the `Dictionary<String, …>` members in the emit and reflection layers

**Alternative B — invalidate unconditionally on any non-const access.** Set
`cachedHash = 0` in the non-const `getData()` and in every mutating method. This
is conservative and correct but means `StringBuilder`-produced strings never
benefit until they are copied.

**Recommendation:** do Alternative A first. It is strictly safe, it captures most
of the benefit (the hot maps are the ones that matter), and it can land
independently. Treat "make `String` itself cache" as a follow-up that needs a
careful audit of `StringBuilder` and `getData()`.

### Interaction with `kHasUniformHash`

`String` declares `kHasUniformHash = true` (`slang-string.h:797`), which makes
`Slang::Hash<String>` advertise `is_avalanching` and therefore makes ankerl and
boost **skip** their own mixing. That is a separate question — see issue
[14](14-khasuniformhash-audit.md) — but note that caching does not change it
either way, since the cached value is the same value.

---

## Risks and things to watch

- **Stale hash after mutation is a silent, data-dependent wrong-answer bug**: a
  mutated string would hash as its old contents and be found (or not found)
  incorrectly. Any implementation must be accompanied by a test that mutates a
  `String`/`StringBuilder` and re-looks-it-up.
- `StringRepresentation` grows by 8 bytes. It is allocated as
  `sizeof(StringRepresentation) + capacity + 1` (`slang-string.h:337-346`), so
  every string pays 8 more bytes. For a compiler that interns heavily this is
  probably a win overall, but measure RSS.
- `String` is in `SLANG_RT_API` — `StringRepresentation` is exported. Changing
  its layout is an ABI change for anything linking `libslang-rt` that was built
  against the old header. Check whether that matters for the shipped artifacts
  (`slang-rt` is a public runtime library).
- The `cachedHash == 0` sentinel collides with a legitimately-zero hash. Either
  accept the (rare) recomputation, or use a separate flag bit. Do not use a
  separate `bool` — that costs 8 bytes of padding; steal a bit or accept the
  collision.

---

## Validation

1. Unit test: hash a `String`, mutate it via `StringBuilder`, hash again, assert
   the hash changed and that a `Dictionary` lookup finds the new value.
2. Unit test: `String(s).getHashCode() == UnownedStringSlice(s).getHashCode()`
   for a corpus including the empty string — this equality is relied on by issue
   [02](02-transparent-heterogeneous-lookup.md) and must not regress.
3. Benchmark: a microbenchmark of N lookups into a `Dictionary<String, int>`
   with keys of varying length, before and after.
4. Measure peak RSS on a large compile (the 8-byte-per-string growth).

---

## Related

- [02](02-transparent-heterogeneous-lookup.md) — removes the _allocation_ per
  slice lookup; this issue removes the remaining _byte scan_. They compose.
- [12](12-namepool-double-allocation.md) — the hottest string-keyed map.
- [13](13-ir-link-mangled-name-lookups.md) — the existing
  `ImmutableHashedString` users.
- [14](14-khasuniformhash-audit.md) — `String`'s `kHasUniformHash` declaration.
