# Claim-driven test methodology

How to turn a source doc into a complete test bundle. This is the
shared methodology referenced by [`CAMPAIGN.md`](../CAMPAIGN.md) and
the per-bundle `_prompt.md` files. It is **tree-agnostic**: it applies
identically whether the bundle's `source_doc` is a `docs/language-reference/`
file (the `conformance/` tree) or a `docs/generated/design/` file (the
`design/` tree). The only difference is the source-of-truth weight
of the doc — see [`_common.md` § Source-of-truth hierarchy](_common.md).

The campaign is **claim-driven, not count-driven**. A doc is decomposed
into a flat list of documented claims, and the bundle is complete when
every claim is covered by at least one test along the dimensions the
doc commits to (basic cases, corner cases, boundary values, main
feature combinations, different types, and meaningful back-ends), or is
listed as untested with a classified reason. Test count is an output of
that process, never a target.

## 1. Enumerate every claim in the doc

This is the load-bearing step. Before writing a single test, produce a
flat numbered list of every normative, testable statement the doc makes.
A _claim_ is one independently verifiable assertion the doc commits to —
a grammar production, a table row, a "must" / "shall" / "is rejected"
sentence, a worked example, an explicit carve-out.

Style: each claim is one sentence. Use the doc's own wording where
possible. Number them. Group by sub-area (the doc's section headings).
Examples of what counts as one claim:

- "`0xFFFFFFFF` (no suffix, hex base) has type `uint`."
- "Prefix `++` yields the new value; postfix `++` yields the old value."
  _(That's two claims — the prefix yield and the postfix yield. They
  could be tested by one file but they are separate claims.)_
- "`UINT_MAX + 1` wraps to `0` (every binary arithmetic operator on
  every documented unsigned integer type wraps on overflow)." That is
  one claim, but it multiplies along
  `{int, uint, int64_t, uint64_t}` × each operator (`+`, `-`, `*`,
  `/`, …) — one claim, many tests.

The enumerated list is checked into the bundle as the **Claims** section
in the bundle README (see § 4). It is the artefact reviewers and the
next campaign session read to decide whether the bundle is complete.
**Claim count is the bundle's target, not test count.**

## 2. Map claims to tests along the documented dimensions

Each claim must end up either in `## Functional coverage` (covered) or
`## Untested claims` (with a reason). The bundle is done when every
enumerated claim appears in one of those two tables.

For each claim, walk these dimensions and write a test for every one the
doc actually commits to (skip dimensions the doc doesn't cover — don't
invent claims):

- **Basic case.** The straight-line "this is what the doc literally
  says" test. Always present.
- **Corner cases.** Documented edge conditions: empty input,
  zero-length array, single-element vector, default-initialised value,
  ambiguous-but-resolvable overload, etc.
- **Boundary values.** Numeric edges the type or grammar names: `MIN` /
  `MAX` / zero / `MIN-1` / `MAX+1` / `0xFFFFFFFF` / `NaN` / `±Inf` /
  empty string / capacity-1.
- **Main feature combinations.** Documented compositions: the claim
  under struct membership, inside a generic, behind an `extension`,
  under `enum` carve-outs, inside a `[ForceInline]` callee — whichever
  combinations the doc itself names.
- **Different types.** Every type the doc says the claim applies to:
  every documented numeric type (`int` / `uint` / `int64_t` /
  `uint64_t` / `float` / `double` / `half`), every documented shape
  (scalar / vector / matrix), every documented qualifier (`const` /
  `static` / `uniform` / `in` / `inout` / `out`).
- **Meaningful back-ends.** Classify the claim as target-independent or
  target-dependent, then cover accordingly:
  - _target-independent_ (value/semantics resolved before codegen):
    `INTERPRET` (slangi) and/or `COMPARE_COMPUTE -cpu` — one or two
    directives, no per-target fan-out.
  - _target-dependent_ (emitted code / legalization / capability): a
    `SIMPLE -target <T>` emission directive for **every feasible
    text-emit target** the claim is observable on — `hlsl`, `glsl`,
    `spirv-asm`, `metal`, `wgsl`, `cuda`, `cpp` — not just the ones the
    doc names, and not just HLSL+SPIR-V. Pair with the functional check.

  This is the mandatory fan-out; see
  [`_common.md` § Exercise every feasible back-end](_common.md). Targets
  that can't express the claim go in `## Untested claims`
  (`unsupported-on-target`), never a weakened CHECK.
