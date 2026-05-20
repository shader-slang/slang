# Prompt: tests-agentic/syntax-reference/keywords-and-builtins/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at
`tests-agentic/syntax-reference/keywords-and-builtins/`, anchored to
[`docs/llm-generated/syntax-reference/keywords-and-builtins.md`](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md).

Audience: nightly CI. The bundle exercises the **parser-surface
behavior of Slang's keywords and core-module vocabulary** — which
identifiers acquire special meaning, and how that meaning surfaces
when a small piece of user code is compiled. The doc partitions the
vocabulary into:

- statement keywords (`if`, `for`, `while`, `do`, `break`, `continue`,
  `return`, `switch`/`case`/`default`, `discard`, ...);
- decl keywords (`typedef`, `typealias`, `let`, `var`, `func`,
  `interface`, `extension`, `namespace`, `__init`, `__subscript`,
  `property`, `enum`, `struct`, `class`, `cbuffer`, `__generic`,
  `import`, `module`, ...);
- modifier keywords, both simple (`in`, `out`, `inout`, `static`,
  `const`, `uniform`, `groupshared`, `public`, `private`,
  `row_major`, ...) and complex (`layout`, `volatile`, `__intrinsic_op`,
  ...);
- expression keywords (`this`, `true`, `false`, `none`, `nullptr`,
  `sizeof`, `alignof`, `try`, `no_diff`, `fwd_diff`, ...);
- the **core-module-supplied vocabulary** that behaves like keywords
  from a user's point of view (`int`, `uint`, `float`, `bool`, `vec3`,
  `mat4`, `StructuredBuffer`, `RWTexture2D`, `mul`, ...);
- reserved identifier prefixes (`__`, `gl_`, `SV_`).

The non-obvious doc claim is that almost none of these are recognized
by the lexer — they arrive as ordinary `Identifier` tokens and become
keywords because of:

1. direct identifier comparison in `parseStmt` / `parseDecl`;
2. a `SyntaxParseInfo` entry in
   `g_parseSyntaxEntries[]` (registered through `_makeParseDecl`,
   `_makeParseModifier`, `_makeParseExpr`);
3. a built-in declaration in the meta-modules
   (`core.meta.slang`, `hlsl.meta.slang`, `glsl.meta.slang`,
   `diff.meta.slang`).

A test in this bundle anchors to a documented keyword and verifies
that **the identifier is in fact bound to its documented role**.

## Required structure

1. `BUNDLE.md` with the structure named in `_common.md`.
2. One test file per verifiable claim. A claim is "verifiable" via
   `slangc` / `slangi` if its consequence can be observed in a small,
   self-contained compile:
   - **statement keywords** — write the smallest valid program that
     reaches the corresponding parse path (`if (c) { ... } else { ... }`,
     `for (...) { break; }`, `do { ... } while (c)`, `switch / case /
     default`, etc.) and observe the runtime effect. Negative form:
     `//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):` to verify the
     keyword's misuse is rejected (e.g. `let x = ...; x = ...;` for
     immutability).
   - **decl keywords** — declare a `typealias`, `typedef`,
     `interface`, `extension`, `namespace`, `enum`, `__init`,
     `__subscript`, `property`, etc., and use the result; the test
     fails to compile if the keyword is dropped from the parser.
   - **modifier keywords** — apply the modifier to a parameter or
     declaration and observe its consequence. `in`/`out`/`inout` are
     cleanest by checking whether a callee mutation is visible at
     the caller. `groupshared`, `row_major`, etc. need to be observed
     in emitted text (multi-target FileCheck).
   - **expression keywords** — `this` in a member function, `true` /
     `false` selecting a conditional branch, `none` constructing an
     `Optional<T>`, `sizeof(int)` returning a compile-time constant.
   - **core-module-supplied vocabulary** — define
     `int kind(int)` / `int kind(uint)` / `int kind(float)` /
     `int kind(bool)` overloads and dispatch the right one by passing
     a literal of the right type. This is the cleanest mechanism in
     the suite for proving an identifier names the documented type:
     overload resolution against typed parameters fails noisily if
     the identifier resolves to something else.
   - **reserved prefixes** — verify that the documented spelling
     (`__init`, `gl_Position`, `SV_Position`) is accepted and behaves
     as documented when emitted to its target.

