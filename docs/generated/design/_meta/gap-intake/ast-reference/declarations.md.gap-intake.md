---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:42:09Z
target_doc: ast-reference/declarations.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 7
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated_to_finding: 0
---

# Gap-intake report for ast-reference/declarations.md

## Summary

Nothing was escalated. Six of the seven gaps were fixed by editing six
sections — two `## Nodes` rows (`GLSLInterfaceBlockDecl`,
`AttributeDecl`), `### SyntaxDecl and the syntax-as-declaration model`,
`### NamespaceDecl, ModuleDecl, FileDecl`,
`### EnumDecl and EnumCaseDecl`, `### AccessorDecl family` — plus two
new callouts, `### SemanticDecl` and `### GLSLInterfaceBlockDecl`. One
gap was deferred: the claim that `type_param` /
`__generic_value_param` are bound by external specialization arguments
cannot be confirmed from this page's `watched_paths`. Nothing was
rejected.

Two hypotheses did not survive confirmation and were replaced by what
the source actually says. `91fe0f7f6aa5` asked for a
`GLSLInterfaceBlockDecl` example "accepted in GLSL-compatibility
mode"; in fact `Parser::ParseGLSLInterfaceBlock`
(`source/slang/slang-parser.cpp:6457`) has no caller anywhere under
`source/`, and the GLSL `uniform` / `buffer` / `in` / `out` block
spellings desugar to a `StructDecl` + `VarDecl` pair instead — so the
node has no reachable spelling to exemplify, and its `Grammar` cell is
now `(none)`. `2fede3d30d6e` assumed the "empty body" the document
mentions is `{ }`; the parser's own comment sits on the *semicolon*
branch of `parseStorageDeclBody`, and the two forms coincide only
because neither records an accessor member.

The `attribute_syntax` gap (`ec5f9a6f1ce7`) is `drift-from-source`
where the source backs the observation, so the document was the thing
that was wrong: `parseAttributeSyntaxDecl` resolves the `: <class>`
clause through `ASTBuilder::findSyntaxClass`, which can only find a
class the compiler was built with. The internal error the reporting
bundle saw for an unknown class is a separate compiler defect already
recorded as
`docs/generated/tests/_meta/findings/declarations-attribute-syntax-unknown-class-ice.yaml`;
the new prose states the requirement without blessing the ICE as
documented behaviour.

