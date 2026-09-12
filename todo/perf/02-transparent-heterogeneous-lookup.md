# 02 — Heterogeneous lookup is silently disabled; every slice→`String` lookup heap-allocates

**Status:** not started
**Estimated size:** M
**Impact:** one `operator new` + `memcpy` + refcounted free per lookup, on paths
that run once per identifier and once per linked symbol

---

## Summary

`Slang::Dictionary` exposes lookup methods that _look_ like they support
heterogeneous keys:

```cpp
// source/core/slang-dictionary.h:208-223
template<typename K>
bool containsKey(const K& k) const { return map.find(k) != map.end(); }

template<typename K>
const TValue* tryGetValue(const K& key) const { auto i = map.find(key); ... }
```

They do not. Every backend gates its heterogeneous `find` overload on the hash
and the comparator both declaring `is_transparent`, and neither
`Slang::Hash<TKey>` nor the default `std::equal_to<TKey>` does. So `map.find(k)`
resolves to the _homogeneous_ `find(Key const&)` overload and the argument is
implicitly converted to `TKey`.

For `Dictionary<String, V>` looked up with an `UnownedStringSlice`, that implicit
conversion constructs a `String`, which heap-allocates a `StringRepresentation`
and copies the bytes — then throws it away after the probe.

---

## Background

### The declaration

```cpp
// source/core/slang-dictionary.h:102-113
template<
    typename TKey,
    typename TValue,
    typename Hash = Slang::Hash<TKey>,
    typename KeyEqual = std::equal_to<TKey>>
class Dictionary
{
    using InnerMap = HashMapImpl::Map<TKey, TValue, Hash, KeyEqual>;
    ...
};
```

`Slang::Hash<T>` (`source/core/slang-hash.h:92`) has no `is_transparent` member.
`std::equal_to<T>` (the non-`void` specialisation) has no `is_transparent`
member. The transparent specialisation is `std::equal_to<void>`.

### The gate in the backend

```cpp
// external/unordered_dense/include/ankerl/unordered_dense.h:472-487
template <typename T>
using detect_is_transparent = typename T::is_transparent;

template <typename Hash, typename KeyEqual>
constexpr bool is_transparent_v =
    is_detected_v<detect_is_transparent, Hash> && is_detected_v<detect_is_transparent, KeyEqual>;
```

```cpp
// external/unordered_dense/include/ankerl/unordered_dense.h:1808-1823
auto find(Key const& key) -> iterator { return do_find(key); }
auto find(Key const& key) const -> const_iterator { return do_find(key); }

template <class K, class H = Hash, class KE = KeyEqual,
          std::enable_if_t<is_transparent_v<H, KE>, bool> = true>
auto find(K const& key) -> iterator { return do_find(key); }
```

Both conditions must hold. With `Slang::Hash` and `std::equal_to<TKey>`, the
templated overload is SFINAE'd out, and overload resolution falls back to
`find(Key const&)` with an implicit conversion. `absl`, `boost::unordered` and
`tsl::robin_map` all gate the same way; `std::unordered_map` requires
`Hash::is_transparent && KeyEqual::is_transparent` too (C++20, and we build as
C++17 so it does not have the overload at all).

### The one place that gets it right

`ASTBuilder::m_cachedNodes` is declared with custom functors that _do_ declare
the marker:

```cpp
// source/slang/slang-ast-builder.h:186-210
template<>
struct Hash<ValKey>
{
    using is_transparent = void;
    auto operator()(const ValKey& k) const { return k.getHashCode(); }
    auto operator()(const ValNodeDesc& k) const { return Hash<ValNodeDesc>{}(k); }
};

struct ValKeyEqual
{
    using is_transparent = void;
    bool operator()(const Slang::ValKey& a, const Slang::ValKey& b) const { return a == b; }
    bool operator()(const Slang::ValNodeDesc& a, const Slang::ValKey& b) const { return b == a; }
    bool operator()(const Slang::ValKey& a, const Slang::ValNodeDesc& b) const { return a == b; }
};
```

Note the comment there about argument order: ankerl compares
`equal(probe, stored)` while absl and tsl compare `equal(stored, probe)`, so a
transparent comparator must accept both orders. Any fix here must do the same.