3. Each test is small, single-purpose, and named after the keyword or
   group it verifies. Use kebab-case prefixes that mirror the doc's
   sections:
   - `statement-if-else.slang`, `statement-switch-case.slang`,
     `statement-discard-fragment.slang`, ...
   - `decl-typealias.slang`, `decl-interface-conformance.slang`,
     `decl-init-constructor.slang`, ...
   - `modifier-in-out-inout.slang`, `modifier-groupshared-multitarget.slang`,
     ...
   - `expr-this-self-reference.slang`, `expr-none-optional.slang`,
     ...
   - `builtin-numeric-types-overload.slang`,
     `builtin-glsl-vec3.slang`, `builtin-hlsl-structured-buffer.slang`.
   - `reserved-sv-prefix-hlsl.slang`,
     `reserved-gl-prefix-glsl.slang`, ...

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`

Secondary (allowed citations; use sparingly and only when the primary
doc explicitly hands off):

- `docs/llm-generated/syntax-reference/tokens.md`
- `docs/llm-generated/syntax-reference/grammar.md`

If you would cite anything else, stop and record a doc-gap finding in
`BUNDLE.md`.

## Source files you may consult for _verification only_

You may look at these files to verify that a claim in the doc is
realizable (e.g. the exact identifier of a parse callback, the
spelling of a modifier as it appears in the parser's syntax table).
You may **not** mine them for behavioral claims that the doc does not
make.

- `source/slang/slang-parser.cpp`
- `source/slang/slang-syntax.h`
- `source/slang/slang-syntax.cpp`

## Test directives

Most keyword claims are best exercised on the interpreter or on a
text-emit target (the parser- and check-stage behavior we care about
is observable independent of which backend runs). Pick the directive
by the type of claim:

- `//TEST:INTERPRET(filecheck=CHECK):` — default. Use for any claim
  whose effect is observable by `printf`-ing a value the compiler
  computed.
- `//TEST:SIMPLE(filecheck=CHECK):-target <X> -entry main -stage <S>` —
  use when the keyword has no runtime effect at all and only manifests
  in the emitted text (`groupshared`, `cbuffer`, `discard`, the
  `gl_*` / `SV_*` prefixed names). Per `_common.md`'s **multi-backend
  rule**, if the keyword lowers differently per target (e.g.
  `groupshared` becomes `groupshared` in HLSL and `shared` in GLSL),
  add one `//TEST:SIMPLE` line per feasible text-emit target with
  distinct `CHECK_<TARGET>` blocks.
- `//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):` — use for "the
  keyword's misuse is rejected" claims (e.g. `let` immutability,
  `private` member of a `public` struct).
- `//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute` —
  use when the runtime side of the claim is unimplemented in the
  interpreter (`switch`/`case`/`default`, `enum`). The C++ emit is
  enough to confirm the parser accepts the syntax.

Do not use any GPU-only directive. Do not use `(int)x` C-style casts;
use `int(x)` constructor-style casts.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `syntax-reference/keywords-and-builtins.md` (or one of the
      listed secondary docs, used sparingly).
- [ ] Every doc section that names user-facing keywords has at least
      one test anchored to it: `#statement-keywords`,
      `#decl-keywords`, `#simple-modifiers`, `#expression-keywords`,
      `#core-module-supplied-vocabulary`,
      `#reserved-identifier-prefixes`.
- [ ] At least one test uses **overload resolution against typed
      parameters** to prove that `int`, `uint`, `float`, and `bool`
      are bound to the documented core-module types.
- [ ] At least one test covers a **target-dependent** keyword (e.g.
      `groupshared`) with a `//TEST:SIMPLE` directive per feasible
      text-emit target where the spelling differs (HLSL vs. GLSL).
- [ ] At least one **negative / diagnostic** test for a misuse that
      the keyword's parsing rules forbid (e.g. `let`-bound name
      cannot be reassigned).
- [ ] No test depends on a GPU.
- [ ] No test was written by inspecting an uncovered source line. If
      you find yourself thinking "this would cover the branch at
      `slang-parser.cpp:NNNN`", stop and re-read the doc.
