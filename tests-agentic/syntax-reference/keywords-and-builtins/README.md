---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:00:00+00:00
source_commit: ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
watched_paths_digest: 46d545d00112e20974ad7c66196cd74864348c29baa96c46b13ae4a26b395634
source_doc: docs/llm-generated/syntax-reference/keywords-and-builtins.md
source_doc_digest: 149c069099bcdbf65d61cb7c0a8efad6ebfce473dad9ffb85251ba6cb96f4820
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for syntax-reference/keywords-and-builtins

## Intent

Tests verify the parser-surface behaviors described in
[`docs/llm-generated/syntax-reference/keywords-and-builtins.md`](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md):
which identifiers acquire special meaning as statement, decl,
modifier, expression, or core-module-supplied keywords, and how the
documented meaning surfaces when a small program is compiled. Each
test pins a single doc claim and uses the smallest valid program that
makes the claim observable. Most tests are observed through
`//TEST:INTERPRET`; target-dependent keywords (`groupshared`,
`discard`, `gl_*`/`SV_*`, GLSL/HLSL meta-module types) are observed
through text-emit `FileCheck`s on the relevant backends; one negative
test confirms that `let` immutability is enforced by the checker.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                       | Claim (one line)                                                                                                | Tests                                       |
| -------- | -------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| C-01     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `if`/`else` form the conditional statement and dispatch to the correct arm.                                     | `statement-if-else.slang`                   |
| C-02     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `for (init; cond; update) { body }` is the counted-loop statement.                                              | `statement-for-loop.slang`                  |
| C-03     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `while (c) { ... }` and `do { ... } while (c)` are head-tested and tail-tested loop statements respectively.    | `statement-while-do.slang`                  |
| C-04     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `break` exits the enclosing loop; `continue` skips to the next iteration.                                       | `statement-break-continue.slang`            |
| C-05     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `return <expr>` hands a value back to the caller and halts the function body.                                   | `statement-return.slang`                    |
| C-06     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `switch`/`case`/`default` form the multi-way dispatch statement.                                                | `statement-switch-case.slang`               |
| C-07     | [#statement-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#statement-keywords)                              | `discard` is a fragment-shader statement terminating the current invocation.                                    | `statement-discard-fragment.slang`          |
| C-08     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `typealias` binds a name to a type; the alias resolves to its underlying type for overload resolution.          | `decl-typealias.slang`                      |
| C-09     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `let` is an immutable binding; `var` is a mutable binding.                                                      | `decl-let-var.slang`                        |
| C-10     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | A `let`-bound name cannot be reassigned (checker rejects with an l-value diagnostic).                           | `decl-let-immutable-rejected.slang`         |
| C-11     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `func name(params) -> RetType { ... }` is the Slang-style function-decl form.                                   | `decl-func-arrow.slang`                     |
| C-12     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `interface` declares a protocol-style requirement set that other types conform to via `:`.                      | `decl-interface-conformance.slang`          |
| C-13     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `extension <Type> { ... }` adds members to an existing type.                                                    | `decl-extension-method.slang`               |
| C-14     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `namespace name { ... }` introduces a named scope; members are reached with `::` qualification.                 | `decl-namespace-scope.slang`                |
| C-15     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `enum Name { Case = N, ... }` declares an enumeration with explicit case values (dispatched via direct lookahead, not the syntax table). | `decl-enum-value.slang`                     |
| C-16     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `__init` is the compiler-internal spelling of the constructor decl; it is invoked when the type name is used as a callable. | `decl-init-constructor.slang`               |
| C-17     | [#decl-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#decl-keywords)                                        | `__subscript(args) -> T { get { ... } }` binds `v[args]` syntax to a user-defined accessor.                     | `decl-subscript-index.slang`                |
| C-18     | [#simple-modifiers](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers)                                  | `in`, `out`, `inout` are simple parameter-direction modifiers; an `out`/`inout` mutation is visible at the caller. | `modifier-in-out-inout.slang`               |
| C-19     | [#simple-modifiers](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers)                                  | `static const` at file scope is a compile-time-known read-only binding.                                         | `modifier-static-const.slang`               |
| C-20     | [#simple-modifiers](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers)                                  | `public` and `private` are visibility modifiers; a `private` member is reachable from another member of the same type. | `modifier-public-private.slang`             |
| C-21     | [#simple-modifiers](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#simple-modifiers)                                  | `groupshared` is an HLSL-flavored storage modifier; it lowers to `groupshared` in HLSL and to `shared` in GLSL. | `modifier-groupshared-multitarget.slang`    |
| C-22     | [#expression-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords)                            | `this` is an expression keyword denoting the current object inside a member function.                           | `expr-this-self-reference.slang`            |
| C-23     | [#expression-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords)                            | `true` and `false` produce boolean-typed values that select different branches of a conditional.                | `expr-true-false-bool.slang`                |
| C-24     | [#expression-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords)                            | `none` is the empty value of `Optional<T>`; `hasValue` discriminates the empty case from the present case.      | `expr-none-optional.slang`                  |
| C-25     | [#expression-keywords](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#expression-keywords)                            | `sizeof(T)` is a compile-time-known size, in bytes, of `T`.                                                     | `expr-sizeof-int.slang`                     |
| C-26     | [#core-module-supplied-vocabulary](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary)    | `int`, `uint`, `float`, `bool` are core-module type names distinguishable by overload resolution.               | `builtin-numeric-types-overload.slang`      |
| C-27     | [#core-module-supplied-vocabulary](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary)    | `vec3` is supplied by `glsl.meta.slang` and is available in GLSL emit.                                          | `builtin-glsl-vec3.slang`                   |
| C-28     | [#core-module-supplied-vocabulary](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary)    | `StructuredBuffer<T>` is supplied by `hlsl.meta.slang` and survives into the emitted HLSL text.                 | `builtin-hlsl-structured-buffer.slang`      |
| C-29     | [#reserved-identifier-prefixes](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes)          | `SV_`-prefixed identifiers are HLSL system-value semantics surviving into HLSL emit.                            | `reserved-sv-prefix-hlsl.slang`             |
| C-30     | [#reserved-identifier-prefixes](../../../docs/llm-generated/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes)          | `gl_`-prefixed identifiers stand for GLSL-stage built-ins in the GLSL emit.                                     | `reserved-gl-prefix-glsl.slang`             |

## Tests in this bundle

| File                                       | Intent     | Doc anchor                          |
| ------------------------------------------ | ---------- | ----------------------------------- |
| `statement-if-else.slang`                  | functional | `#statement-keywords`               |
| `statement-for-loop.slang`                 | functional | `#statement-keywords`               |
| `statement-while-do.slang`                 | functional | `#statement-keywords`               |
| `statement-break-continue.slang`           | functional | `#statement-keywords`               |
| `statement-return.slang`                   | functional | `#statement-keywords`               |
| `statement-switch-case.slang`              | functional | `#statement-keywords`               |
| `statement-discard-fragment.slang`         | functional | `#statement-keywords`               |
| `decl-typealias.slang`                     | functional | `#decl-keywords`                    |
| `decl-let-var.slang`                       | functional | `#decl-keywords`                    |
| `decl-let-immutable-rejected.slang`        | negative   | `#decl-keywords`                    |
| `decl-func-arrow.slang`                    | functional | `#decl-keywords`                    |
| `decl-interface-conformance.slang`         | functional | `#decl-keywords`                    |
| `decl-extension-method.slang`              | functional | `#decl-keywords`                    |
| `decl-namespace-scope.slang`               | functional | `#decl-keywords`                    |
| `decl-enum-value.slang`                    | functional | `#decl-keywords`                    |
| `decl-init-constructor.slang`              | functional | `#decl-keywords`                    |
| `decl-subscript-index.slang`               | functional | `#decl-keywords`                    |
| `modifier-in-out-inout.slang`              | functional | `#simple-modifiers`                 |
| `modifier-static-const.slang`              | functional | `#simple-modifiers`                 |
| `modifier-public-private.slang`            | functional | `#simple-modifiers`                 |
| `modifier-groupshared-multitarget.slang`   | functional | `#simple-modifiers`                 |
| `expr-this-self-reference.slang`           | functional | `#expression-keywords`              |
| `expr-true-false-bool.slang`               | functional | `#expression-keywords`              |
| `expr-none-optional.slang`                 | functional | `#expression-keywords`              |
| `expr-sizeof-int.slang`                    | functional | `#expression-keywords`              |
| `builtin-numeric-types-overload.slang`     | functional | `#core-module-supplied-vocabulary`  |
| `builtin-glsl-vec3.slang`                  | functional | `#core-module-supplied-vocabulary`  |
| `builtin-hlsl-structured-buffer.slang`     | functional | `#core-module-supplied-vocabulary`  |
| `reserved-sv-prefix-hlsl.slang`            | functional | `#reserved-identifier-prefixes`     |
| `reserved-gl-prefix-glsl.slang`            | functional | `#reserved-identifier-prefixes`     |
| `boundary-sizeof-double.slang`              | boundary   | `#expression-keywords`              |
| `boundary-sizeof-bool-byte.slang`           | boundary   | `#expression-keywords`              |
| `boundary-alignof-int.slang`                | boundary   | `#expression-keywords`              |
| `boundary-countof-variadic-pack.slang`      | boundary   | `#expression-keywords`              |
| `boundary-nullptr-pointer-literal.slang`    | boundary   | `#expression-keywords`              |
| `boundary-optional-float-none.slang`        | boundary   | `#expression-keywords`              |
| `boundary-int8-min.slang`                   | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-uint8-max.slang`                  | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-int64-max.slang`                  | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-half-overload.slang`              | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-float4-swizzle-reverse.slang`     | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-float4x4-mul-vector.slang`        | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-rwstructuredbuffer-index-zero-hlsl.slang` | boundary | `#core-module-supplied-vocabulary` |
| `boundary-register-u0-binding-hlsl.slang`   | boundary   | `#core-module-supplied-vocabulary`  |
| `boundary-enum-int-min-max.slang`           | boundary   | `#decl-keywords`                    |
| `boundary-typedef-c-style.slang`            | boundary   | `#decl-keywords`                    |
| `boundary-using-namespace.slang`            | boundary   | `#decl-keywords`                    |
| `boundary-defer-statement-order.slang`      | boundary   | `#statement-keywords`               |
| `negative-rwstructuredbuffer-missing-args.slang` | negative | `#core-module-supplied-vocabulary` |
| `negative-int-as-identifier-in-expr.slang`  | negative   | `#core-module-supplied-vocabulary`  |
| `negative-let-through-inout-argument.slang` | negative   | `#decl-keywords`                    |
| `stress-deeply-nested-namespace-scope.slang` | stress    | `#decl-keywords`                    |
| `stress-twelve-numeric-overloads.slang`     | stress     | `#core-module-supplied-vocabulary`  |
| `stress-typealias-eight-deep-chain.slang`   | stress     | `#decl-keywords`                    |

## Doc gaps observed

- The `#complex-modifiers-take-arguments` table lists `layout`,
  `volatile`, `coherent`, `restrict`, etc. as taking arguments, but
  does not give a complete syntactic shape (what the argument list
  looks like, which contexts accept the modifier). No test in this
  bundle anchors there because the user-facing syntax is not described
  precisely enough to derive a verifiable claim. Filing as a doc gap
  would help.
- `#expression-keywords` lists `try`, `no_diff`, `__fwd_diff`,
  `fwd_diff`, `__bwd_diff`, `bwd_diff`, `new`, and the shape/pack
  utility expressions (`__first`, `__last`, `__trimFirst`, ...) but
  does not document a minimum standalone use that would let us
  exercise them through `slangi`. Most need accompanying imports or
  generic-resolution machinery that would not fit in a single test
  file. Recorded as candidates for expansion once the doc spells out
  a self-contained example for each.
- `#decl-keywords` says `struct`, `class`, and `enum` are not
  registered through `g_parseSyntaxEntries[]`. The text mentions
  `parseClassDecl` for `class` but no user-facing claim describes how
  `class` differs from `struct` for the user. Without a documented
  behavioral difference, no test for `class` was added beyond the
  general decl-keyword family covered by `decl-enum-value.slang` and
  `decl-init-constructor.slang`.
- `row_major` / `column_major` are listed as simple modifiers but
  their effect on HLSL emit is hidden by Slang's natural-layout
  rewriting, so the post-emit text does not contain the
  `row_major` token in a self-contained `cbuffer` test. Without a
  documented user-observable claim distinct from the emit text, no
  test was added.
- `import` is listed as a decl keyword but importing a real module
  (`diff`) from the agentic suite would require the matching
  `.slang` module to resolve in `slangi`'s search path. The
  no-GPU runner cannot reliably stage modules outside the test
  directory, so this claim was not exercised here.
- `countof(arr)` on a plain fixed-size local array under `slangi`
  returned `4` (the size of `int` in bytes) rather than the array
  element count, regardless of the declared size. The variadic-pack
  form (`countof(D)` for `let each D : int`) returns the documented
  element count, so the `boundary-countof-variadic-pack.slang` probe
  uses the pack form. Worth a note in the doc that the array form is
  only constant-folded in some contexts, or that the canonical
  `countof` shape is the pack form.
- The keyword-vs-identifier rules are not spelled out: `let`, `var`,
  and even `int` can be reused as identifier names in
  declarator position (e.g. `int let = 5;` is accepted), but `int`
  cannot appear at the head of an expression-typed declaration
  (`int x = int + 1;` is rejected with `E30060`). A short paragraph
  on which contexts re-classify an identifier as a value vs a type
  would let us write more precise boundary tests.
- `SV_`-prefixed and `gl_`-prefixed names are documented as
  "reserved", but in practice a user-named local `int SV_userVar`
  inside a function compiles and produces the printed value. The
  prefix policy is therefore advisory rather than enforced; a note
  to that effect in the doc would justify removing speculative
  user-prefix boundary tests.

## Out of scope (no-GPU runner)

- `[shader("raygeneration")]` / `hitAttributeEXT` raytracing modifiers
  cannot be exercised without GPU staging.
- `dyn` / dynamic dispatch through `IFunc` requires runtime
  reflection support that does not surface in the no-GPU CI image.
