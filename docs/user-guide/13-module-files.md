---
layout: user-guide
permalink: /user-guide/module-files
---

Writing Module Files, Import, and Include
=========================================

This chapter is for authors of Slang modules and source packages. It states how to name files, how to write `import` and `__include`, and how those constructs are resolved. The compiler rules are the same whether or not you use `slang package`; the package layout is the conventional way to keep those rules from colliding across dependencies.

The language model for modules, `__include` versus `#include`, and `public` / `internal` / `private` is in [Modules and Access Control](modules). Fetching and validating packages is in [Slang Source Packages](source-packages).

## A running example

The rest of this chapter uses one small library: a value-noise helper published under the namespace directory `acme`. The package export root is `src/`:

```text
src/
  acme/
    noise.slang
    noise/
      hash.slang
      fade.slang
```

Consumers will write `import acme.noise;`. That import path is the path of the primary file under `src/`, without the `.slang` suffix. The declared module name inside the file is only `noise`, not `acme.noise`. Namespace directories participate in lookup; they are not part of the `module` / `implementing` declaration.

```slang
// src/acme/noise.slang

module noise;

__include "noise/hash";
__include "noise/fade";

public float valueNoise(float2 p)
{
    let i = floor(p);
    let f = fade(frac(p));
    let a = hash2(i);
    let b = hash2(i + float2(1.0, 0.0));
    let c = hash2(i + float2(0.0, 1.0));
    let d = hash2(i + float2(1.0, 1.0));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}
```

```slang
// src/acme/noise/hash.slang

implementing noise;

internal float hash2(float2 p)
{
    return frac(sin(dot(p, float2(127.1, 311.7))) * 43758.5453);
}
```

```slang
// src/acme/noise/fade.slang

implementing noise;

internal float2 fade(float2 t)
{
    return t * t * (3.0 - 2.0 * t);
}
```

A shader in another package or in an application imports only the primary:

```slang
// app/tonemap.slang

import acme.noise;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint2 tid: SV_DispatchThreadID)
{
    float n = valueNoise(float2(tid) * 0.1);
    // ...
}
```

Compile that shader with the export root on the search path, for example `slangc app/tonemap.slang -I src ...`. After `slang package fetch`, pass each path listed in `.slang/search-paths` the same way.

## Primary files and import paths

Every module has exactly one primary file. In a package, every `.slang` file that is not under another module's companion directory is a primary.

| Role | Example | First declaration | How others refer to it |
| --- | --- | --- | --- |
| Primary | `src/acme/noise.slang` | `module noise;` | `import acme.noise;` |
| Companion | `src/acme/noise/hash.slang` | `implementing noise;` | `__include "noise/hash";` from the primary (or another file already in this module) |

The filename of the primary (without extension) must match `NAME` in `module NAME;`. `slang package validate` enforces that, and also requires that the same import path is not exported by two packages in the lockfile.

You can only `import` a primary. `import hash;` against `hash.slang` is an error, because that file starts with `implementing` rather than `module`.

Identifier and string forms are equivalent once underscores in identifier form are translated to hyphens in the file name:

```slang
import acme.noise;
import "acme/noise";
import "acme/noise.slang";
```

All three look for `acme/noise.slang` (and, as described below, `acme/noise.slang-module`).

## How `import` is resolved

`import` uses the same search as a preprocessor `#include`:

1. The directory of the file that contains the `import`.
2. Each `-I` / search directory, in order.

The compiler then tries candidate **file names** derived from the import, not a special “package root” besides those search directories. For `import acme.noise` it looks for `acme/noise.slang-module` and `acme/noise.slang` (plus a dash-for-underscore spelling and `.slang.md` variants). Outside the language server it tries a `.slang-module` **before** `.slang` at every search step. The first file that loads wins. Later `import acme.noise` in the same session reuse that loaded module; they do not search again.

