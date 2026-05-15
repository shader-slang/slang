---
generated: true
model: claude-opus-4.7
generated_at: 2026-05-15T15:58:00+00:00
source_commit: e75b9a3d03659cefb39882da3adecb2eb8751e0d
watched_paths_digest: 7176a22219b18c44d0407a900615049a6a078771700ca274d01d15aeb88bc3ad
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Compiler-Internals Glossary

This page defines the compiler-internals vocabulary used across the
other documents in this tree and across [../../source/](../../source/).
It is aimed at a new contributor who has just encountered an unfamiliar
term in one of the peer documents or in the code and wants a short,
anchored definition before drilling in further.

## Conventions

Every entry is tagged `[Slang]` or `[General]`:

- `[Slang]` — meaning is defined by this project; you would not learn it
  from a general compilers textbook.
- `[General]` — drawn from general compiler / programming-language
  theory; included because the project uses the term in its normal
  sense. `[General]` entries usually carry an `External:` link to
  Wikipedia or another canonical reference.

Every entry has a mandatory `See:` link to the peer document under
`docs/llm-generated/` that explains the term in depth. The glossary is a
lookup aid; the peer document is the source of truth.

This page does **not** redefine terms that already live in the
user-facing language/runtime glossary at
[../language-reference/glossary.md](../language-reference/glossary.md)
(threads, dispatch, observable behavior, tangled function, ...).
A handful of terms (notably `entry point`) have both a runtime sense and
a compile-pipeline sense; in those cases the language-reference glossary
owns the runtime sense and this page covers the compile-pipeline sense
explicitly.

## Terms

**abstract syntax tree** `[General]`
: A tree representation of the syntactic structure of source code,
  produced by the parser. In Slang, AST nodes derive from `NodeBase`
  declared in [slang-ast-base.h](../../source/slang/slang-ast-base.h)
  and split into the `Decl`, `Expr`, `Stmt`, `Type`, and `Modifier`
  families.

  See: [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md)
  External: https://en.wikipedia.org/wiki/Abstract_syntax_tree

**ASTBuilder** `[Slang]`
: The allocator and hash-consing helper used to construct AST nodes,
  declared in [slang-ast-builder.h](../../source/slang/slang-ast-builder.h).
  It owns the arena that AST nodes live in for the lifetime of a
  compile and deduplicates structural values such as types.

  See: [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md)

**block parameter** `[Slang]`
: An `IRParam` instruction at the head of an `IRBlock` that names a
  value flowing into the block from each predecessor. Slang's IR uses
  block parameters as the SSA-without-phi encoding: each predecessor's
  branch terminator passes one argument per parameter, and there are no
  separate `phi` instructions. Block parameters are listed first in the
  block; non-parameter instructions follow. Contrast with the
  conventional `phi` node (see `single static assignment (SSA)`).

  See: [ir-reference/control-flow.md](ir-reference/control-flow.md)

**capability atom** `[Slang]`
: One named feature, target, or extension in the capability system —
  the unit of granularity in
  [slang-capabilities.capdef](../../source/slang/slang-capabilities.capdef).
  Atoms are combined into capability sets that gate the use of intrinsics
  and decide whether a function is callable on a given target.

  See: [cross-cutting/targets.md](cross-cutting/targets.md)

**control-flow graph** `[General]`
: A directed graph whose nodes are basic blocks and whose edges are
  control transfers between them. Slang's IR represents functions as
  CFGs of `IRBlock`s; most IR passes walk this graph.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Control-flow_graph

**conversion cost** `[Slang]`
: An `unsigned int` score used by overload resolution to compare
  candidates' implicit-conversion chains. Each named conversion has a
  `kConversionCost_*` level in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h);
  per-argument costs are summed by `getImplicitConversionCostWithKnownArg`
  in
  [slang-check-conversion.cpp](../../source/slang/slang-check-conversion.cpp).
  Anything at or above `kConversionCost_GeneralConversion` is rejected
  for implicit use.

  See: [name-resolution/overload-resolution.md](name-resolution/overload-resolution.md)

**core module** `[Slang]`
: The built-in module written in Slang itself
  ([core.meta.slang](../../source/slang/core.meta.slang)) that defines
  fundamental types and intrinsics available to every shader. It is
  compiled at build time and embedded into `libslang`.

  See: [cross-cutting/core-module.md](cross-cutting/core-module.md)

**dataflow analysis** `[General]`
: A family of techniques that computes facts about values at every
  program point by propagating information along the CFG. Slang uses
  dataflow analysis in passes such as liveness, reachability, and
  use-def queries inside `slang-ir-*` files.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Data-flow_analysis