- **Construct & legalization variants (depth).** When the doc describes a
  transform or emission whose handling *branches on* the input's type,
  shape, or form — a type-mapping / layout table, an opcode or decoration
  list, per-element-type array strides, per-op atomics, per-address-space
  storage classes — instantiate **one test per row / branch the doc
  enumerates**, not one test for the whole table. This is where emission
  and legalization *depth* lives: a single `StructuredBuffer<int>` stride
  test does not exercise the `int8`/`int64`/struct/`float4[3]` stride
  branches the layout rules describe, nor does one `InterlockedAdd` cover
  the and/or/xor/min/max/exchange/compare-exchange forms. Re-read the
  doc's tables and "for each …" rules as **enumerations to expand**, not
  prose to summarize. Stay **branch-driven, not Cartesian** — cover only
  the variants the doc and the target pipeline actually treat differently.

One claim therefore typically yields several test files. The claim row
in `## Functional coverage` lists them all. A claim with only a single
basic-case test is acceptable only when the doc itself commits to no
edges, types, combinations, or back-end-specific observations.

**Cross-bundle reference**: when the claim is fully exercised in a
sibling bundle, record it in `## Untested claims` with reason
`out-of-bundle` and a link to the covering tests.

Don't pad. Don't shortcut. The size of the bundle follows from the size
of the doc — a thin doc yields a thin bundle and that's the right
answer. A rich doc yields a rich bundle.

### Systematic variant expansion when the doc names a family but doesn't enumerate it

The design docs frequently state a rule *generically* — "atomic intrinsics
lower to the corresponding target op", "structured-buffer layout follows
the rules below", "narrow integer types are emitted per the target's width
support" — without spelling out the individual members or cells the
compiler branches on. You may instantiate the individual cells **even when
the immediate doc section does not list them**, under all of the following
conditions. This is *not* a license to write tests against observed
uncovered code (see [`_expand.md` § The hard rule](_expand.md)); it is a
disciplined way to read an authoritative source for the variants it
*implies*.

1. **The variant axis is real, confirmed by an authoritative surface — not
   guessed.** Enumerate the candidate cells from one of:
   - the **language reference** type / intrinsic / operator tables (the
     authoritative spec; this is also the citation when it names the
     family), or
   - the **core-module declarations** (`*.meta.slang`) as ground truth for
     which intrinsic overloads / operators actually exist. The core module
     is compiler source, so it is used **only to enumerate the candidate
     cells — never as the test's `doc_ref`** (see
     [`_common.md` § Source-of-truth hierarchy](_common.md)).

   If neither surface confirms the cell exists, you are guessing — stop and
   record a doc gap instead.

