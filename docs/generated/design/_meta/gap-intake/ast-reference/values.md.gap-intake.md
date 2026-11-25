---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ast-reference/values.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 3
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/values.md

## Summary

This is a re-intake of three gaps that were recorded `fixed` and are
reported `reopened` (the reporting bundle was regenerated and still
raised them). Nothing was escalated. All three were re-decided `fixed`
with stronger edits: the earlier fixes added prose but gave no
worked example, did not state the user-visible reading as a plain
verdict, and (for the modifier) stopped at "per-target, see the emit
page". `f9503231dcd0` (deferred) was not part of this queue.

The refresh also found that the earlier text had drifted: the
`hlsl.meta.slang` line range for the WGSL texel check had moved, and
the `createTypeEqualityWitness` call-site list was stale.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f6fb37ba186d | fixed | `source/slang/slang-ast-val.h:17-35` (`DirectDeclRef` has one `Decl` operand; comment `MemberDeclRef(DirectDeclRef(A), B) ==> DirectDeclRef(B)` folds the path); `:62-76` (`LookupDeclRef` operand 2 is a `SubtypeWitness`); `:98-120` (`GenericAppDeclRef` args from operand 2). Equality of `Val`s is node identity after resolution (`source/slang/slang-ast-base.h:434-437`), so differing args or witness give distinct values. User-visible halves pinned by bundle tests `genericappdeclref-different-args-distinct.slang` (E30019) and `lookupdeclref-associated-type-specializes.slang`; read, not run (Linux-only slangc binary). | added a verdict paragraph to `### DeclRef family`: Direct and Member are not distinguishable from program text; only GenericApp (`Box<int>` vs `Box<float>`) and Lookup (`T.Elem` per conformer) have a language-level reading |
| 14976d471146 | fixed | `source/slang/slang-ast-base.h:434-437` (`Val::equals` is `this == val` or equal `resolve()` results); `source/slang/slang-ast-val.cpp:1840-1900` (polynomial factors and terms are sorted, so `2*N+3` and `3+2*N` canonicalize identically); `source/slang/slang-ast-val.h` `BuiltinOperationIntVal` invariant as already documented. Example from bundle `polynomialintval-equivalent-expressions.slang` (`sumBoth<let N : int>(int a[2*N+3], int b[3+2*N])` with `int c[2*N+3] = a;`); not run here. | added a worked example to `### Hash-consing and the ASTBuilder` showing the surface consequence: the two bounds are one value, so the assignment type-checks, and a second node would make it a mismatch |
| b226ec1bc278 | fixed | HLSL: `source/slang/slang-emit-c-like.cpp:484-493` calls `_emitPrefixTypeAttr`; `source/slang/slang-emit-hlsl.cpp:2332-2345` emits `unorm `/`snorm `. SPIR-V: `source/slang/slang-emit-spirv.cpp:3347-3380` derives the image format from `UNormAttr` (`Rgba8` for 3-4 components) when it is otherwise unknown. Erasure: `source/slang/slang-emit-c-like.cpp:403-410` default emits nothing; `source/slang/slang-emit-wgsl.cpp:767-775` emits the base type. WGSL front-end check: `source/slang/hlsl.meta.slang:1329-1337`. Note the emitters are not in `watched_paths`; they were read directly and cited, and the doc says so. GLSL/Metal/CUDA were only checked by absence of attribute handling (grep); the gap's GLSL `layout(rgba32f)` remark was not traced. | rewrote the `### Modifier values` paragraph as a per-target list (HLSL re-spell, SPIR-V format operand, erased elsewhere) with an explicit statement of what was and was not verified |

## Unresolved points

- The gap's claim that WGSL rejects `Texture2D<unorm float4>` (E41400)
  could not be confirmed from source: `is` goes through `isSubtype`, and
  for a `ModifiedType` without `no_diff` the inheritance computation
  returns the base type's info unchanged
  (`source/slang/slang-check-inheritance.cpp:2421-2488`), which may now
  let the modified type pass the `static_assert`. The doc records the
  earlier observation as unverified for this commit rather than as
  behaviour. This is a candidate bundle re-run or, if the bare-vs-modified
  result is inconsistent with the emitter's WGSL attribute erasure, a
  finding; no finding is claimed here.
- Operator follow-up: add `source/slang/slang-emit-c-like.cpp`,
  `slang-emit-hlsl.cpp`, `slang-emit-spirv.cpp`, `slang-emit-wgsl.cpp`
  to this page's `watched_paths` so the per-target list is regenerated
  and checked by the digest.