**dead-code elimination** `[General]`
: A transformation that removes instructions whose results are never
  used, and unreachable blocks. Implemented in
  [slang-ir-dce.cpp](../../source/slang/slang-ir-dce.cpp) and run
  multiple times during `linkAndOptimizeIR`.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Dead_code_elimination

**decl-ref** `[Slang]`
: A pair of a declaration plus the generic and existential substitutions
  needed to refer to it in a specific context. Internally `DeclRef`
  in [slang-ast-base.h](../../source/slang/slang-ast-base.h); the type
  system carries decl-refs rather than bare declarations so that
  generic instantiation is uniform with ordinary name lookup. Decl-refs
  are produced by name resolution (see
  [name-resolution/lookup.md](name-resolution/lookup.md)).

  See: [pipeline/03-semantic-check.md](pipeline/03-semantic-check.md)

**decoration** `[Slang]`
: A non-essential annotation attached to another IR instruction, encoded
  as a child instruction whose opcode is in the `Decoration` family of
  [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua) (every
  opcode name ends in `Decoration`). Decorations carry information such
  as the name hint preserved across passes, layout / binding indices,
  target-intrinsic spellings, loop hints, entry-point markers, and
  capability requirements. Synonym: `IRDecoration`. The full per-opcode
  list is in [ir-reference/decorations.md](ir-reference/decorations.md).

  See: [ir-reference/decorations.md](ir-reference/decorations.md),
  [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md)

**DiagnosticSink** `[Slang]`
: The interface that receives diagnostics (errors, warnings, notes)
  during a compile, declared in
  [slang-diagnostic-sink.h](../../source/compiler-core/slang-diagnostic-sink.h).
  Every compiler component takes a sink rather than printing directly.

  See: [cross-cutting/diagnostics.md](cross-cutting/diagnostics.md)

**differential pair** `[Slang]`
: A two-component value (primal, differential) used by Slang's
  automatic-differentiation machinery. The IR opcode
  `MakeDifferentialPair` constructs one;
  `DifferentialPairGetPrimal` / `DifferentialPairGetDifferential`
  project the components back out. The pair's type is one of the
  `DifferentialPairTypeBase` opcodes (`DiffPair`, `DiffRefPair`). See
  [ir-reference/differentiation.md](ir-reference/differentiation.md)
  for the full per-opcode list.

  See: [ir-reference/differentiation.md](ir-reference/differentiation.md)

**dominator** `[General]`
: In a CFG with a single entry, a block A *dominates* block B if every
  path from entry to B passes through A. Slang uses dominator
  information in SSA-form passes such as loop analysis and code motion.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Dominator_(graph_theory)

**downstream compiler** `[Slang]`
: An external compiler that consumes Slang's emitted source artifact and
  produces the final binary or intermediate-language output. Examples:
  DXC and fxc for HLSL → DXIL / DXBytecode; spirv-link, spirv-val, and
  spirv-opt for the SPIR-V direct-emit path; the Apple `metal` compiler
  for Metal → MetalLib; Tint for WGSL → SPIR-V; nvrtc for CUDA C++ →
  PTX. Slang dispatches the downstream tool from
  `createArtifactFromIR` in
  [slang-emit.cpp](../../source/slang/slang-emit.cpp); the downstream
  tools live outside the Slang source tree.

  See: [target-pipelines/index.md](target-pipelines/index.md)

**entry point** `[Slang]`
: In the compile-pipeline sense, a function plus a pipeline stage
  (vertex, fragment, compute, ...) that defines a kernel to compile.
  Modeled in the API by the `EntryPointRequest` / `EntryPointReference`
  family in [slang-compile-request.h](../../source/slang/slang-compile-request.h).
  The runtime sense (a function from which a thread begins execution)
  lives in the language-reference glossary.

  See: [pipeline/overview.md](pipeline/overview.md)

**existential type** `[Slang]`
: A type that abstracts over an unknown conforming implementation
  ("some `IFoo`"). Slang uses existentials to represent dynamic-dispatch
  values; the IR carries dedicated opcodes
  (`ExtractExistentialValue`, `LookupWitness`, ...) for them. The full
  list of existential-related IR opcodes is in
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md);
  the existential type opcodes (`BindExistentials`, `BoundInterface`,
  `RTTIPointerType`, `DynamicType`) live in
  [ir-reference/types.md](ir-reference/types.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md)

**FIDDLE** `[Slang]`
: A code-generation macro system used throughout the AST and IR. A
  `FIDDLE(...)` annotation in a header tells the build to generate
  reflection, visitor, and serialization boilerplate under
  `build/source/slang/fiddle/`. Most AST and IR node declarations are
  partially generated this way.

  See: [architecture/overview.md](architecture/overview.md)

