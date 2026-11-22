---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 11
actions:
  fixed: 10
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for syntax-reference/keywords-and-builtins.md

## Summary

Nothing was escalated. All eleven gaps were REOPENED: an earlier intake
recorded them as fixed, yet regenerated bundles still report them. They
were re-decided against `source/slang/slang-parser.cpp` and the other
watched files at `2518a942d59b1228fc8cc70a37739baab82b6002`, in the
same pass as a refresh that re-derived every line citation (the parser
moved by several hundred lines). Ten are `fixed` again, with the doc now
stating the behaviour more concretely; one (`9a1a7ad263e3`) is
`rejected-out-of-scope`, because the effect it asks for is decided
outside the watched paths.

Notes for the operator, none blocking. (1) `slang-ir-legalize-varying-params.cpp`
is now watched, which let `1d8770e795d5` name the targets that map
`SV_WaveIndex`; the GLSL / SPIR-V legalizer is a different, unwatched
file and the doc only says this file has no arm for it. (2) The `functype`
callability observation in `80abe4cc4205` (a local of function type
rejected on call) is a semantic-check matter outside the watched paths;
the doc states the spelling and says nothing about callability, and no
finding was opened because the watched source neither promises nor
denies it. (3) Source facts the refresh changed in this doc: `class` in
HLSL source now parses through `ParseStruct` as `HLSLClassDecl`;
`allowGLSLInput` was replaced by `getSourceLanguage()`; `$for` is
deprecated from language version 2025 and removed in 202c; new simple
modifiers `__ref_readonly`, `__ref_writeonly`, `__borrow`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f848c4fac6e9 | fixed | `source/slang/slang-parser.cpp:10661-10760` (`shared`, `volatile`, `coherent`, `restrict`, `readonly`, `writeonly` read no tokens), `:10932-10937` (`hitAttributeEXT`), `:10764` (`layout` reads `(` then names or `name = expr`), `:10484-10600` and `:10940-11040` (the `__`-prefixed argument forms, optional parentheses for `__intrinsic_op`, `__target_intrinsic`, `__specialized_for_target`, `__implicit_conversion`). Callbacks construct nodes only; declaration applicability is not checked there. | added a "Written as" column giving one usage form per row, marked the bare rows, and stated that the parser does not check the declaration kind |
| 1b40bd3f3dc7 | fixed | `source/slang/hlsl.meta.slang:29254-29298` (handle types, the two heap types with `[require(glsl_hlsl_spirv_wgsl, descriptor_handle)]` subscripts, and `static const ... ResourceDescriptorHeap` / `SamplerDescriptorHeap`), `:29095-29135` (`__implicit_conversion` constructor taking the untyped handle on each castable type, including `RWStructuredBuffer<T, L>` at `:29081`). | kept the `ResourceDescriptorHeap[i]` form and the `RWStructuredBuffer<uint> buf = ResourceDescriptorHeap[0];` example, and added line citations for each declaration |
| 1d8770e795d5 | fixed | `source/slang/hlsl.meta.slang:15-17` (semantics on hidden `in` globals, no `[require]`), `:18430` (`WaveGetWaveIndex()` with a `__target_switch` arm per target); `source/slang/slang-ir-legalize-varying-params.cpp:4608` (Metal context maps `WaveIndex` to `simdgroup_index_in_threadgroup`) and `:5380` (WGSL context maps it to `subgroup_id`); no other `WaveIndex` arm in that file. | named Metal and WGSL as the targets this legalizer maps `SV_WaveIndex` for, noted the GLSL / SPIR-V path is not in this file, and kept `WaveGetWaveIndex()` as the portable spelling |
| 0ff3440b9c31 | fixed | `source/slang/slang-parser.cpp:4593-4617` (`__constraint`, equality or subtype), `:4620-4632` (`__associatedfunc`: type expression then name), `:4494-4548` (`__func_extension`: target via syntax-decl lookup, optional `throws`, optional `->`, body); `source/slang/slang-check-decl.cpp:17466-17470` rejects it unless `CompilerOptionName::ExperimentalFeature` is set (declared flag-only in `slang-options.cpp`, per the earlier intake); examples at `source/slang/core.meta.slang:771` and `source/slang/hlsl.meta.slang:33606`. | refreshed the line citations and stated that `-experimental-feature` takes no value and is checked in semantic checking |
| 80abe4cc4205 | fixed | `source/slang/slang-parser.cpp:3705-3708` (`functype` lookahead), `:3403-3417` (`(`, comma-separated type list, mandatory `->`, result type), `:1731` (`functype F` generic parameter); usage `source/slang/hlsl.meta.slang:30554`. No parser code states whether a value of the type is callable. | refreshed citations, kept the `functype(<parameter types>) -> <result type>` spelling, and added an explicit statement that callability is not specified by the parser |
| bbc3d4c97d7b | fixed | `source/slang/slang-parser.cpp:3669-3677` (HLSL source: `ParseStruct(HLSLClassDecl)`, with the comment "HLSL `class` declarations have value semantics"; otherwise `ParseClass`, "reference-type `ClassDecl`"), `:6719-6786` (`ParseStruct`: anonymous name, `parseOptGenericDecl`, `S;`, `S = T;`), `:6789-6800` (`ParseClass`: name, inheritance, body), `:7507-7518` (`class` routed to declarations in blocks only for HLSL). | rewrote the `class` versus `struct` paragraph around the source-language split and the grammar difference |
| 131453a3ff0f | fixed | `source/slang/slang-parser.cpp:8412-8475` (`sizeof`, `alignof`, `countof`), `:8552-8575` (`__floatAsInt(expr)`, `__getAddress(expr)`), `:8578-8590` (`try`, `no_diff` leaf expressions), `:3289-3365` (`fwd_diff`, `bwd_diff`, `__apply`, `__func_as_type`, `__dispatch_kernel`), `:10093` (`new`). | added `__func_as_type(f)`, `__getAddress(v)`, `__floatAsInt(x)` to the one-operand examples next to the existing per-row forms and refreshed the `new` citation |
| a2d9ef39a8a4 | fixed | `source/slang/slang-parser.cpp:5475-5560` (`syntax <name> [: <class>] [= <existing>];`, class resolved via `findSyntaxClass` at `:5497`), `:5830-5900` (`attribute_syntax [<Name>(...)] : <class>;`); `source/slang/slang-ast-builder.h:884-891`; core uses at `source/slang/core.meta.slang:28` and `:164`. | stated that no user-level example exists in the tree and gave the two declaration shapes taken from the grammar |
| aa48d92eed58 | fixed | `source/slang/slang-parser.cpp:2607-2622` (`isReservedKeywordName`: only `struct`, `class`, `enum`, `typealias`, `typedef`), `:6252` (the one `gl_` inspection), no `SV_` test in the file, `:11120-11129` (only `__init`, `__subscript`, `__include` registered; bare spellings are not), `:9828-9831` (core-module `%__` register assert). | stated that both prefixes are advisory naming policy with no parser diagnostic, used a local named `SV_userVar` as the example, added line citations, and noted the one `__` check on SPIR-V registers |
| 9a1a7ad263e3 | rejected-out-of-scope | Every simple-modifier row uses the `getSyntaxClass<...>()` overload whose callback `parseSimpleSyntax` (`source/slang/slang-parser.cpp:5468`, registered at `:11085-11094`) constructs the node and reads nothing, so the parse-time meaning is the node. Emitted-code and runtime effects (matrix layout rewriting, interpolation, geometry stages) are decided in layout and emit code outside `watched_paths`; the owning page is `docs/generated/design/ast-reference/modifiers.md` (see its matrix-layout rows around lines 201-214). The doc already links there and says so. | — |
| fb6c0e3e7b12 | fixed | `source/slang/slang-parser.cpp:7210-7265` (`parseCompileTimeForStmt`: `$for(i in Range(N))`, version checks at `:7218-7227`), `:6959-7064` (`__stage_switch` / `__target_switch`), `:7067-7083` (`__intrinsic_asm`), `:7085-7110` (`__GPU_FOREACH`, literal `LAMBDA`), `:8002-8030` (`__requireCapability`), `:7342` (`$` entry). | refreshed every statement line citation and added the `$for` deprecation (2025) and removal (202c) diagnostics to the `for` row; the per-keyword forms already in the bullets below the table were kept |
