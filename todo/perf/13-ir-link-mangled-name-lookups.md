# 13 — IR linking allocates and rehashes mangled names on every symbol lookup

**Status:** not started
**Estimated size:** M
**Impact:** one heap allocation plus a full mangled-name hash per symbol
resolution during `linkIR`, on a path that runs for every cloned global value

---

## Summary

The linker's symbol table is keyed by `ImmutableHashedString` — a type that
exists specifically to cache its hash:

```cpp
// source/slang/slang-ir-link.cpp:56-59
typedef Dictionary<ImmutableHashedString, RefPtr<IRSpecSymbol>> SymbolDictionary;
SymbolDictionary symbols;

Dictionary<ImmutableHashedString, bool> isImportedSymbol;
```

But every lookup is performed with an `UnownedStringSlice`, and
`ImmutableHashedString`'s converting constructor is not `explicit`. So overload
resolution silently inserts a conversion that **allocates a `String` copy of the
mangled name and hashes it**, purely to probe — then destroys it.

The cached-hash design is defeated on every lookup. It only helps the entries
already stored.

---

## Background

### The hot lookup

```cpp
// source/slang/slang-ir-link.cpp:2470-2482
virtual IRInst* maybeCloneValue(IRInst* originalVal) override
{
    // If `originalVal` has a linkage, and the current module already contains
    // a symbol with the same mangled name, then we will skip and return that
    // prexisting val.
    if (auto linkage = originalVal->findDecoration<IRLinkageDecoration>())
    {
        RefPtr<IRSpecSymbol> symbol;
        if (shared->symbols.tryGetValue(linkage->getMangledName(), symbol))
        {
            return symbol->irGlobalValue;
        }
    }
    ...
}
```

`IRLinkageDecoration::getMangledName()` returns an `UnownedStringSlice` pointing
directly at the character data of an `IRStringLit` — deliberately zero-copy.
`shared->symbols` is keyed by `ImmutableHashedString`. The conversion:

```cpp
// source/core/slang-string.h:814-817
ImmutableHashedString(const UnownedStringSlice& slice)
    : slice(slice), hashCode(slice.getHashCode())
{
}
```

`slice(slice)` invokes `String(UnownedStringSlice const&)`, which calls
`StringRepresentation::create` — `operator new` plus `memcpy` of the whole
mangled name. Mangled names are long: `_S12MyModule4FooC...` style symbols run to
tens or hundreds of bytes.

`maybeCloneValue` is called for every global value the linker considers.

### The multi-probe lookup

```cpp
// source/slang/slang-ir-link.cpp:119-134
IRSpecSymbol* findSymbols(UnownedStringSlice mangledName)
{
    ImmutableHashedString hashedName(mangledName);        // allocation + hash
    RefPtr<IRSpecSymbol> symbol;
    if (shared->symbols.tryGetValue(hashedName, symbol))  // probe 1
        return symbol;
    for (auto m : irModules)
    {
        for (auto inst : m->findSymbolByMangledName(hashedName))
            insertGlobalValueSymbol(shared, inst);
    }
    if (shared->symbols.tryGetValue(hashedName, symbol))  // probe 2
        return symbol;
    shared->symbols[hashedName] = nullptr;                // probe 3 + key copy
    return nullptr;
}
```

Here the `ImmutableHashedString` is at least built once and reused across the
three probes — better than `maybeCloneValue` — but probes 2 and 3 are redundant
with each other on the miss path: the map has not changed between them, and
probe 3's `operator[]` re-finds the same absent key it just failed to find.

Also note `shared->symbols[hashedName] = nullptr` copies the key, which copies
the `String` (a refcount bump, not an allocation — `String` is refcounted, so
this one is cheap).

### The explicit conversion

```cpp
// source/slang/slang-ir-link.cpp:1681
auto mangledName = String(linkage->getMangledName());
```

Here the allocation is at least visible in the source. It is used for both a
`tryGetValue` and an `add` on `sharedContext->symbols`
(`slang-ir-link.cpp:1681-1694`), so one of the two copies is genuinely needed —
the map must own the key — but the lookup half does not need it.

### Related maps on the same path

```cpp
// source/slang/slang-ir.h:2310
Dictionary<ImmutableHashedString, List<IRInst*>> m_mapMangledNameToGlobalInst;
```

reached via `IRModule::findSymbolByMangledName`, called in the loop above.

```cpp
// source/slang/slang-ir-link.cpp:115
HashSet<UnownedStringSlice> deferredWitnessTableEntryKeys;
```

This one is keyed by a _non-owning_ slice — a different, deliberate trade-off
(no allocation, but a lifetime coupling to whatever owns the bytes). Worth
noting as precedent that the codebase is willing to do this where the lifetime
is clear.

---

## Why this is independent of the selected hash function and map

The allocation happens in the implicit conversion, before the map is entered.
The redundant probes are call-site structure. Neither depends on `SLANG_HASH` or
`SLANG_HASHMAP`.

---

## Proposed change

### Step 1 — transparent lookup for `ImmutableHashedString` keys

Give `Dictionary<ImmutableHashedString, V>` a hash/equality pair that can hash
and compare an `UnownedStringSlice` directly, following the pattern already used
for `ValKey` (`source/slang/slang-ast-builder.h:186-210`):