**fossil format** `[Slang]`
: The memory-mappable binary serialization backend used for compiled
  modules, declared in
  [slang-serialize-fossil.h](../../source/slang/slang-serialize-fossil.h).
  Fossil is designed so that loading a module is a single `mmap` plus
  pointer fix-ups; it pairs with the RIFF container format.

  See: [cross-cutting/serialization.md](cross-cutting/serialization.md)

**hoistable instruction** `[Slang]`
: An IR instruction whose value depends only on its operands, marked
  `hoistable = true` in
  [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua). Such
  instructions are deduplicated globally inside an `IRModule` and live
  at the module scope rather than inside a function body — this is how
  Slang represents types and other "value" entities in SSA form. The
  per-opcode catalog flags every hoistable opcode with `H` in its
  flags column; see for instance
  [ir-reference/types.md](ir-reference/types.md) (every Type opcode is
  hoistable).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/index.md](ir-reference/index.md)

**inlining** `[General]`
: Replacing a call instruction with a copy of the callee's body. Slang
  inlines aggressively (in `slang-ir-inline.cpp` and related passes)
  because most front-end abstractions disappear at the IR level.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Inline_expansion

**intermediate representation** `[General]`
: A compiler's internal program form between the source AST and the
  emitted target code. Slang has a custom SSA-style IR (not LLVM)
  defined in [slang-ir.h](../../source/slang/slang-ir.h); essentially
  all optimization and legalization happens at this level.

  See: [pipeline/04-ast-to-ir.md](pipeline/04-ast-to-ir.md)
  External: https://en.wikipedia.org/wiki/Intermediate_representation

**IRBuilder** `[Slang]`
: The API used to construct IR instructions during AST-to-IR lowering
  and during IR transformations, declared in
  [slang-ir.h](../../source/slang/slang-ir.h). It owns an insertion
  point and routes hoistable instructions to module scope automatically.

  See: [pipeline/04-ast-to-ir.md](pipeline/04-ast-to-ir.md)

**IRDecoration** `[Slang]`
: A non-essential annotation attached to an IR instruction —
  things like name hints, layout, binding indices, target intrinsics, or
  loop hints. Decoration opcodes have the `*Decoration` suffix in
  [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua) and are
  preserved through optimization unless explicitly dropped. Synonym:
  `decoration`. The full per-opcode catalog is in
  [ir-reference/decorations.md](ir-reference/decorations.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/decorations.md](ir-reference/decorations.md)

**IRFunc** `[Slang]`
: The IR node representing a function: its signature, parameters, and a
  CFG of `IRBlock`s. Functions are the unit of inlining,
  specialization, and emission. Per-opcode reference for `func` and
  related structural opcodes is in
  [ir-reference/structure.md](ir-reference/structure.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/structure.md](ir-reference/structure.md)

**IRInst** `[Slang]`
: The base class for every IR node, declared in
  [slang-ir.h](../../source/slang/slang-ir.h). An `IRInst` carries an
  opcode (`kIROp_*`), a type, a list of operand uses, optional
  decorations, and an intrusive position in a parent (function, block,
  or module). The complete opcode catalog grouped by family lives in
  [ir-reference/index.md](ir-reference/index.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/index.md](ir-reference/index.md)

**IRModule** `[Slang]`
: The top-level container for an IR program. It holds module-scope
  values (hoistable instructions, global variables, functions) and is
  the unit of linking and serialization. The `module` opcode and the
  other module-structure opcodes are tabulated in
  [ir-reference/structure.md](ir-reference/structure.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/structure.md](ir-reference/structure.md)

**IROp** `[Slang]`
: The integer enum tag identifying every IR instruction opcode (e.g.
  `kIROp_Add`, `kIROp_StructType`, `kIROp_NameHintDecoration`).
  Declared in `slang-ir-insts-enum.h.fiddle` (generated by the build
  from [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua) into
  `build/source/slang/fiddle/`). The opcode is stored in `IRInst::m_op`
  and read via `IRInst::getOp()`; downstream casts (`as<IRFoo>()`,
  `cast<IRFoo>()`) test the opcode against a contiguous range
  determined by the Lua nesting.

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/index.md](ir-reference/index.md)

**layout IR module** `[Slang]`
: A per-target IR module produced by
  `TargetProgram::createIRModuleForLayout` in
  [slang-lower-to-ir.cpp](../../source/slang/slang-lower-to-ir.cpp)
  whose only contents are `IRLayoutDecoration`s on stub
  `IRGlobalVar` and entry-point `IRFunc` instances for one
  specific target's layout rules. The layout IR module is a
  sibling of the executable per-translation-unit IR module: it
  is cached on `TargetProgram::m_irModuleForLayout`, is not fed
  into `linkAndOptimizeIR`, and carries no function bodies for
  user-defined functions. SPIR-V and Metal targets additionally
  receive `IRRequireCapabilityAtomDecoration`s on entry-point
  stubs; other targets do not.

  See: [pipeline/04c-layout-ir.md](pipeline/04c-layout-ir.md)