---

## Evidence

Measured with an allocation-counting probe against `build/Debug/lib`:

```cpp
Dictionary<String, int> d;
d.add(String("hello world this is a fairly long key"), 1);
d.add(String("another key entirely"), 2);

UnownedStringSlice s = UnownedStringSlice::fromLiteral("hello world this is a fairly long key");

g_allocs = 0;
for (int i = 0; i < 1000; ++i) if (d.containsKey(s)) found++;
// slice lookups: found=1000 allocs=1000

String sk = String("hello world this is a fairly long key");
g_allocs = 0;
for (int i = 0; i < 1000; ++i) if (d.containsKey(sk)) found++;
// String lookups: found=1000 allocs=0
```

**One heap allocation per lookup**, plus the `memcpy` of the key bytes, plus the
refcount-driven free. And, of course, the hash is then computed over the copy
rather than the original (see issue [06](06-string-hash-not-cached.md)).

### Scale

There are 95 declarations matching `Dictionary<String` / `HashSet<String` /
`Dictionary<Slang::String` in `source/`. The hot ones:

**`NamePool::names` — every identifier in every source file.**

```cpp
// source/compiler-core/slang-name.h:60-61
Dictionary<String, RefPtr<Name>> names;
```

```cpp
// source/compiler-core/slang-name.cpp:24-34
Name* NamePool::getName(UnownedStringSlice text)
{
    RefPtr<Name> name;
    if (names.tryGetValue(text, name))      // <-- allocates a String, probes, frees it
        return name;
    name = new Name();
    name->text = text;                      // <-- allocates again (this one is necessary)
    names.add(text, name);                  // <-- allocates a third time
    return name;
}
```

See issue [12](12-namepool-double-allocation.md) for the full treatment of this
function; the transparent-lookup fix removes the first of the three allocations
on the hit path, which is the one that dominates because it happens on _every_
lookup, not just on a miss.

**IR linking — every symbol resolved during `linkIR`.**

```cpp
// source/slang/slang-ir-link.cpp:56-57
typedef Dictionary<ImmutableHashedString, RefPtr<IRSpecSymbol>> SymbolDictionary;
SymbolDictionary symbols;
```

```cpp
// source/slang/slang-ir-link.cpp:2474-2481
if (auto linkage = originalVal->findDecoration<IRLinkageDecoration>())
{
    RefPtr<IRSpecSymbol> symbol;
    if (shared->symbols.tryGetValue(linkage->getMangledName(), symbol))
        return symbol->irGlobalValue;
}
```

`IRLinkageDecoration::getMangledName()` returns an `UnownedStringSlice` pointing
directly at the chars of an `IRStringLit` — zero copy. The implicit conversion to
`ImmutableHashedString` (whose constructor at `source/core/slang-string.h:814-817`
is not `explicit`) allocates a `String` copy of the mangled name _and_ hashes it,
purely to probe. `maybeCloneValue` is the inner loop of linking. See issue
[13](13-ir-link-mangled-name-lookups.md).

**SPIR-V emit.**

```cpp
// source/slang/slang-emit-spirv.cpp:1921-1924
bool hasExtensionDeclaration(const UnownedStringSlice& name)
{
    return m_extensionInsts.containsKey(name);
}
```

---

## Why this is independent of the selected hash function and map

The allocation happens _before_ the map is entered, in the implicit conversion
that overload resolution inserts. Every one of the 32 matrix configurations pays
it identically.

---

## Proposed change

Make `Dictionary`'s default hash and comparator transparent, so the backends'
heterogeneous `find` overloads become visible.

### Step 1 — a transparent comparator

The default cannot simply become `std::equal_to<void>`, because that would
change the behaviour for keys where `TKey`'s `operator==` is not defined against
the probe type — but that is exactly the set of cases that currently compile via
implicit conversion, so it needs care. A conservative shape:

```cpp
struct TransparentEqual
{
    using is_transparent = void;
    template<typename A, typename B>
    bool operator()(const A& a, const B& b) const { return a == b; }
};
```

and use it as `Dictionary`'s default `KeyEqual`.

### Step 2 — a transparent hash

