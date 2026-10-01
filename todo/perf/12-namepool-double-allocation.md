# 12 — `NamePool::getName` allocates the name text up to three times

**Status:** not started
**Estimated size:** S
**Impact:** one heap allocation per _successful_ name lookup — and this runs once
per identifier token in every source file, including the whole core module

---

## Summary

```cpp
// source/compiler-core/slang-name.cpp:24-34
Name* NamePool::getName(UnownedStringSlice text)
{
    RefPtr<Name> name;
    if (names.tryGetValue(text, name))      // (1) allocates a String, probes, frees it
        return name;

    name = new Name();
    name->text = text;                      // (2) allocates — this one is necessary
    names.add(text, name);                  // (3) allocates a second owned copy
    return name;
}
```

with

```cpp
// source/compiler-core/slang-name.h:59-61
// The mapping from text strings to the corresponding name.
Dictionary<String, RefPtr<Name>> names;
```

Three allocations for a name that is seen for the first time, and — the part
that matters most — **one allocation on every lookup of a name that already
exists**, which is the overwhelmingly common case.

---

## Background

### Why `NamePool` exists

```cpp
// source/compiler-core/slang-name.h:13-24
// The key benefit of using `Name`s instead of raw strings is that `Name`s
// can be compared for equality just by testing pointer equality. Names
// also don't require any memory management; you can just retain an ordinary
// pointer to one and not deal with reference-counting overhead.
```

The design is right — interning so that downstream comparisons are pointer
comparisons. The cost is entirely in the interning step itself.

### Allocation (1) — the lookup

`names.tryGetValue(text, name)` calls `Dictionary::tryGetValue<UnownedStringSlice>`
(`source/core/slang-dictionary.h:236-243`), which forwards to `map.find(key)`.
Because neither `Slang::Hash<String>` nor `std::equal_to<String>` declares
`is_transparent`, the backend's heterogeneous `find` overload is SFINAE'd out and
overload resolution picks `find(Key const&)` with an implicit
`UnownedStringSlice → String` conversion. That conversion calls
`StringRepresentation::create(slice)`, which is an `operator new` plus a
`memcpy`.

Measured with an allocation-counting probe on an equivalent
`Dictionary<String, int>`:

```
slice lookups:  found=1000 allocs=1000
String lookups: found=1000 allocs=0
```

**One allocation per lookup, hit or miss.** See issue
[02](02-transparent-heterogeneous-lookup.md) for the general form of this
problem.

### Allocations (2) and (3) — the insert

`name->text = text` builds the owned copy that the `Name` will keep — necessary.

`names.add(text, name)` then needs a `String` key, and `Dictionary::add` has no
`template<typename K>` overload (`slang-dictionary.h:333-347`), so the slice
converts again, producing a **second independent owned copy of the same bytes**.
The dictionary key and `Name::text` now hold identical, separately-allocated
strings for the lifetime of the pool.

(Separately: `Dictionary::add` routes through `addIfNotExists` →
`tryGetValueOrAdd({key, value})`, which builds a pair temporary before probing —
see issue [05](05-dictionary-insert-eager-value-type-construction.md). For a
`String` key that is a refcount round-trip rather than an allocation, so it is
minor here.)

### Scale

`NamePool::getName` is called for every identifier the lexer produces, for every
name looked up during semantic checking, and for every synthesised name. The
core module alone contributes a large fixed cost on every compile. This is
plausibly among the highest-frequency `Dictionary` lookups in the compiler.

---

## Why this is independent of the selected hash function and map

Allocation (1) happens in the implicit conversion, before the map is entered.
Allocation (3) happens in `Dictionary::add`'s parameter conversion. Neither
depends on which hash or which map is selected.

---

## Proposed change

### Step 1 — eliminate allocation (1) via transparent lookup

This is issue [02](02-transparent-heterogeneous-lookup.md). Once
`Dictionary<String, V>` supports a heterogeneous `find` with
`UnownedStringSlice`, `getName`'s hit path becomes allocation-free.

Required correctness property, which must be tested rather than assumed:
`Hash<String>{}(s) == Hash<String>{}(UnownedStringSlice(s))`. It holds today
because `String::getHashCode()` delegates to `UnownedStringSlice::getHashCode()`
(`source/core/slang-string.h:797-801` and `:207-208`), but it is the invariant
the whole change rests on.

### Step 2 — eliminate allocation (3) by keying off the `Name`'s own storage

The pool already owns the bytes, in `Name::text`. Two shapes:

**(a) Key by `Name*` with a transparent string hash.** Change `names` to a
`HashSet`-like structure keyed by `Name*` whose hash and equality are defined in
terms of `name->text`, with transparent overloads accepting
`UnownedStringSlice`. This stores exactly one copy of the bytes.

Needs: a transparent functor pair, and care that `Name::text` is never mutated
after insertion (it is not — `Name` is written once in `getName` and never
again; confirm by grepping for writes to `Name::text`).

**(b) Key by a non-owning `UnownedStringSlice` pointing into `Name::text`.**
Simpler, but creates a lifetime coupling: the slice is only valid while the
`Name` is alive and its `String` buffer is not reallocated. Since `Name` is
`RefPtr`-held by the pool and never mutated, that holds — but it is a fragile
invariant to encode in a container. There is precedent in the codebase
(`HashSet<UnownedStringSlice> deferredWitnessTableEntryKeys` at
`source/slang/slang-ir-link.cpp:115`), so it is not unprecedented, just
delicate.

**Recommendation: (a).** It keeps ownership explicit and makes the "one copy of
the bytes" property structural rather than conventional.

### Step 3 — while here, fix `tryGetName`

```cpp
// source/compiler-core/slang-name.cpp:41-47
Name* NamePool::tryGetName(String const& text)
{
    RefPtr<Name> name;
    if (names.tryGetValue(text, name))
        return name;
    return nullptr;
}
```

This takes a `String const&`, so callers holding a slice must build one. Add an
`UnownedStringSlice` overload alongside it, mirroring `getName`'s pair of
overloads (`slang-name.cpp:24` and `:36`).

---

## Risks and things to watch

- `Name::text` must be immutable after interning for either step-2 shape to be
  sound. Verify by grep; `Name` has a single public field
  (`source/compiler-core/slang-name.h:33`) so this needs an actual audit, not an
  assumption.
- `NamePool` is shared across compilation units and sessions
  (`SharedASTBuilder::m_namePool`, `ASTBuilder::getNamePool()` at
  `source/slang/slang-ast-builder.h:293`). Check for thread-safety
  expectations before changing the container — if any path can call `getName`
  concurrently today, the change must not make that worse.
- Changing the key type changes iteration order of `names`. Grep for anything
  that iterates the pool; there should be nothing, but confirm.

---

## Validation

1. Allocation probe: assert 0 allocations for N lookups of an existing name, and
   exactly 1 for the first insertion of a new name.
2. Count `StringRepresentation::createWithCapacityAndLength` calls over a full
   `slangc` run of a representative shader, before and after.
3. Full `sti`. Name interning underpins all name resolution, so breakage is
   immediate and obvious.
4. Measure peak RSS: step 2 removes one owned copy of every distinct identifier
   in the program, including the entire core module. The saving should be visible.

---

## Related

- [02](02-transparent-heterogeneous-lookup.md) — the general fix that step 1
  depends on.
- [05](05-dictionary-insert-eager-value-type-construction.md) — the pair
  temporary in `Dictionary::add`.
- [06](06-string-hash-not-cached.md) — after the allocation is gone, the
  remaining per-lookup cost is rescanning the identifier's bytes to hash them.