**lexer** `[General]`
: The compiler stage that converts a source character stream into a
  sequence of tokens. Slang's lexer is in
  [slang-lexer.cpp](../../source/compiler-core/slang-lexer.cpp); the
  token kinds are listed in
  [slang-token-defs.h](../../source/compiler-core/slang-token-defs.h).

  See: [pipeline/01-lex-preprocess.md](pipeline/01-lex-preprocess.md)
  External: https://en.wikipedia.org/wiki/Lexical_analysis

**linkage** `[Slang]`
: The compile-time object that scopes loaded modules, search paths,
  target requirements, and the core module. Roughly the "compiler
  configuration handle" exposed through `slang::ILinkage` in
  [slang.h](../../include/slang.h).

  See: [architecture/overview.md](architecture/overview.md)

**lookup breadcrumb** `[Slang]`
: A linked navigation step recorded during name lookup so that the
  checker can reconstruct the canonical AST expression for a found
  decl. `LookupResultItem_Breadcrumb::Kind` enumerates the kinds:
  `Member` (a transparent in-scope decl was looked through), `Deref`
  (a pointer-like value was implicitly dereferenced), `SuperType` (a
  super-type witness was applied), `This` (an implicit `this`/`This`
  needs to be supplied). Declared in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h).

  See: [name-resolution/lookup.md](name-resolution/lookup.md)

**lookup mask** `[Slang]`
: A `uint8_t` bitset that filters lookup results by decl kind. Values
  are `type`, `Function`, `Value`, `Attribute`, `SyntaxDecl`,
  `Semantic`; the `Default` combination is the one used by the parser
  and most checker entry points. Declared as `LookupMask` in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h);
  applied by `DeclPassesLookupMask` in
  [slang-lookup.cpp](../../source/slang/slang-lookup.cpp).

  See: [name-resolution/lookup.md](name-resolution/lookup.md)

**lookup options** `[Slang]`
: A `uint8_t` bitset of behavior flags consulted by name lookup. The
  flags include `IgnoreBaseInterfaces`, `Completion`, `NoDeref`,
  `ConsiderAllLocalNamesInScope`, `IgnoreInheritance`,
  `IgnoreTransparentMembers`. Declared as `LookupOptions` in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h).

  See: [name-resolution/lookup.md](name-resolution/lookup.md)

**lookup result** `[Slang]`
: The outcome of name lookup during semantic checking — zero, one, or
  multiple `LookupResultItem` entries, each carrying a decl-ref and a
  breadcrumb chain describing how it was reached. `LookupResult` is
  defined in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h);
  it is produced by the entry points in
  [slang-lookup.h](../../source/slang/slang-lookup.h).

  See: [name-resolution/lookup.md](name-resolution/lookup.md)

**lower-to-IR** `[Slang]`
: The pipeline stage that walks a type-checked AST and emits Slang IR,
  driven by `generateIRForTranslationUnit` in
  [slang-lower-to-ir.cpp](../../source/slang/slang-lower-to-ir.cpp).

  See: [pipeline/04-ast-to-ir.md](pipeline/04-ast-to-ir.md)

**mandatory optimization pass** `[Slang]`
: An IR pass that runs unconditionally on every per-translation-unit
  IR module inside `generateIRForTranslationUnit` (in
  [slang-lower-to-ir.cpp](../../source/slang/slang-lower-to-ir.cpp))
  before the module is cached on `Module::m_irModule` and pulled
  into `linkAndOptimizeIR` by `linkIR`. The set comprises Phase B
  lowering passes (`prelinkIR`, `lowerErrorHandling`, `lowerDefer`,
  `synthesizeBitFieldAccessors`, `lowerExpandType`,
  `insertDebugValueStore`) and Phase C optimization passes
  (`constructSSA`, `applySparseConditionalConstantPropagation`,
  per-function `eliminateDeadCode`, optional `simplifyCFG` +
  `peepholeOptimize`, optional `invertLoops`, and the
  `performMandatoryEarlyInlining` fixed-point loop). These passes
  serve two purposes: simplifying the IR shape that downstream
  passes consume, and establishing dataflow invariants that the
  Phase D validators rely on. They are target-agnostic — the same
  sequence runs for every shader target.

  See: [pipeline/04b-pre-link-passes.md](pipeline/04b-pre-link-passes.md)