Search directory order does **not** mean “prefer source from this `-I` over a binary from a later `-I`.” A `.slang-module` found earlier in the search beats a `.slang` found later.

Put **export roots** on the search path (`src/` in the example), not companion directories such as `src/acme/noise/`. Companions are not import targets. Adding those directories to `-I` only makes short include names more likely to hit the wrong file.

A `.slang-module` does not have to sit next to its `.slang`. It must appear at the **same import-relative path** on some search directory: `acme/noise.slang-module` for `import acme.noise`. Implementation `.slang` files are not required on the search path when the consumer loads the binary.

## Companion files

Additional files belong to `noise` only if they are discovered by a chain of `__include` starting at the primary. A dangling `implementing noise;` file that is never included is not compiled.

In a package, those files live in a directory named after the primary, next to it: `src/acme/noise/` for `src/acme/noise.slang`. Each of them must begin with `implementing noise;` — the simple declared name, matching the primary file stem, not `acme.noise`. `slang package validate` requires that spelling.

Write `__include` so the path is resolved **relative to the file that contains the include**, and so it names the companion subdirectory. From `src/acme/noise.slang`:

```slang
__include "noise/hash";
__include "noise/fade";
```

Those paths are `src/acme/noise/hash.slang` and `src/acme/noise/fade.slang`. Lookup succeeds in the primary's directory and does not consult other packages' search roots.

`hash.slang` and `fade.slang` do not need to `__include` each other. Once the primary has pulled both in, they share the module's declarations. `fade` may call `hash2` even though `fade.slang` never mentions `hash.slang`. Mark helpers `internal` so `import acme.noise` does not expose them.

## How include is resolved

`__include` searches like `#include`: first next to the **including** file, then `-I`. It does **not** automatically look inside a directory named after the module.

Consider this mistake in `src/acme/noise.slang`:

```slang
__include hash;          // looks for src/acme/hash.slang, not src/acme/noise/hash.slang
```

If that sibling does not exist, search continues through every `-I`. Another dependency that happens to contain a `hash.slang` can be pulled in. The `implementing` check only compares the **simple** module name. A file that says `implementing noise;` in a different package can pass that check even though it is not part of `acme.noise`.

Prefer the quoted, directory-qualified form shown in the example. Identifier form `__include noise.hash` is equivalent to `"noise/hash"` after underscore-to-hyphen translation.

Do not `__include` a primary (`module` file). That is an error; the user meant `import`.

Prefer `__include` over preprocessor `#include` for module fragments. `#include` shares preprocessor state, is not limited to `implementing` files, and has no module-name check, so a shared basename on `-I` is enough to splice the wrong text into a translation unit.

## Rules for module and package authors

1. One primary per module. Place it at the import path you want consumers to write, under an export listed in `slang-package.json`.
2. Declare `module` / `implementing` with the primary's file stem (`noise`), not the namespace prefix (`acme`).
3. Put implementation files under `primaryName/` next to the primary, and `__include` them with that prefix from the including file.
4. Do not treat companion files as importable modules. Do not add companion directories as extra `-I` entries.
5. Keep import paths unique across the packages a project resolves together (`slang package validate` checks this for primaries).
6. Hide implementation APIs with `internal` (or `private`). File layout does not hide symbols; access control does.
7. When shipping or consuming precompiled modules, keep the import-relative path (`acme/noise.slang-module`). You do not need implementation sources on the consumer search path.

## What consumers write

From another module in the same package, or from application code, import the primary only:

```slang
import acme.noise;
```

If that other module is itself a package primary, for example `src/acme/tonemap.slang` with `module tonemap;`, the same `import acme.noise;` is resolved from `src/` on the search path, not by reaching into `noise/`.

Do not `__include` files from a module you do not own. `__include` means “this file is part of **my** module.” Crossing a package boundary that way is how short include names pick up the wrong implementation file. Depend on the other module with `import` and its `public` API.