2. **Anchor to the doc section that names the family**, by the normal
   source-of-truth hierarchy: the language reference if it covers the
   family, otherwise the design-doc fallback. Add a `## Doc gaps observed`
   row noting the doc names the family generically but does not enumerate
   the cells (Kind `missing-example` or `undocumented-behavior`; Suggested
   addition: "enumerate the &lt;atomic-op / stride / width&gt; variant
   table"). Surface-grounded expansion and the doc-gap channel run
   together — do both, not one or the other.

3. **Establish each cell's behaviour by running the compiler — never by
   assuming it works.** For each candidate cell, compile it and classify
   the *actual* result:
   - **clean, expected-shape output** → keep the test; pin the `CHECK` to
     the **real emitted text** you observed.
   - **clean rejection / diagnostic** → keep it as a **negative test**
     anchored to the capability or type rule (`DIAGNOSTIC_TEST`, exact
     `E####`).
   - **abort / crash / internal error / malformed output** → this is a
     **finding (compiler bug), not a test** (see
     [`_common.md` § Reporting suspected compiler bugs](_common.md)). Do
     not write a passing test with a loosened `CHECK` that hides it.

   "Assume it works" is the one thing this rule forbids: the WGSL `double`
   and `int8` aborts were found precisely because cells were run and
   classified rather than assumed green.

4. **Prune cells that collapse — branch-driven, not Cartesian.** The
   surface gives you the *candidate* grid; keep only the cells the pipeline
   treats distinctly:
   - keep cells the surface itself distinguishes (signed vs. unsigned
     `Min`/`Max` are different ops; `device` vs. `threadgroup` atomics are
     different address spaces);
   - when you cannot tell two cells apart from the spec, compile both and
     **diff the emitted text** (observing output is always allowed; reading
     *coverage* is not). Identical emission ⇒ keep one representative and
     note the rest in the claim's `Tests` cell as "same emission".

5. **`CHECK` tightness.** Pin the token that *identifies the cell* — the
   specific op (`OpAtomicAnd`), decoration (`ArrayStride 2`), or layout
   token — tightly, and leave incidental names / IDs / registers wild
   (`%{{[0-9]+}}`, `a_{{[0-9]+}}`). A cell whose distinguishing token you
   cannot name is a cell you have not differentiated — fold it back into
   the collapse step above.

Coverage still never tells you *which cells to add* — only **which bundles
are worth this effort**. The candidate grid comes from the surface; the
keep/drop decision comes from observed compiler output.

## 3. Functional + emission pairs are the default

For any claim that the doc says is observable in emitted text on a
particular target, the bundle gets **two** test files:

- a **functional** test — `//TEST:INTERPRET(filecheck=CHECK):` or
  `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu` — that observes
  the runtime value or behaviour via `printf` / output buffer.
- an **emission** test — `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`
  (and / or `-target spirv-asm` / `-target glsl` / `-target cuda`) —
  that pins the user-visible emitted text for the same claim.

Naming convention: `<topic>-<sub-area>-functional.slang` and
`<topic>-<sub-area>-emission.slang`. Cross-target emission can either
live in one file with multiple `//TEST` directives or be split
file-per-target when the emit pattern differs enough that CHECK lines
would collide.

## 4. The bundle README carries the canonical claim enumeration

The bundle README is the artefact reviewers and the next campaign
session read to decide whether the bundle is complete. It has three
claim-bearing sections:

- `## Claims` — flat numbered list of every claim extracted in § 1,
  grouped by sub-area / doc heading. This is the ground-truth
  enumeration. Every claim ID here must appear in one of the two tables
  below.
- `## Functional coverage` — one row per claim that has at least one
  test. Columns: claim ID + claim sentence, dimensions covered
  (`basic` / `corner` / `boundary` / `combinations` / `types` /
  `backends`), doc anchor, test files.
- `## Untested claims` — one row per claim that has no test in this
  bundle, with a reason (`out-of-bundle` / `compiler-bug-pending` /
  `non-normative` / `unclassified`).

The Claim cell names the **specific** surface, not the doc's top-level
concept. Reviewers should be able to read the table and know what every
test is for without opening the `.slang` file. Example tone (excerpt
from one bundle's table):

> | C7 | `WaveActiveSum` after a divergent `if/else` reconverges and sums across the originally-divergent threads. | basic, backends | [#wave-divergence](...) | wave-divergent-if-functional.slang, wave-divergent-if-emission.slang |
> | C8 | Same operator on a divergent `switch` with `[unroll]` does **not** reconverge. | corner | [#wave-divergence](...) | wave-divergent-switch-no-reconverge.slang |

Lead with the **observation** (what the test asserts) and pin the
diagnostic code (E####) or specific intrinsic / SPIR-V op when the claim
hangs on one. Vague rows like "vector operators work" are a sign of
claim under-decomposition (one row is being asked to carry many distinct
claims).

## 5. Test patterns

One `.slang` file per (claim × dimension) combination, with a `//META`
block citing the most specific source-doc anchor that covers the claim.
Use the established patterns (full detail in [`_common.md`](_common.md)):

- **Function-param to defeat the constant folder** for tests that
  observe optimization-pass behaviour or runtime arithmetic.
- **Overload-probe** for tests that observe the _type_ of a literal or
  expression (`int probe(int) { return 1; } int probe(uint) { return 2; }`).
- **`//TEST:INTERPRET(filecheck=CHECK):`** for value-correctness
  observations through `slangi` + `printf`.
- **`//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu`** for
  dispatch-shape behaviour (thread IDs, group IDs, atomics, groupshared,
  wave ops) where INTERPRET cannot model the execution model.
- **`//TEST:SIMPLE(filecheck=CHECK):-target <backend>`** for
  emission-pinning tests. Pair with the corresponding functional test
  (§ 3).
- **`//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`** for "is rejected" claims,
  with caret-anchored `^^^^` annotations and an `E####` code.

## 6. Stopping criterion

A bundle is complete when **every claim enumerated in `## Claims` has
either**:

- at least one test in `## Functional coverage` covering the dimensions
  the doc commits to (basic / corner / boundary / combinations / types /
  back-ends — only those the doc names), or
- a row in `## Untested claims` with a classified reason
  (`out-of-bundle` / `compiler-bug-pending` / `non-normative` /
  `unclassified`).

Do not pad with extra tests for already-covered claims, and do not stop
early because a fixed test count has been reached — the test count is
whatever the claim list × dimensions yields. `unclassified` rows are a
reviewer-flag, not a closed bundle.