**module** `[Slang]`
: In the compile-pipeline sense, a unit of separate compilation: the
  result of parsing, checking, and lowering one or more translation
  units to IR. Modules can be serialized, linked, and loaded
  independently. Exposed as `slang::IModule` in
  [slang.h](../../include/slang.h).

  See: [architecture/overview.md](architecture/overview.md)

**monomorphization** `[General]`
: The transformation that replaces a generic function with one
  specialized copy per concrete type argument set, eliminating runtime
  type parameters. Slang's specialization pass in
  [slang-ir-specialize.cpp](../../source/slang/slang-ir-specialize.cpp)
  and the `Specialize` IR opcode drive this.

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md)
  External: https://en.wikipedia.org/wiki/Monomorphization

**name resolution** `[General]`
: Binding identifier uses to the declarations they refer to, taking
  scope, generics, inheritance, and overloading into account. In Slang
  this happens during semantic checking and produces decl-refs. The
  detailed rules — scope construction, the lookup algorithm,
  shadowing, visibility, and overload resolution — are documented in
  the [name-resolution/](name-resolution/) subtree.

  See: [name-resolution/index.md](name-resolution/index.md),
  [pipeline/03-semantic-check.md](pipeline/03-semantic-check.md)
  External: https://en.wikipedia.org/wiki/Name_resolution_(programming_languages)

**overload resolution** `[General]`
: Choosing one declaration from a set of candidates that share a name
  by ranking them on parameter compatibility, conversion cost, and
  generic specificity. Slang's implementation builds `OverloadCandidate`
  records and filters them through a multi-stage pipeline in
  [slang-check-overload.cpp](../../source/slang/slang-check-overload.cpp).

  See: [name-resolution/overload-resolution.md](name-resolution/overload-resolution.md)
  External: https://en.wikipedia.org/wiki/Function_overloading

**parent instruction** `[Slang]`
: An IR instruction that owns a list of child instructions, marked
  `parent = true` in
  [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua). Examples:
  `module` (children: hoistable values, global vars, functions),
  `func` (children: blocks), `block` (children: parameters and
  ordinary instructions), `StructType` / `ClassType` /
  `InterfaceType` (children: fields, keys, requirements),
  `WitnessTable` (children: requirement-to-impl mappings), `Generic`
  (children: the type-level computation body). The IR builder routes
  newly created child instructions to the current parent via its
  insertion point.

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/structure.md](ir-reference/structure.md)

**parser** `[General]`
: The compiler stage that consumes tokens and produces an AST. Slang's
  parser is a recursive-descent parser in
  [slang-parser.cpp](../../source/slang/slang-parser.cpp) extended by
  the syntax-decl mechanism.

  See: [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md)
  External: https://en.wikipedia.org/wiki/Parsing

**partial generic application** `[Slang]`
: A still-generic intermediate that overload resolution produces when
  it picks a `GenericDecl` whose type arguments are not all known yet.
  Represented by `PartiallyAppliedGenericExpr` in
  [slang-ast-expr.h](../../source/slang/slang-ast-expr.h); the
  remaining type parameters are deduced when the partial value is
  invoked or used in a context that supplies them.

  See: [name-resolution/overload-resolution.md](name-resolution/overload-resolution.md)

**prelude** `[Slang]`
: A target-specific snippet of source code that is prepended to the
  emitted output so that the generated code can use Slang-defined
  intrinsics, types, and helpers. Preludes live under
  [../../prelude/](../../prelude/) (one per target family).

  See: [cross-cutting/core-module.md](cross-cutting/core-module.md)

**preprocessor** `[General]`
: A stage that handles `#include`, macros, and conditional compilation
  before parsing proper. Slang's preprocessor is in
  [slang-preprocessor.cpp](../../source/slang/slang-preprocessor.cpp);
  it works on a token stream emitted by the lexer.

  See: [pipeline/01-lex-preprocess.md](pipeline/01-lex-preprocess.md)
  External: https://en.wikipedia.org/wiki/Preprocessor

**profile** `[Slang]`
: A coarse handle that pairs a stage with a feature level
  (`sm_6_6`, `glsl_450`, ...). Defined in
  [slang-profile.h](../../source/slang/slang-profile.h). Mostly a
  legacy concept now layered on top of the capability system.

  See: [cross-cutting/targets.md](cross-cutting/targets.md)

**recursive descent** `[General]`
: A top-down parsing technique in which each non-terminal of the
  grammar is implemented as a function that consumes tokens and
  recursively calls peers. Slang uses recursive descent throughout
  [slang-parser.cpp](../../source/slang/slang-parser.cpp).

  See: [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md)
  External: https://en.wikipedia.org/wiki/Recursive_descent_parser

