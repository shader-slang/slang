---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:42:39Z
target_doc: ast-reference/values.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 6
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/values.md

## Summary

Nothing was escalated: every observation that could be checked agreed
with the watched source, and none of the six gaps was a
`drift-from-source` claim against the document. Five gaps were fixed by
editing four sections of the target document — the `### DeclRef family`
preamble, two `### IntVal family` table rows, `### Modifier values`, and
`### Hash-consing and the ASTBuilder` — each confirmed against
`source/slang/slang-ast-val.h`, `source/slang/slang-ast-val.cpp`,
`source/slang/slang-ast-base.h`, `source/slang/core.meta.slang`, or
`source/slang/hlsl.meta.slang`. One gap was deferred: the user-level
spelling of a variadic-pack count constraint is
`where countof(Pack) == N`, but the only site that establishes it is
`maybeParseGenericConstraints` in `source/slang/slang-parser.cpp`
(lines 1934-1981), which is not in this page's `watched_paths`, and no
bundle test pins the form.

Two of the gaps asked for material the watched paths only partly
support, and the edits deliberately stop where the evidence does.
`b226ec1bc278` proposed a per-target rendering table for
`ResourceFormatModifierVal`; the HLSL element respelling, the SPIR-V
`Rgba8` image-format operand, and the GLSL / Metal / CUDA erasure are
all decided in `source/slang/slang-emit-*.cpp`, which is not watched
here, so what was written is the rule the watched sources do state —
the modifier lives on the _type_, how much of it survives is a
per-target decision, and the modified element type is not
interchangeable with the bare one in target type checks — with a
pointer to `../pipeline/06-emit.md` for the emitted spellings.
`f6fb37ba186d`'s suggested wording ("only `GenericAppDeclRef` and
`LookupDeclRef` have user-visible consequences") was rephrased around
what the header actually shows, because `MemberDeclRef` does carry a
substitutable parent operand and the suggestion reads as if it does
not.

Operator follow-ups: (1) adding `source/slang/slang-parser.cpp` to this
document's `watched_paths` would unblock `f9503231dcd0` and would also
let the page cite the parser entry points that the AST-reference family
contract asks for; (2) adding `source/slang/slang-emit-hlsl.cpp`,
`slang-emit-glsl.cpp`, `slang-emit-spirv*.cpp` would let the
per-target half of `b226ec1bc278` be written here rather than deferred
to the emit page — though that material arguably belongs to
`../pipeline/06-emit.md` regardless.

## Actions