Operator follow-up: `ca655cf14cfe` needs either a `watched_paths`
expansion to `source/slang/slang-check-shader.cpp` (which turns a
`GlobalGenericParamDecl` into a specialization parameter, `:2798-2806`)
and `source/slang/slang-diagnostics.lua` (which defines E38207,
`:4620-4625`), or a CLI harness. The same shape of blocker applies to
the checker-side halves of `ba8f19c2205d` (which base is the tag type,
what the choice affects) and `2fede3d30d6e` (implicit-getter
materialization) — those live in `source/slang/slang-check-decl.cpp`,
so the edits state the parse-shape facts and leave the checking rules
to `pipeline/03-semantic-check.md`, as the generation prompt's
forbidden-content clause requires.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 2fede3d30d6e | fixed | `source/slang/slang-parser.cpp:4773-4791` — `parseStorageDeclBody` takes either a braced accessor list or a bare `;`, and the comment "empty body should be treated like `{ get; }`" sits on the semicolon branch, not on an empty `{ }`; `:4706-4728` — `parseAccessorDecl` calls `Unexpected(parser)` for any token that is not `get` / `set` / `ref`, so `property int p { return v; }` cannot parse. An empty `{ }` adds no member either, so both forms reach checking with zero accessors (corroborated at `source/slang/slang-check-decl.cpp:16602-16628`, outside `watched_paths`). The legality of `property int p { }` as a requirement is the verified form in `docs/generated/tests/design/ast-reference/declarations/accessordecl-implicit-getter-property.slang` and `accessordecl-implicit-getter-subscript.slang` | rewrote the `### AccessorDecl family` sentence to separate a statement body (a parse error) from the two no-accessor forms `{ }` and `;` that share the implicit-getter case |
| ecac3ca82a5b | fixed | `source/slang/slang-parser.cpp:4629-4634` — `parseFileDecl` creates the `FileDecl` and calls `parseDeclBody`, which at `:6292-6300` pushes the decl's own scope; contrast `parseTransparentBlockDecl` at `:4616-4627`, which parses into the *enclosing* container and so really is transparent. `source/slang/slang-ast-decl.h:900` — the include machinery's `FileDecl` is the one stored in `IncludeDeclBase::fileDecl`; `:1191-1196` declares `addSiblingScopeForContainerDecl` ("Add a sibling lookup scope for `dest` to refer to `source`"), the mechanism that makes such a scope transparent | added three sentences to `### NamespaceDecl, ModuleDecl, FileDecl` saying transparency belongs to the checker-built file scopes, not to a hand-written `__file_decl` block |
| ec5f9a6f1ce7 | fixed | `source/slang/slang-parser.cpp:5543-5589` — `parseAttributeSyntaxDecl` resolves the `: <class>` clause with `parser->astBuilder->findSyntaxClass(classNameAndLoc.name)` (`:5587`) followed by `SLANG_ASSERT(syntaxClass)` (`:5589`), so only a class the compiler was built with can be named; `:5534-5541` — the comment says attribute-specific code is not invoked during parsing and "all specialized behavior takes place during semantic checking", which is why placement rules come from the C++ class. All 110 uses are in `source/slang/core.meta.slang` (108, e.g. `:4381`) and `source/slang/hlsl.meta.slang` (2). The unknown-class internal error is already covered by `docs/generated/tests/_meta/findings/declarations-attribute-syntax-unknown-class-ice.yaml` and is deliberately not documented | reworded the `AttributeDecl` row to "binds the spelling ... to an AST attribute class the compiler already knows" and added an `attribute_syntax` paragraph to the `SyntaxDecl` callout |
| 91fe0f7f6aa5 | fixed | `source/slang/slang-parser.cpp:214` and `:6457-6472` — `Parser::ParseGLSLInterfaceBlock` is the only site that constructs a `GLSLInterfaceBlockDecl`, and a tree-wide grep finds no call to it, so the node is unreachable at this commit. The GLSL block forms go elsewhere: `:5866` (`buffer` -> `parseGLSLShaderStorageBufferDecl`), `:5881` (`uniform` -> `parseHLSLCBufferDeclWithLayout`), `:5891` (`in`/`out` -> `ParseBufferBlockDecl` with an empty wrapper name), all of which build a `StructDecl` + `VarDecl` pair at `:4010-4110` and add a `TransparentModifier` at `:4159` when no instance name is written. `:3530-3541` confirms the `options.allowGLSLInput` gate | set the row's `Grammar` to `(none)`, noted the node is unreached, and added a `### GLSLInterfaceBlockDecl` callout naming the three desugarings instead of inventing an example |
| d03e7f78dc55 | fixed | `source/slang/slang-parser.cpp:4979-4992` — `parseSemanticDecl` reads `semantic <name>` then `parseSemanticDeclBody`, which at `:4957-4977` requires a braced body; `:4917-4954` — `parseSemanticAccessorDecl` accepts only `get : <type>;` and `set : <type>;`, producing `SemanticGetterDecl` / `SemanticSetterDecl`. The two-line surface is the verified form in `docs/generated/tests/design/ast-reference/declarations/semanticdecl-typed-getter.slang` | added a `### SemanticDecl` callout with the `semantic MySem { get : int; }` / `struct S { int v : MySem; }` example |
| ba8f19c2205d | fixed | `source/slang/slang-parser.cpp:6490-6559` — `parseEnumDecl` parses the `: T` part with the same `parseOptionalInheritanceClause` a struct uses, so the tag type and any conformances become sibling `InheritanceDecl` children; no assignment to `tagType` appears anywhere in `slang-parser.cpp`, and `source/slang/slang-ast-decl.h:448` declares the field on `EnumDecl`. Which base becomes the tag type, and the default when none is written, are decided in `source/slang/slang-check-decl.cpp:12185-12299` — outside `watched_paths` and forbidden content per `_meta/prompts/ast-reference-declarations.md:63-64`, so they are left to `pipeline/03-semantic-check.md` | added the `enum E : uint8_t { A, B }` spelling and the base-list parse shape to `### EnumDecl and EnumCaseDecl`, and said the parser never sets `tagType` |
| ca655cf14cfe | deferred | Both halves of the claim are outside `watched_paths`. `source/slang/slang-parser.cpp:4384-4412` (`parseGlobalGenericTypeParamDecl`, `parseGlobalGenericValueParamDecl`) only shows that `type_param` / `__generic_value_param` parse into `GlobalGenericParamDecl` / `GlobalGenericValueParamDecl`, and `source/slang/slang-ast-decl.h:572-583` only calls them "generic entry-point" / "existential value" parameters. The binding path is `source/slang/slang-check-shader.cpp:2798-2806` (the decl becomes a `SpecializationParam`) and the diagnostic is E38207 in `source/slang/slang-diagnostics.lua:4620-4625`; neither file is watched here, and `slangc` cannot be run on this host (Linux x86-64 build, arm64 host) to reproduce. Follow-up: expand `watched_paths` with those two files, or add a CLI-driving harness on the tests side | — |
