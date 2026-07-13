# PR 9069 — High-Level Clarity Candidates

Change under review: wrap the `get*LibraryCode()` region of
`source/slang-core-module/slang-embedded-core-module-source.cpp` in
`#pragma clang optimize off` / `on` (guarded by `SLANG_CLANG`) to cut Clang
compile time, with a justifying comment.

---

### C001: Comment says "this file" but the pragma only covers the region below it

- Status: Proposed
- Confidence: High
- Scope: Direct
- Category: comments
- Location: `source/slang-core-module/slang-embedded-core-module-source.cpp:329-334`
- GitHub link:

Context:

```cpp
#if SLANG_CLANG
// Clang takes around 10 minutes to compile this file with optimizations, with EarlyCSEPass taking
// ~95% of the execution time. Disabling optimizations here reduces the compilation time to seconds
// and has no noticeable impact on run-time performance.
#pragma clang optimize off
#endif

ComPtr<ISlangBlob> Session::getCoreLibraryCode()
```

Proposed comment:

> The comment says Clang takes ~10 minutes to compile "this file", but the
> `#pragma clang optimize off` is placed at line 329, so it only de-optimizes
> the four `get*LibraryCode()` functions below it — the type tables and
> `getBaseTypeConversionCost` / `getBaseTypeConversionOp` above this point are
> still optimized. That mismatch makes the intended scope of the workaround
> unclear: is de-optimizing only these four functions sufficient, and why?
> Since the cost appears to live in these functions specifically (they
> `#include` the large generated `*.meta.slang.h` bodies), it would help a
> future maintainer to say so — e.g. name the `#include`d generated sources as
> the reason the pragma is placed here rather than at the top of the file — so
> the placement reads as a deliberate, correct choice rather than an accident.

Notes:
The pragma affects only functions defined after it. Placing it just before
`getCoreLibraryCode()` narrows the effect to the four `get*LibraryCode()`
functions, each of which `#include`s a large generated header
(`core.meta.slang.h`, `hlsl.meta.slang.h`, `diff.meta.slang.h`,
`glsl.meta.slang.h`). This is very likely the intended and correct placement,
but the "this file" wording obscures the narrowing and the reason for it.

---

### C002: "no noticeable impact on run-time performance" is asserted without justification

- Status: Proposed
- Confidence: Medium
- Scope: Direct
- Category: invariant
- Location: `source/slang-core-module/slang-embedded-core-module-source.cpp:330-332`
- GitHub link:

Context:

```cpp
// Clang takes around 10 minutes to compile this file with optimizations, with EarlyCSEPass taking
// ~95% of the execution time. Disabling optimizations here reduces the compilation time to seconds
// and has no noticeable impact on run-time performance.
#pragma clang optimize off
```

Proposed comment:

> The claim that disabling optimization here has "no noticeable impact on
> run-time performance" is stated as a bare assertion. A reader cannot verify it
> from the comment alone. The reason it holds is worth stating: these
> `get*LibraryCode()` functions run once per session (each caches its result and
> returns early on subsequent calls) and do little beyond appending string
> literals into a `StringBuilder`, so their generated code is not
> performance-sensitive. Making that argument explicit would let a maintainer
> trust the claim instead of having to reconstruct it.

Notes:
Each function guards with `if (!xLibraryCode)` and stores the result, so it
executes its body at most once. The bodies are string concatenation of the
embedded core-module source. This supports the "no runtime impact" claim, but
the comment does not say why.