**RIFF container** `[Slang]`
: A chunked tagged-container format used to bundle serialized AST, IR,
  and auxiliary data into a single artifact. Implemented in
  [slang-serialize-riff.h](../../source/slang/slang-serialize-riff.h);
  pairs with the fossil format to give random-access loading.

  See: [cross-cutting/serialization.md](cross-cutting/serialization.md)

**scope** `[Slang]`
: A node in the lexical chain that maps names to the declarations
  defined in one block, declaration body, or generic header. The
  `Scope` struct in
  [slang-ast-base.h](../../source/slang/slang-ast-base.h) carries a
  `containerDecl` pointer plus `parent` and `nextSibling` links; the
  parser builds the chain on the fly and stores it on the AST so the
  checker can re-walk it.

  See: [name-resolution/scopes.md](name-resolution/scopes.md)
  External: https://en.wikipedia.org/wiki/Scope_(computer_science)

**session** `[Slang]`
: The top-level compiler instance that owns global state shared across
  compiles — the core module, downstream compiler discovery, and the
  built-in target descriptors. Exposed as `slang::IGlobalSession`.

  See: [architecture/overview.md](architecture/overview.md)

**shadowing** `[General]`
: A name defined in an inner scope hides a same-named name from an
  outer scope for the duration of that inner scope. Slang implements
  shadowing implicitly by stopping unqualified lookup at the first
  scope that yields a match, and additionally via per-decl markers —
  `Decl::hiddenFromLookup` and the `_prevInContainerWithSameName`
  chain in [slang-ast-base.h](../../source/slang/slang-ast-base.h) —
  that prune earlier declarations in the same container.

  See: [name-resolution/lookup.md](name-resolution/lookup.md)
  External: https://en.wikipedia.org/wiki/Variable_shadowing

**single static assignment (SSA)** `[General]`
: An IR property in which every value is defined exactly once and uses
  refer to that single definition. Slang's IR is in SSA form. Instead
  of `phi` nodes, Slang uses `block parameter`s: each `IRBlock`
  declares parameters via leading `IRParam` instructions, and each
  predecessor's branch terminator (`unconditionalBranch`,
  `conditionalBranch`, `loop`, `ifElse`, `Switch`) passes one argument
  per parameter. See
  [ir-reference/control-flow.md](ir-reference/control-flow.md) for the
  per-opcode catalog.

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/control-flow.md](ir-reference/control-flow.md)
  External: https://en.wikipedia.org/wiki/Static_single-assignment_form

**source-loc** `[Slang]`
: A compact encoded reference to a position in a source file, declared
  in
  [slang-source-loc.h](../../source/compiler-core/slang-source-loc.h).
  Every token, AST node, and IR instruction can carry one; the
  decoder maps it back to file/line/column on demand.

  See: [pipeline/01-lex-preprocess.md](pipeline/01-lex-preprocess.md)

**specialization** `[Slang]`
: The IR transformation that resolves generic parameters and
  existential witnesses into concrete types and values, primarily
  via the `Specialize` opcode and
  [slang-ir-specialize.cpp](../../source/slang/slang-ir-specialize.cpp).
  The result is a monomorphic IR ready for backend-specific lowering.
  The opcodes consumed and produced by specialization
  (`specialize`, `lookupWitness`, `MakeExistential`,
  `ExtractExistential*`) are catalogued in
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md).

  See: [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md),
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md)

**syntax-decl** `[Slang]`
: A declaration that introduces a parser-level construct from inside
  Slang source. Most keywords (`if`, `for`, `struct`, `__init`, ...)
  are bound as syntax-decls in `core.meta.slang` rather than being
  hardcoded into the lexer; the dispatch table is the
  `SyntaxParseInfo` array in
  [slang-parser.cpp](../../source/slang/slang-parser.cpp).

  See: [syntax-reference/keywords-and-builtins.md](syntax-reference/keywords-and-builtins.md)

**target** `[Slang]`
: A `TargetRequest` describing one desired output: a code-generation
  format (HLSL, SPIR-V, Metal, WGSL, C++, CUDA, ...), plus a profile,
  capability set, and per-target options. Multiple targets can share
  one compile request.

  See: [cross-cutting/targets.md](cross-cutting/targets.md)

**target legalization driver** `[Slang]`
: A target-specific IR pass that runs inside `linkAndOptimizeIR` and
  performs the bulk of the target's pre-emit transformations as a
  single pass call. Examples are `legalizeIRForSPIRV`
  ([slang-ir-spirv-legalize.cpp](../../source/slang/slang-ir-spirv-legalize.cpp)),
  `legalizeIRForMetal`
  ([slang-ir-metal-legalize.cpp](../../source/slang/slang-ir-metal-legalize.cpp)),
  and `legalizeIRForWGSL`
  ([slang-ir-wgsl-legalize.cpp](../../source/slang/slang-ir-wgsl-legalize.cpp)).
  Not every target has one; HLSL and CUDA do their target-specific
  work through individual `SLANG_PASS` calls instead.

  See: [target-pipelines/index.md](target-pipelines/index.md)