`Slang::Hash<T>` is specialised per type, so the `is_transparent` marker has to
be added in a way that lets `Hash<String>{}(slice)` work. Two shapes:

- Add `using is_transparent = void;` to `Slang::Hash<T>` unconditionally and let
  its `operator()` stay a template over the argument type. `Hash<T>::operator()`
  currently takes `const T&`; making it a template that dispatches on
  `HasSlangHash` / `HasLibraryHash` of the _argument_ type rather than `T` would
  do it. Risk: it would then silently accept arguments whose hash is _not_
  consistent with `T`'s — e.g. `Hash<String>{}(someInt)` would compile and
  produce a hash that never matches any stored key. That is a correctness
  hazard, and it is exactly the hazard `is_transparent` normally puts on the
  author.
- Safer: introduce a dedicated `TransparentStringHash` / `TransparentStringEqual`
  pair and give `Dictionary<String, V>` a partial specialisation or a named
  alias (`StringDictionary<V>`) that uses them, then migrate the 95
  `Dictionary<String, …>` declarations over. More churn, no hazard.

**Recommendation:** start with the safe, targeted version for the string-ish key
types (`String`, `ImmutableHashedString`) since that is where all the measured
cost is, and leave the general case alone. Slang's own key types that want
heterogeneous lookup already opt in explicitly, as `ValKey` does.

### Step 3 — the insert side

`Dictionary::add` / `addIfNotExists` have **no** `template<typename K>` overload
(`slang-dictionary.h:312-347`), so even with transparent lookup, insertion from
a slice still converts. That is fine and mostly necessary — the map must own a
copy — but see issue [12](12-namepool-double-allocation.md) for a case where the
copy is redundant with one the caller already made.

### Consistency check

The three types involved already have the cross-type `operator==` needed:

- `String::operator==(const UnownedStringSlice&)` — `slang-string.h:677`
- `ImmutableHashedString::operator==(const UnownedStringSlice&)` — `slang-string.h:849`
- `UnownedStringSlice::getHashCode()` and `String::getHashCode()` both route
  through `Slang::getHashCode(ptr, len)` → `HashImpl::hashBytes`, so they agree
  by construction (`slang-string.h:207-208` and `:797-801`).

That last point is load-bearing: **a transparent hash is only correct if
`hash(slice) == hash(String(slice))`.** Verify it with a test, do not assume it.

---

## Risks and things to watch

- A transparent comparator that is too permissive turns "wrong key type" from a
  compile error into a silent always-miss. Add a static test that
  `Dictionary<String,int>::containsKey(someSlice)` finds an entry added as a
  `String`, so a hash/equality mismatch is caught.
- `ImmutableHashedString` compares `hashCode == other.hashCode && slice == other.slice`
  for the homogeneous case but `slice == other` for the slice case
  (`slang-string.h:841-849`). A transparent hash for it must produce
  `slice.getHashCode()` for a slice probe — which it does, since that is exactly
  how its own `hashCode` field is initialised — but confirm.
- Argument-order: supply both `(probe, stored)` and `(stored, probe)` overloads,
  per the comment on `ValKeyEqual` in `slang-ast-builder.h:193-201`. Failing to
  do so silently restricts which `SLANG_HASHMAP` backends compile.

---

## Validation

1. Extend the allocation probe: assert `allocs == 0` for 1000 slice lookups into
   a `Dictionary<String, int>` and a `Dictionary<ImmutableHashedString, int>`.
2. Unit test: `hash(UnownedStringSlice(s)) == hash(String(s))` for a corpus
   including the empty string (note `hashBytes` has a `len ? … : empty` special
   case in the absl and std implementations — `slang-hash-impl.h:112-129`).
3. Unit test: add via `String`, look up via `UnownedStringSlice` and
   `const char*`, expect a hit; under every `SLANG_HASHMAP` value.
4. Profile a `slangc` run before/after and look at `StringRepresentation::create`
   call counts.

---

## Related

- [06](06-string-hash-not-cached.md) — once the copy is gone, the remaining cost
  of a string lookup is rescanning the bytes to hash them.
- [12](12-namepool-double-allocation.md) — `NamePool::getName`.
- [13](13-ir-link-mangled-name-lookups.md) — IR linking symbol lookups.