| Gap ID       | Action   | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | Fix summary                                                                                                                                                                                                                                                                                                                                               |
| ------------ | -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| f6fb37ba186d | fixed    | `source/slang/slang-ast-val.h:21` gives `DirectDeclRef` a single `Decl` operand (a `Decl` is not a `Val`, so there is nothing to substitute); `:32-35` carries the folding comment `MemberDeclRef(DirectDeclRef(A), B) ==> DirectDeclRef(B)`, making the Direct/Member choice a canonicalization of the access path; `:74-77` and `:106-128` show that only `LookupDeclRef` and `GenericAppDeclRef` add a `SubtypeWitness` and an argument list on top of that path. Bundle tests `genericappdeclref-different-args-distinct.slang` (E30019 on `Box<int>` vs `Box<float>`) and `lookupdeclref-associated-type-specializes.slang` (`T.Elem` resolving per conformer) pin the two user-visible halves.                                                                                                                                | added a paragraph before the `### DeclRef family` table stating that the four shapes record how a declaration was reached, that Direct and Member differ only in whether the path had to be written out and fold together when it is static, and that the argument list and the witness are what make two decl-refs to one `Decl` denote different things |
| ef872879ed48 | fixed    | `source/slang/slang-ast-val.cpp:2098-2104` — `TypeCastIntVal::_toTextOverride` renders the node as `Type(base)`, i.e. the conversion spelling itself; `:2106-2170` (`tryFoldImpl`) folds it by converting through the target `BaseType`; `source/slang/slang-ast-val.h:197` fixes the operand pair. Bundle test `typecastintval-uint-param-array-bound.slang` uses `int sumCast<let N : uint>(int a[int(N)])` and CHECKs `cast=6`.                                                                                                                                                                                                                                                                                                                                                                                                  | added the surface spelling to the `TypeCastIntVal` row — a conversion in a compile-time position, e.g. the bound `int[int(N)]` over a `let N : uint` parameter                                                                                                                                                                                            |
| f8c3987bbe41 | fixed    | `source/slang/slang-ast-val.cpp:3172-3177` — `WitnessLookupIntVal::_toTextOverride` prints `<sub>.<key name>`, so the value's own spelling is `T.Name`; `:3227-3246` (`tryFoldOrNull`) resolves the key against the witness table and folds when the entry is a `val`. The core module writes exactly that form: `source/slang/hlsl.meta.slang:506-511` declares `interface __ITextureShape { static const int flavor; dimensions; planeDimensions; }` and `:1183` reads it through the generic's conformance as `typealias TextureCoord = vector<float, Shape.dimensions>;` inside `_Texture<..., Shape : __ITextureShape, ...>`.                                                                                                                                                                                                  | named the spelling in the `WitnessLookupIntVal` row: `T.Name` for a `static const int` interface requirement read through a type parameter's conformance, with `Shape.dimensions` in `_Texture` as the concrete instance                                                                                                                                  |
| b226ec1bc278 | fixed    | `source/slang/core.meta.slang:44-60` (and `:62-78` for `snorm`) declares the modifier as marking a buffer or texture element type as backed by normalized data, states it does not change the semantics of the `float` or vector carrying it, and says "Some platforms may require a `unorm` qualifier for such buffers and textures, and others may operate correctly without it". `source/slang/hlsl.meta.slang:1130-1134` is the WGSL texel check whose `static_assert` requires `T` to be `float`/`int`/`uint` or a vector of one, which a `ModifiedType` element fails; bundle test `unormmodifierval-carried-into-target-check.slang` pins the resulting E41400. The HLSL/SPIR-V/GLSL/Metal/CUDA renderings observed by the gap are decided in `source/slang/slang-emit-*.cpp`, not in `watched_paths`, and were not written. | added a paragraph to `### Modifier values` stating that the value rides on the type into every later type-level decision, that per-target survival is by design, and that the WGSL texel check rejects the modified element type where the bare one passes; pointed at `../pipeline/06-emit.md` for the emitted spellings                                 |
| 14976d471146 | fixed    | `source/slang/slang-ast-base.h:434-437` — `Val::equals` is `this == val \|\| resolve() == val->resolve()`, so every surface comparison of two compile-time values is decided by node identity after resolution; the section already records that `PolynomialIntVal` canonicalizes commuted spellings. Bundle test `polynomialintval-equivalent-expressions.slang` shows the consequence at the language surface: `int sumBoth<let N : int>(int a[2*N+3], int b[3+2*N])` type-checks `int c[2*N+3] = a;` and uses both parameters interchangeably.                                                                                                                                                                                                                                                                                   | added a sentence to `### Hash-consing and the ASTBuilder` giving the surface reading of the invariant — `int[2*N+3]` and `int[3+2*N]` are the same type inside a generic over `let N : int`, and would not be if either spelling built a second node                                                                                                      |
| f9503231dcd0 | deferred | The spelling is `where countof(Pack) == <expr>`, parsed by `maybeParseGenericConstraints` in `source/slang/slang-parser.cpp:1965-1980`, which creates the `GenericVariadicPackCountConstraintDecl` (the comment at `:1939-1943` also records that `N == countof(T)` is rejected). That file is not in this page's `watched_paths`; the watched half only reaches `ASTBuilder::getDeclaredVariadicPackCountWitness` / `getConcreteVariadicPackCountWitness` (`source/slang/slang-ast-builder.cpp:1380-1391`), which shows construction but no surface. The core module never writes the constraint — the only `countof` uses in `hlsl.meta.slang` (`:31747`, `:31764`) are `static_assert`s, not `where` clauses — and no bundle test pins the form. Add `source/slang/slang-parser.cpp` to `watched_paths` to close this.           | —                                                                                                                                                                                                                                                                                                                                                         |