**target intrinsic** `[Slang]`
: A function whose implementation is supplied as a per-target
  expression rather than as Slang body. Declared in the core module
  by attaching a `[__target_intrinsic(target, "spelling")]`
  attribute, which lowers to a `TargetIntrinsicDecoration` IR node
  carrying the per-target spelling and applicability predicate.
  Related decorations are `IntrinsicOpDecoration` (a numeric
  intrinsic opcode for the emit backends), `RequirePreludeDecoration`
  (forces a prelude snippet on use), and the `TargetSwitchDecoration`
  family. The per-decoration catalog is in
  [ir-reference/decorations.md](ir-reference/decorations.md).

  See: [ir-reference/decorations.md](ir-reference/decorations.md),
  [cross-cutting/targets.md](cross-cutting/targets.md)

**terminator instruction** `[Slang]`
: The last instruction in an `IRBlock`; it decides what runs next
  (or that nothing does). Members of the `TerminatorInst` family in
  [slang-ir-insts.lua](../../source/slang/slang-ir-insts.lua) —
  `Return`, `unconditionalBranch`, `conditionalBranch`, `loop`,
  `ifElse`, `Switch`, `Unreachable`, `discard`, `Throw`,
  `TryCall`, `defer`. Branch terminators carry per-target arguments
  that supply the destination block's `block parameter`s. Per-opcode
  catalog is in
  [ir-reference/control-flow.md](ir-reference/control-flow.md).

  See: [ir-reference/control-flow.md](ir-reference/control-flow.md)

**translation unit** `[Slang]`
: One logical unit of source given to the compiler — for HLSL each
  source file is its own translation unit; for Slang several files can
  belong to the same translation unit. Internally represented by
  `TranslationUnitRequest` in
  [slang-compile-request.h](../../source/slang/slang-compile-request.h).

  See: [architecture/overview.md](architecture/overview.md)

**transparent member** `[Slang]`
: A member declaration carrying `TransparentModifier` whose own
  members are made visible in its enclosing scope as if they had been
  declared there. Anonymous `cbuffer` contents and unnamed namespaces
  are the canonical examples; lookup recurses through transparent
  members in `_lookUpDirectAndTransparentMembers` in
  [slang-lookup.cpp](../../source/slang/slang-lookup.cpp) and records
  a `Breadcrumb::Member` step for each one traversed.

  See: [name-resolution/lookup.md](name-resolution/lookup.md)

**two-stage parsing** `[Slang]`
: Slang's strategy of parsing declarations eagerly but deferring the
  parsing of function bodies until after the surrounding declarations
  have been semantically checked. This lets the parser disambiguate
  identifiers that depend on lookup (most notably the `<` of a generic
  application versus a comparison).

  See: [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md)

**type inference** `[General]`
: Inferring the types of expressions and declarations that omit them.
  In Slang this is interleaved with overload resolution and generic
  argument deduction inside the `slang-check-*` files.

  See: [pipeline/03-semantic-check.md](pipeline/03-semantic-check.md)
  External: https://en.wikipedia.org/wiki/Type_inference

**visibility** `[Slang]`
: An access-control attribute attached to a declaration and projected
  onto each scope it appears in. The three levels — `Public`,
  `Internal`, `Private` — are the `DeclVisibility` enum in
  [slang-ast-support-types.h](../../source/slang/slang-ast-support-types.h),
  expressed in source by the `public`/`internal`/`private` keywords
  (modeled by `VisibilityModifier` subclasses in
  [slang-ast-modifier.h](../../source/slang/slang-ast-modifier.h)),
  and enforced both at the lookup boundary and during overload
  resolution by `isDeclVisibleFromScope` in
  [slang-check-expr.cpp](../../source/slang/slang-check-expr.cpp).

  See: [name-resolution/visibility.md](name-resolution/visibility.md)

**witness table** `[Slang]`
: A compile-time table that maps each requirement of an interface to
  the concrete implementation supplied by a conforming type. Witness
  tables underlie Slang's static-dispatch model for generics and
  existentials; they appear in the IR as a dedicated value family.
  Structural opcodes (`WitnessTable`, `WitnessTableEntry`,
  `InterfaceRequirementEntry`) are in
  [ir-reference/structure.md](ir-reference/structure.md); the
  consuming opcodes (`lookupWitness`, `thisTypeWitness`,
  `TypeEqualityWitness`) are in
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md).

  See: [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md),
  [ir-reference/structure.md](ir-reference/structure.md),
  [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md)