```cpp
struct MangledNameHash
{
    using is_transparent = void;
    HashCode64 operator()(const ImmutableHashedString& s) const { return s.getHashCode(); }
    HashCode64 operator()(const UnownedStringSlice& s) const { return s.getHashCode(); }
};

struct MangledNameEqual
{
    using is_transparent = void;
    bool operator()(const ImmutableHashedString& a, const ImmutableHashedString& b) const { return a == b; }
    bool operator()(const UnownedStringSlice& a, const ImmutableHashedString& b) const { return b == a; }
    bool operator()(const ImmutableHashedString& a, const UnownedStringSlice& b) const { return a == b; }
};
```

Both argument orders are required — ankerl compares `equal(probe, stored)` while
absl and tsl compare `equal(stored, probe)`. This is documented on `ValKeyEqual`
at `slang-ast-builder.h:193-201` and supplying only one order silently restricts
which `SLANG_HASHMAP` backends compile.

The correctness property: `ImmutableHashedString`'s cached `hashCode` is
initialised as `slice.getHashCode()` (`slang-string.h:814-817`), so
`hash(ImmutableHashedString(s)) == hash(s)` holds by construction. Test it
anyway — including the empty-slice case, since `hashBytes` has a `len ? … :
empty` special case in the absl and std implementations
(`source/core/slang-hash-impl.h:112-129`).

Also note `ImmutableHashedString::operator==(const UnownedStringSlice&)` already
exists (`slang-string.h:849`) and compares only the text, which is the right
semantics for a heterogeneous probe.

If the general `Dictionary` default is made transparent by issue
[02](02-transparent-heterogeneous-lookup.md), this step reduces to "use the
defaults"; otherwise supply the functors explicitly at the three declaration
sites.

### Step 2 — make the constructor `explicit`

Once transparent lookup works, mark
`ImmutableHashedString(const UnownedStringSlice&)` `explicit` so that any
remaining accidental conversion becomes a compile error rather than a silent
allocation. This is the change that prevents the problem from coming back.

Expect fallout at the call sites that legitimately want the conversion
(`slang-ir-link.cpp:121`, the `add`/`operator[]` sites); those become explicit,
which is exactly right.

### Step 3 — collapse the redundant probes in `findSymbols`

On the miss path, probes 2 and 3 are the same lookup. With the
`find`/`tryEmplace` API from issue
[04](04-dictionary-find-iterator-api-and-double-lookups.md):

```cpp
IRSpecSymbol* findSymbols(UnownedStringSlice mangledName)
{
    if (auto found = shared->symbols.tryGetValue(mangledName))
        return *found;

    for (auto m : irModules)
        for (auto inst : m->findSymbolByMangledName(mangledName))
            insertGlobalValueSymbol(shared, inst);

    auto [it, inserted] = shared->symbols.tryEmplace(ImmutableHashedString(mangledName), nullptr);
    return it->second;
}
```

One probe before the module scan, one after — and the "after" probe doubles as
the negative-caching insert.

**Verify the negative caching semantics first:** `shared->symbols[hashedName] = nullptr`
caches a _failed_ lookup so the module scan is not repeated. The `tryEmplace`
form above preserves that only if `insertGlobalValueSymbol` never inserts a
`nullptr` entry that should be overwritten. Read `insertGlobalValueSymbol`
(`slang-ir-link.cpp:~1675-1695`) before changing this.

### Step 4 — the explicit `String(...)` at :1681

```cpp
auto mangledName = String(linkage->getMangledName());
...
if (sharedContext->symbols.tryGetValue(mangledName, prev))
    ...
else
    sharedContext->symbols.add(mangledName, sym);
```

Restructure so the owned copy is only made on the insert branch. With
`tryEmplace` this becomes a single probe that only materialises the key when
inserting — though note the key will still need constructing to _pass_ to
`tryEmplace`, so the win here is the collapsed probe rather than the allocation.
Lower priority than steps 1–3.

---

## Risks and things to watch

- The linker's symbol table drives which definition wins when multiple modules
  export the same mangled name (`sym->nextWithSameName` chaining at
  `slang-ir-link.cpp:1683-1694`). Any change to lookup or insert order can change
  which definition is selected. Test with multi-module compiles and with the
  core module plus user modules.
- Negative caching (step 3) is subtle; getting it wrong turns a linear scan into
  a quadratic one rather than producing wrong output, so it may not show up in
  correctness tests. Time a large multi-module link explicitly.
- `deferredWitnessTableEntryKeys` being a `HashSet<UnownedStringSlice>` means
  some mangled-name bytes are already borrowed. Do not "fix" that one to own its
  keys without checking why it borrows.

---

## Validation

1. Allocation probe or a counter in `StringRepresentation::createWithCapacityAndLength`:
   count allocations during `linkIR` on a compile that imports the core module,
   before and after.
2. Time `linkIR` specifically (it is a distinct phase) on a large multi-module
   program.
3. Build under every `SLANG_HASHMAP` value — the two-argument-order requirement
   for the transparent comparator is a per-backend compile issue.
4. Full `sti`, with attention to module-import and separate-compilation tests.

---

## Related

- [02](02-transparent-heterogeneous-lookup.md) — the general form.
- [04](04-dictionary-find-iterator-api-and-double-lookups.md) — the
  `find`/`tryEmplace` needed for step 3.
- [06](06-string-hash-not-cached.md) — `ImmutableHashedString` is the existing
  workaround for uncached `String` hashing.