## Cross-reference index

The table below maps each peer document to the glossary terms it
introduces. Use it when you have a document in front of you and want a
quick map of the vocabulary you are about to encounter.

| Peer document | Terms |
| --- | --- |
| [architecture/overview.md](architecture/overview.md) | entry point, FIDDLE, linkage, module, session, translation unit |
| [architecture/module-map.md](architecture/module-map.md) | (no glossary entries originate here; consult overview / pipeline) |
| [architecture/dependency-graph.md](architecture/dependency-graph.md) | (no glossary entries originate here; consult overview / pipeline) |
| [pipeline/overview.md](pipeline/overview.md) | entry point |
| [pipeline/01-lex-preprocess.md](pipeline/01-lex-preprocess.md) | lexer, preprocessor, source-loc |
| [pipeline/02-parse-ast.md](pipeline/02-parse-ast.md) | abstract syntax tree, ASTBuilder, parser, recursive descent, two-stage parsing |
| [pipeline/03-semantic-check.md](pipeline/03-semantic-check.md) | decl-ref, lookup result, name resolution, type inference |
| [name-resolution/index.md](name-resolution/index.md) | name resolution |
| [name-resolution/scopes.md](name-resolution/scopes.md) | scope |
| [name-resolution/lookup.md](name-resolution/lookup.md) | lookup breadcrumb, lookup mask, lookup options, lookup result, shadowing, transparent member |
| [name-resolution/visibility.md](name-resolution/visibility.md) | visibility |
| [name-resolution/overload-resolution.md](name-resolution/overload-resolution.md) | conversion cost, overload resolution, partial generic application |
| [pipeline/04-ast-to-ir.md](pipeline/04-ast-to-ir.md) | intermediate representation, IRBuilder, lower-to-IR |
| [pipeline/05-ir-passes.md](pipeline/05-ir-passes.md) | control-flow graph, dataflow analysis, dead-code elimination, dominator, inlining, monomorphization, specialization |
| [pipeline/06-emit.md](pipeline/06-emit.md) | (consult cross-cutting/targets.md and pipeline/04-ast-to-ir.md) |
| [syntax-reference/tokens.md](syntax-reference/tokens.md) | (consult pipeline/01-lex-preprocess.md) |
| [syntax-reference/keywords-and-builtins.md](syntax-reference/keywords-and-builtins.md) | syntax-decl |
| [syntax-reference/grammar.md](syntax-reference/grammar.md) | (consult pipeline/02-parse-ast.md) |
| [cross-cutting/diagnostics.md](cross-cutting/diagnostics.md) | DiagnosticSink |
| [cross-cutting/ir-instructions.md](cross-cutting/ir-instructions.md) | existential type, hoistable instruction, IRDecoration, IRFunc, IRInst, IRModule, IROp, single static assignment (SSA), witness table |
| [cross-cutting/targets.md](cross-cutting/targets.md) | capability atom, profile, target, target intrinsic |
| [cross-cutting/core-module.md](cross-cutting/core-module.md) | core module, prelude |
| [cross-cutting/serialization.md](cross-cutting/serialization.md) | fossil format, RIFF container |
| [ir-reference/index.md](ir-reference/index.md) | IROp, IRInst, IRModule |
| [ir-reference/types.md](ir-reference/types.md) | (no glossary entries originate here; consult cross-cutting/ir-instructions.md) |
| [ir-reference/values.md](ir-reference/values.md) | (no glossary entries originate here; consult cross-cutting/ir-instructions.md) |
| [ir-reference/structure.md](ir-reference/structure.md) | IRFunc, IRModule, parent instruction, witness table |
| [ir-reference/control-flow.md](ir-reference/control-flow.md) | block parameter, terminator instruction |
| [ir-reference/generics-and-existentials.md](ir-reference/generics-and-existentials.md) | existential type, specialization, witness table |
| [ir-reference/resources-and-atomics.md](ir-reference/resources-and-atomics.md) | (no glossary entries originate here; consult cross-cutting/ir-instructions.md) |
| [ir-reference/differentiation.md](ir-reference/differentiation.md) | differential pair |
| [ir-reference/decorations.md](ir-reference/decorations.md) | decoration, IRDecoration, target intrinsic |
| [ir-reference/metadata.md](ir-reference/metadata.md) | (no glossary entries originate here; consult cross-cutting/ir-instructions.md) |
| [ir-reference/misc.md](ir-reference/misc.md) | (no glossary entries originate here; consult cross-cutting/ir-instructions.md) |
